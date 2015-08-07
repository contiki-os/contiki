/*
 * Copyright (C) 2015-2016, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "contiki.h"
#include "i2c.h"

#include "i2c-registers.h"
#include "paging.h"
#include "shared-isr.h"
#include "syscalls.h"

#define I2C_CLOCK_SPEED 25 /* kHz */
#define I2C_FIFO_DEPTH  16

#define I2C_STD_HCNT (I2C_CLOCK_SPEED * 4)
#define I2C_STD_LCNT (I2C_CLOCK_SPEED * 5)
#define I2C_FS_HCNT  (I2C_CLOCK_SPEED)
#define I2C_FS_LCNT  (I2C_CLOCK_SPEED)

#define I2C_FS_SPKLEN_LCNT_OFFSET 8
#define I2C_FS_SPKLEN_HCNT_OFFSET 6

#define I2C_POLLING_TIMEOUT (CLOCK_SECOND / 10)

#define I2C_IRQ 9

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__PAGING
#define MMIO_SZ MIN_PAGE_SIZE
#else
#define MMIO_SZ (QUARKX1000_IC_HIGHEST + 4)
#endif

typedef enum {
  I2C_DIRECTION_READ,
  I2C_DIRECTION_WRITE
} I2C_DIRECTION;

PROT_DOMAINS_ALLOC(pci_driver_t, drv);

struct quarkX1000_i2c_config {
  QUARKX1000_I2C_SPEED speed;
  QUARKX1000_I2C_ADDR_MODE addressing_mode;

  quarkX1000_i2c_callback cb_rx;
  quarkX1000_i2c_callback cb_tx;
  quarkX1000_i2c_callback cb_err;
};

struct i2c_internal_data {
  struct quarkX1000_i2c_config config;

  I2C_DIRECTION direction;

  uint8_t rx_len;
  uint8_t *rx_buffer;
  uint8_t tx_len;
  uint8_t *tx_buffer;
  uint8_t rx_tx_len;

  uint32_t hcnt;
  uint32_t lcnt;
};

static struct i2c_internal_data device;

static int inited = 0;

void quarkX1000_i2c_mmin(uint32_t offset, uint32_t *res);
SYSCALLS_DEFINE_SINGLETON(quarkX1000_i2c_mmin, drv,
                          uint32_t offset, uint32_t *res)
{
  uint32_t *loc_res;

  PROT_DOMAINS_VALIDATE_PTR(loc_res, res, sizeof(*res));
  if(QUARKX1000_IC_HIGHEST < offset) {
    halt();
  }

  prot_domains_enable_mmio();
  PCI_MMIO_READL(drv, *loc_res, offset);
  prot_domains_disable_mmio();
}

static inline uint32_t
read(uint32_t offset)
{
  uint32_t res;
  quarkX1000_i2c_mmin(offset, &res);

  return res;
}

void quarkX1000_i2c_mmout(uint32_t offset, uint32_t val);
SYSCALLS_DEFINE_SINGLETON(quarkX1000_i2c_mmout, drv,
                          uint32_t offset, uint32_t val)
{
  if(QUARKX1000_IC_HIGHEST < offset) {
    halt();
  }

  prot_domains_enable_mmio();
  PCI_MMIO_WRITEL(drv, offset, val);
  prot_domains_disable_mmio();
}

static inline void
write(uint32_t offset, uint32_t val)
{
  quarkX1000_i2c_mmout(offset, val);
}

static uint32_t
get_value(uint32_t offset, uint32_t mask, uint32_t shift)
{
  uint32_t register_value = read(offset);

  register_value &= ~(0xFFFFFFFF - mask);

  return register_value >> shift;
}

static void
set_value(uint32_t offset, uint32_t mask, uint32_t shift, uint32_t value)
{
  uint32_t register_value = read(offset);

  register_value &= ~mask;
  register_value |= value << shift;

  write(offset, register_value);
}

static void
i2c_data_read(void)
{
  uint8_t i, rx_cnt;

  if (device.rx_len == 0)
    return;

  rx_cnt = get_value(QUARKX1000_IC_RXFLR,
    QUARKX1000_IC_RXFLR_MASK, QUARKX1000_IC_RXFLR_SHIFT);

  if (rx_cnt > device.rx_len)
    rx_cnt = device.rx_len;

  for (i = 0; i < rx_cnt; i++) {
    device.rx_buffer[i] = get_value(QUARKX1000_IC_DATA_CMD,
      QUARKX1000_IC_DATA_CMD_DAT_MASK, QUARKX1000_IC_DATA_CMD_DAT_SHIFT);
  }

  device.rx_buffer += i;
  device.rx_len -= i;
}

static void
i2c_data_send(void)
{
  uint32_t data = 0;
  uint8_t i, tx_cnt;

  if (device.rx_tx_len == 0)
    return;

  tx_cnt = I2C_FIFO_DEPTH - get_value(QUARKX1000_IC_TXFLR,
    QUARKX1000_IC_TXFLR_MASK, QUARKX1000_IC_TXFLR_SHIFT);

  if (tx_cnt > device.rx_tx_len)
    tx_cnt = device.rx_tx_len;

  for (i = 0; i < tx_cnt; i++) {
    if (device.tx_len > 0) {
      data = device.tx_buffer[i];

      if (device.tx_len == 1)
        data |= (device.rx_len > 0) ? QUARKX1000_IC_DATA_CMD_RESTART_MASK : QUARKX1000_IC_DATA_CMD_STOP_MASK;

      device.tx_len -= 1;
    } else {
      data = QUARKX1000_IC_DATA_CMD_CMD_MASK;

      if (device.rx_tx_len == 1)
        data |= QUARKX1000_IC_DATA_CMD_STOP_MASK;
    }

    write(QUARKX1000_IC_DATA_CMD, data);
    device.rx_tx_len -= 1;
  }

  device.tx_buffer += i;
}

static bool
i2c_isr(void)
{
  bool handled = false;

  if (read(QUARKX1000_IC_INTR_STAT) & QUARKX1000_IC_INTR_STAT_STOP_DET_MASK) {
    i2c_data_read();

    write(QUARKX1000_IC_INTR_MASK, 0);
    read(QUARKX1000_IC_CLR_INTR);

    if (device.direction == I2C_DIRECTION_WRITE) {
      if (device.config.cb_tx)
        device.config.cb_tx();
    } else {
      if (device.config.cb_rx)
        device.config.cb_rx();
    }

    handled = true;
  }

  if (read(QUARKX1000_IC_INTR_STAT) & QUARKX1000_IC_INTR_STAT_TX_EMPTY_MASK) {
    i2c_data_send();
    if (device.rx_tx_len <= 0) {
      set_value(QUARKX1000_IC_INTR_MASK,
        QUARKX1000_IC_INTR_STAT_TX_EMPTY_MASK, QUARKX1000_IC_INTR_STAT_TX_EMPTY_SHIFT, 0);
      set_value(QUARKX1000_IC_INTR_MASK,
        QUARKX1000_IC_INTR_STAT_STOP_DET_MASK, QUARKX1000_IC_INTR_STAT_STOP_DET_SHIFT, 1);
    }

    handled = true;
  }

  if(read(QUARKX1000_IC_INTR_STAT) & QUARKX1000_IC_INTR_STAT_RX_FULL_MASK) {
    i2c_data_read();

    handled = true;
  }

  if (read(QUARKX1000_IC_INTR_STAT) & (QUARKX1000_IC_INTR_STAT_TX_ABRT_MASK
    | QUARKX1000_IC_INTR_STAT_TX_OVER_MASK | QUARKX1000_IC_INTR_STAT_RX_OVER_MASK
    | QUARKX1000_IC_INTR_STAT_RX_UNDER_MASK)) {
    write(QUARKX1000_IC_INTR_MASK, 0);
    read(QUARKX1000_IC_CLR_INTR);

    if (device.config.cb_err)
      device.config.cb_err();

    handled = true;
  }

  return handled;
}

void
quarkX1000_i2c_configure(QUARKX1000_I2C_SPEED speed,
                         QUARKX1000_I2C_ADDR_MODE addressing_mode)
{
  uint32_t hcnt, lcnt;
  uint8_t ic_fs_spklen;

  device.config.speed = speed;
  device.config.addressing_mode = addressing_mode;

  if (device.config.speed == QUARKX1000_I2C_SPEED_STANDARD) {
    lcnt = I2C_STD_LCNT;
    hcnt = I2C_STD_HCNT;
  } else {
    lcnt = I2C_FS_LCNT;
    hcnt = I2C_FS_HCNT;
  }

  ic_fs_spklen = get_value(QUARKX1000_IC_FS_SPKLEN,
    QUARKX1000_IC_FS_SPKLEN_MASK, QUARKX1000_IC_FS_SPKLEN_SHIFT);

  /* We adjust the Low Count and High Count based on the Spike Suppression Limit */
  device.lcnt = (lcnt < (ic_fs_spklen + I2C_FS_SPKLEN_LCNT_OFFSET)) ? ic_fs_spklen + I2C_FS_SPKLEN_LCNT_OFFSET : lcnt;
  device.hcnt = (hcnt < (ic_fs_spklen + I2C_FS_SPKLEN_HCNT_OFFSET)) ? ic_fs_spklen + I2C_FS_SPKLEN_HCNT_OFFSET : hcnt;

  /* Clear interrupts. */
  read(QUARKX1000_IC_CLR_INTR);
}

void
quarkX1000_i2c_set_callbacks(quarkX1000_i2c_callback rx,
                             quarkX1000_i2c_callback tx,
                             quarkX1000_i2c_callback err)
{
  device.config.cb_rx = rx;
  device.config.cb_tx = tx;
  device.config.cb_err = err;
}

static int
i2c_setup(void)
{
  /* Clear all values */
  write(QUARKX1000_IC_CON, 0);

  /* Clear interrupts */
  read(QUARKX1000_IC_CLR_INTR);

  /* Quark X1000 SoC I2C only supports master mode. */
  set_value(QUARKX1000_IC_CON,
    QUARKX1000_IC_CON_MASTER_MODE_MASK, QUARKX1000_IC_CON_MASTER_MODE_SHIFT, 1);

  /* Set restart enable */
  set_value(QUARKX1000_IC_CON,
    QUARKX1000_IC_CON_RESTART_EN_MASK, QUARKX1000_IC_CON_RESTART_EN_SHIFT, 1);

  /* Set addressing mode */
  if (device.config.addressing_mode == QUARKX1000_I2C_ADDR_MODE_10BIT) {
    set_value(QUARKX1000_IC_CON,
      QUARKX1000_IC_CON_10BITADDR_MASTER_MASK, QUARKX1000_IC_CON_10BITADDR_MASTER_SHIFT, 1);
  }

  if (device.config.speed == QUARKX1000_I2C_SPEED_STANDARD) {
    set_value(QUARKX1000_IC_SS_SCL_LCNT,
      QUARKX1000_IC_SS_SCL_LCNT_MASK, QUARKX1000_IC_SS_SCL_LCNT_SHIFT, device.lcnt);
    set_value(QUARKX1000_IC_SS_SCL_HCNT,
      QUARKX1000_IC_SS_SCL_HCNT_MASK, QUARKX1000_IC_SS_SCL_HCNT_SHIFT, device.hcnt);
    set_value(QUARKX1000_IC_CON,
      QUARKX1000_IC_CON_SPEED_MASK, QUARKX1000_IC_CON_SPEED_SHIFT, 0x1);
  } else {
    set_value(QUARKX1000_IC_FS_SCL_LCNT,
      QUARKX1000_IC_FS_SCL_LCNT_MASK, QUARKX1000_IC_FS_SCL_LCNT_SHIFT, device.lcnt);
    set_value(QUARKX1000_IC_FS_SCL_HCNT,
      QUARKX1000_IC_FS_SCL_HCNT_MASK, QUARKX1000_IC_FS_SCL_HCNT_SHIFT, device.hcnt);
    set_value(QUARKX1000_IC_CON,
      QUARKX1000_IC_CON_SPEED_MASK, QUARKX1000_IC_CON_SPEED_SHIFT, 0x2);
  }

  return 0;
}

static void
i2c_operation_setup(uint8_t *write_buf, uint8_t write_len,
  uint8_t *read_buf,  uint8_t read_len, uint16_t addr)
{
  device.rx_len = read_len;
  device.rx_buffer = read_buf;
  device.tx_len = write_len;
  device.tx_buffer = write_buf;
  device.rx_tx_len = device.rx_len + device.tx_len;

  /* Disable controller */
  set_value(QUARKX1000_IC_ENABLE,
    QUARKX1000_IC_ENABLE_MASK, QUARKX1000_IC_ENABLE_SHIFT, 0);

  i2c_setup();

  /* Disable interrupts */
  write(QUARKX1000_IC_INTR_MASK, 0);

  /* Clear interrupts */
  read(QUARKX1000_IC_CLR_INTR);

  /* Set address of target slave */
  set_value(QUARKX1000_IC_TAR,
    QUARKX1000_IC_TAR_MASK, QUARKX1000_IC_TAR_SHIFT, addr);
}

/* This is an interrupt based operation */
static int
i2c_operation(uint8_t *write_buf, uint8_t write_len,
  uint8_t *read_buf,  uint8_t read_len, uint16_t addr)
{
  if (read(QUARKX1000_IC_STATUS) & QUARKX1000_IC_STATUS_ACTIVITY_MASK)
    return -1;

  i2c_operation_setup(write_buf, write_len, read_buf, read_len, addr);

  /* Enable master TX and RX interrupts */
  set_value(QUARKX1000_IC_INTR_MASK,
    QUARKX1000_IC_INTR_STAT_TX_OVER_MASK, QUARKX1000_IC_INTR_STAT_TX_OVER_SHIFT, 1);
  set_value(QUARKX1000_IC_INTR_MASK,
    QUARKX1000_IC_INTR_STAT_TX_EMPTY_MASK, QUARKX1000_IC_INTR_STAT_TX_EMPTY_SHIFT, 1);
  set_value(QUARKX1000_IC_INTR_MASK,
    QUARKX1000_IC_INTR_STAT_TX_ABRT_MASK, QUARKX1000_IC_INTR_STAT_TX_ABRT_SHIFT, 1);
  set_value(QUARKX1000_IC_INTR_MASK,
    QUARKX1000_IC_INTR_STAT_RX_UNDER_MASK, QUARKX1000_IC_INTR_STAT_RX_UNDER_SHIFT, 1);
  set_value(QUARKX1000_IC_INTR_MASK,
    QUARKX1000_IC_INTR_STAT_RX_OVER_MASK, QUARKX1000_IC_INTR_STAT_RX_OVER_SHIFT, 1);
  set_value(QUARKX1000_IC_INTR_MASK,
    QUARKX1000_IC_INTR_STAT_RX_FULL_MASK, QUARKX1000_IC_INTR_STAT_RX_FULL_SHIFT, 1);
  set_value(QUARKX1000_IC_INTR_MASK,
    QUARKX1000_IC_INTR_STAT_STOP_DET_MASK, QUARKX1000_IC_INTR_STAT_STOP_DET_SHIFT, 1);

  /* Enable controller */
  set_value(QUARKX1000_IC_ENABLE,
    QUARKX1000_IC_ENABLE_MASK, QUARKX1000_IC_ENABLE_SHIFT, 1);

  return 0;
}

/* This is an interrupt based write */
int
quarkX1000_i2c_write(uint8_t *buf, uint8_t len, uint16_t addr)
{
  device.direction = I2C_DIRECTION_WRITE;
  return i2c_operation(buf, len, 0, 0, addr);
}

/* This is an interrupt based read */
int
quarkX1000_i2c_read(uint8_t *buf, uint8_t len, uint16_t addr)
{
  device.direction = I2C_DIRECTION_READ;
  return i2c_operation(0, 0, buf, len, addr);
}

static int
i2c_polling_operation(uint8_t *write_buf, uint8_t write_len,
  uint8_t *read_buf,  uint8_t read_len, uint16_t addr)
{
  uint32_t start_time, intr_mask_stat;

  if (!(read(QUARKX1000_IC_CON) & QUARKX1000_IC_CON_MASTER_MODE_MASK))
    return -1;

  /* Wait i2c idle */
  start_time = clock_seconds();
  while (read(QUARKX1000_IC_STATUS) & QUARKX1000_IC_STATUS_ACTIVITY_MASK) {
    if ((clock_seconds() - start_time) > I2C_POLLING_TIMEOUT) {
      return -1;
    }
  }

  /* Get interrupt mask to restore in the end of polling operation */
  intr_mask_stat = read(QUARKX1000_IC_INTR_MASK);

  i2c_operation_setup(write_buf, write_len, read_buf, read_len, addr);

  /* Enable controller */
  set_value(QUARKX1000_IC_ENABLE,
    QUARKX1000_IC_ENABLE_MASK, QUARKX1000_IC_ENABLE_SHIFT, 1);

  /* Transmit */
  if (device.tx_len != 0) {
    while (device.tx_len > 0) {
      start_time = clock_seconds();
      while (!(read(QUARKX1000_IC_STATUS) & QUARKX1000_IC_STATUS_TFNF_MASK)) {
        if ((clock_seconds() - start_time) > I2C_POLLING_TIMEOUT) {
          set_value(QUARKX1000_IC_ENABLE,
            QUARKX1000_IC_ENABLE_MASK, QUARKX1000_IC_ENABLE_SHIFT, 0);
          return -1;
        }
      }
      i2c_data_send();
    }

    start_time = clock_seconds();
    while (!(read(QUARKX1000_IC_STATUS) & QUARKX1000_IC_STATUS_TFE_MASK)) {
      if ((clock_seconds() - start_time) > I2C_POLLING_TIMEOUT) {
        set_value(QUARKX1000_IC_ENABLE,
          QUARKX1000_IC_ENABLE_MASK, QUARKX1000_IC_ENABLE_SHIFT, 0);
        return -1;
      }
    }
  }

  i2c_data_send();

  /* Receive */
  if (device.rx_len != 0) {
    while (device.rx_len > 0) {
      start_time = clock_seconds();
      while (!(read(QUARKX1000_IC_STATUS) & QUARKX1000_IC_STATUS_RFNE_MASK)) {
        if ((clock_seconds() - start_time) > I2C_POLLING_TIMEOUT) {
          set_value(QUARKX1000_IC_ENABLE,
            QUARKX1000_IC_ENABLE_MASK, QUARKX1000_IC_ENABLE_SHIFT, 0);
          return -1;
        }
      }
      i2c_data_read();
    }
  }

  /* Stop Det */
  start_time = clock_seconds();
  while (!(read(QUARKX1000_IC_RAW_INTR_STAT) & QUARKX1000_IC_INTR_STAT_STOP_DET_MASK)) {
    if ((clock_seconds() - start_time) > I2C_POLLING_TIMEOUT) {
      set_value(QUARKX1000_IC_ENABLE,
        QUARKX1000_IC_ENABLE_MASK, QUARKX1000_IC_ENABLE_SHIFT, 0);
      return -1;
    }
  }
  read(QUARKX1000_IC_CLR_STOP_DET);

  /* Wait i2c idle */
  start_time = clock_seconds();
  while (read(QUARKX1000_IC_STATUS) & QUARKX1000_IC_STATUS_ACTIVITY_MASK) {
    if ((clock_seconds() - start_time) > I2C_POLLING_TIMEOUT) {
      set_value(QUARKX1000_IC_ENABLE,
        QUARKX1000_IC_ENABLE_MASK, QUARKX1000_IC_ENABLE_SHIFT, 0);
      return -1;
    }
  }

  /* Disable controller */
  set_value(QUARKX1000_IC_ENABLE,
    QUARKX1000_IC_ENABLE_MASK, QUARKX1000_IC_ENABLE_SHIFT, 0);

  /* Restore interrupt mask */
  write(QUARKX1000_IC_INTR_MASK, intr_mask_stat);

  return 0;
}

int
quarkX1000_i2c_polling_write(uint8_t *buf, uint8_t len, uint16_t addr)
{
  device.direction = I2C_DIRECTION_WRITE;
  return i2c_polling_operation(buf, len, 0, 0, addr);
}

int
quarkX1000_i2c_polling_read(uint8_t *buf, uint8_t len, uint16_t addr)
{
  device.direction = I2C_DIRECTION_READ;
  return i2c_polling_operation(0, 0, buf, len ,addr);
}

int
quarkX1000_i2c_is_available(void)
{
  return inited;
}

DEFINE_SHARED_IRQ(I2C_IRQ, IRQAGENT3, INTC, PIRQC, i2c_isr);

int
quarkX1000_i2c_init(void)
{
  pci_config_addr_t pci_addr;

  pci_addr.raw = 0;
  pci_addr.bus = 0;
  pci_addr.dev = 21;
  pci_addr.func = 2;
  pci_addr.reg_off = PCI_CONFIG_REG_BAR0;

  pci_command_enable(pci_addr, PCI_CMD_1_MEM_SPACE_EN);

  PROT_DOMAINS_INIT_ID(drv);
  pci_init(&drv, pci_addr, MMIO_SZ, 0, 0);
  SYSCALLS_INIT(quarkX1000_i2c_mmin);
  SYSCALLS_AUTHZ(quarkX1000_i2c_mmin, drv);
  SYSCALLS_INIT(quarkX1000_i2c_mmout);
  SYSCALLS_AUTHZ(quarkX1000_i2c_mmout, drv);

  inited = 1;

  return 0;
}
