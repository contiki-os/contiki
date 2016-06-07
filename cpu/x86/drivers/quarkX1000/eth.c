/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
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

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "contiki-net.h"
#include "dma.h"
#include "eth.h"
#include "helpers.h"
#include "syscalls.h"
#include "net/ip/uip.h"
#include "pci.h"

typedef pci_driver_t quarkX1000_eth_driver_t;

/* Refer to Intel Quark SoC X1000 Datasheet, Chapter 15 for more details on
 * Ethernet device operation.
 *
 * This driver puts the Ethernet device into a very simple and space-efficient
 * mode of operation.  It only allocates a single packet descriptor for each of
 * the transmit and receive directions, computes checksums on the CPU, and
 * enables store-and-forward mode for both transmit and receive directions.
 */

/* Transmit descriptor */
typedef struct quarkX1000_eth_tx_desc {
  /* First word of transmit descriptor */
  union {
    struct {
      /* Only valid in half-duplex mode. */
      uint32_t deferred_bit      : 1;
      uint32_t err_underflow     : 1;
      uint32_t err_excess_defer  : 1;
      uint32_t coll_cnt_slot_num : 4;
      uint32_t vlan_frm          : 1;
      uint32_t err_excess_coll   : 1;
      uint32_t err_late_coll     : 1;
      uint32_t err_no_carrier    : 1;
      uint32_t err_carrier_loss  : 1;
      uint32_t err_ip_payload    : 1;
      uint32_t err_frm_flushed   : 1;
      uint32_t err_jabber_tout   : 1;
      /* OR of all other error bits. */
      uint32_t err_summary       : 1;
      uint32_t err_ip_hdr        : 1;
      uint32_t tx_timestamp_stat : 1;
      uint32_t vlan_ins_ctrl     : 2;
      uint32_t addr2_chained     : 1;
      uint32_t tx_end_of_ring    : 1;
      uint32_t chksum_ins_ctrl   : 2;
      uint32_t replace_crc       : 1;
      uint32_t tx_timestamp_en   : 1;
      uint32_t dis_pad           : 1;
      uint32_t dis_crc           : 1;
      uint32_t first_seg_in_frm  : 1;
      uint32_t last_seg_in_frm   : 1;
      uint32_t intr_on_complete  : 1;
      /* When set, descriptor is owned by DMA. */
      uint32_t own               : 1;
    };
    uint32_t tdes0;
  };
  /* Second word of transmit descriptor */
  union {
    struct {
      uint32_t tx_buf1_sz        : 13;
      uint32_t                   : 3;
      uint32_t tx_buf2_sz        : 13;
      uint32_t src_addr_ins_ctrl : 3;
    };
    uint32_t tdes1;
  };
  /* Pointer to frame data buffer */
  uint8_t *buf1_ptr;
  /* Unused, since this driver initializes only a single descriptor for each
   * direction.
   */
  uint8_t *buf2_ptr;
} quarkX1000_eth_tx_desc_t;

/* Transmit descriptor */
typedef struct quarkX1000_eth_rx_desc {
  /* First word of receive descriptor */
  union {
    struct {
      uint32_t ext_stat          : 1;
      uint32_t err_crc           : 1;
      uint32_t err_dribble_bit   : 1;
      uint32_t err_rx_mii        : 1;
      uint32_t err_rx_wdt        : 1;
      uint32_t frm_type          : 1;
      uint32_t err_late_coll     : 1;
      uint32_t giant_frm         : 1;
      uint32_t last_desc         : 1;
      uint32_t first_desc        : 1;
      uint32_t vlan_tag          : 1;
      uint32_t err_overflow      : 1;
      uint32_t length_err        : 1;
      uint32_t s_addr_filt_fail  : 1;
      uint32_t err_desc          : 1;
      uint32_t err_summary       : 1;
      uint32_t frm_len           : 14;
      uint32_t d_addr_filt_fail  : 1;
      uint32_t own               : 1;
    };
    uint32_t rdes0;
  };
  /* Second word of receive descriptor */
  union {
    struct {
      uint32_t rx_buf1_sz        : 13;
      uint32_t                   : 1;
      uint32_t addr2_chained     : 1;
      uint32_t rx_end_of_ring    : 1;
      uint32_t rx_buf2_sz        : 13;
      uint32_t                   : 2;
      uint32_t dis_int_compl     : 1;
    };
    uint32_t rdes1;
  };
  /* Pointer to frame data buffer */
  uint8_t *buf1_ptr;
  /* Unused, since this driver initializes only a single descriptor for each
   * direction.
   */
  uint8_t *buf2_ptr;
} quarkX1000_eth_rx_desc_t;

/* Driver metadata associated with each Ethernet device */
typedef struct quarkX1000_eth_meta {
  /* Transmit descriptor */
  volatile quarkX1000_eth_tx_desc_t tx_desc;
  /* Transmit DMA packet buffer */
  volatile uint8_t tx_buf[ALIGN(UIP_BUFSIZE, 4)];
  /* Receive descriptor */
  volatile quarkX1000_eth_rx_desc_t rx_desc;
  /* Receive DMA packet buffer */
  volatile uint8_t rx_buf[ALIGN(UIP_BUFSIZE, 4)];

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__PAGING
  /* Domain-defined metadata must fill an even number of pages, since that is
   * the minimum granularity of access control supported by paging.  However,
   * using the "aligned(4096)" attribute causes the alignment of the kernel
   * data section to increase, which causes problems when generating UEFI
   * binaries, as is described in the linker script.  Thus, it is necessary
   * to manually pad the structure to fill a page.  This only works if the
   * sizes of the actual fields of the structure are collectively less than a
   * page.
   */
  uint8_t pad[MIN_PAGE_SIZE -
              (sizeof(quarkX1000_eth_tx_desc_t) +
               ALIGN(UIP_BUFSIZE, 4) +
               sizeof(quarkX1000_eth_rx_desc_t) +
               ALIGN(UIP_BUFSIZE, 4))];
#endif
} __attribute__((packed)) quarkX1000_eth_meta_t;

#define LOG_PFX "quarkX1000_eth: "

#define MMIO_SZ 0x2000

#define MAC_CONF_14_RMII_100M          BIT(14)
#define MAC_CONF_11_DUPLEX             BIT(11)
#define MAC_CONF_3_TX_EN               BIT(3)
#define MAC_CONF_2_RX_EN               BIT(2)

#define OP_MODE_25_RX_STORE_N_FORWARD  BIT(25)
#define OP_MODE_21_TX_STORE_N_FORWARD  BIT(21)
#define OP_MODE_13_START_TX            BIT(13)
#define OP_MODE_1_START_RX             BIT(1)

#define REG_ADDR_MAC_CONF              0x0000
#define REG_ADDR_MACADDR_HI            0x0040
#define REG_ADDR_MACADDR_LO            0x0044
#define REG_ADDR_TX_POLL_DEMAND        0x1004
#define REG_ADDR_RX_POLL_DEMAND        0x1008
#define REG_ADDR_RX_DESC_LIST          0x100C
#define REG_ADDR_TX_DESC_LIST          0x1010
#define REG_ADDR_DMA_OPERATION         0x1018

PROT_DOMAINS_ALLOC(quarkX1000_eth_driver_t, drv);
static quarkX1000_eth_meta_t ATTR_BSS_DMA meta;

void quarkX1000_eth_setup(uintptr_t meta_phys_base);

/*---------------------------------------------------------------------------*/
SYSCALLS_DEFINE_SINGLETON(quarkX1000_eth_setup, drv, uintptr_t meta_phys_base)
{
  uip_eth_addr mac_addr;
  uint32_t mac_tmp1, mac_tmp2;
  quarkX1000_eth_rx_desc_t rx_desc;
  quarkX1000_eth_tx_desc_t tx_desc;
  quarkX1000_eth_meta_t ATTR_META_ADDR_SPACE *loc_meta =
    (quarkX1000_eth_meta_t ATTR_META_ADDR_SPACE *)PROT_DOMAINS_META(drv);

  prot_domains_enable_mmio();

  /* Read the MAC address from the device. */
  PCI_MMIO_READL(drv, mac_tmp1, REG_ADDR_MACADDR_HI);
  PCI_MMIO_READL(drv, mac_tmp2, REG_ADDR_MACADDR_LO);

  prot_domains_disable_mmio();

  /* Convert the data read from the device into the format expected by
   * Contiki.
   */
  mac_addr.addr[5] = (uint8_t)(mac_tmp1 >> 8);
  mac_addr.addr[4] = (uint8_t)mac_tmp1;
  mac_addr.addr[3] = (uint8_t)(mac_tmp2 >> 24);
  mac_addr.addr[2] = (uint8_t)(mac_tmp2 >> 16);
  mac_addr.addr[1] = (uint8_t)(mac_tmp2 >> 8);
  mac_addr.addr[0] = (uint8_t)mac_tmp2;

  printf(LOG_PFX "MAC address = %02x:%02x:%02x:%02x:%02x:%02x.\n",
         mac_addr.addr[0],
         mac_addr.addr[1],
         mac_addr.addr[2],
         mac_addr.addr[3],
         mac_addr.addr[4],
         mac_addr.addr[5]
         );

  uip_setethaddr(mac_addr);

  /* Initialize transmit descriptor. */
  tx_desc.tdes0 = 0;
  tx_desc.tdes1 = 0;

  tx_desc.tx_end_of_ring = 1;
  tx_desc.first_seg_in_frm = 1;
  tx_desc.last_seg_in_frm = 1;
  tx_desc.tx_end_of_ring = 1;

  META_WRITEL(loc_meta->tx_desc.tdes0, tx_desc.tdes0);
  META_WRITEL(loc_meta->tx_desc.tdes1, tx_desc.tdes1);
  META_WRITEL(loc_meta->tx_desc.buf1_ptr,
              (uint8_t *)PROT_DOMAINS_META_OFF_TO_PHYS(
                (uintptr_t)&loc_meta->tx_buf, meta_phys_base));
  META_WRITEL(loc_meta->tx_desc.buf2_ptr, 0);

  /* Initialize receive descriptor. */
  rx_desc.rdes0 = 0;
  rx_desc.rdes1 = 0;

  rx_desc.own = 1;
  rx_desc.first_desc = 1;
  rx_desc.last_desc = 1;
  rx_desc.rx_buf1_sz = UIP_BUFSIZE;
  rx_desc.rx_end_of_ring = 1;

  META_WRITEL(loc_meta->rx_desc.rdes0, rx_desc.rdes0);
  META_WRITEL(loc_meta->rx_desc.rdes1, rx_desc.rdes1);
  META_WRITEL(loc_meta->rx_desc.buf1_ptr,
              (uint8_t *)PROT_DOMAINS_META_OFF_TO_PHYS(
                (uintptr_t)&loc_meta->rx_buf, meta_phys_base));
  META_WRITEL(loc_meta->rx_desc.buf2_ptr, 0);

  prot_domains_enable_mmio();

  /* Install transmit and receive descriptors. */
  PCI_MMIO_WRITEL(drv, REG_ADDR_RX_DESC_LIST,
                  PROT_DOMAINS_META_OFF_TO_PHYS(
                    (uintptr_t)&loc_meta->rx_desc, meta_phys_base));
  PCI_MMIO_WRITEL(drv, REG_ADDR_TX_DESC_LIST,
                  PROT_DOMAINS_META_OFF_TO_PHYS(
                    (uintptr_t)&loc_meta->tx_desc, meta_phys_base));

  PCI_MMIO_WRITEL(drv, REG_ADDR_MAC_CONF,
                  /* Set the RMII speed to 100Mbps */
                  MAC_CONF_14_RMII_100M |
                  /* Enable full-duplex mode */
                  MAC_CONF_11_DUPLEX |
                  /* Enable transmitter */
                  MAC_CONF_3_TX_EN |
                  /* Enable receiver */
                  MAC_CONF_2_RX_EN);

  PCI_MMIO_WRITEL(drv, REG_ADDR_DMA_OPERATION,
                  /* Enable receive store-and-forward mode for simplicity. */
                  OP_MODE_25_RX_STORE_N_FORWARD |
                  /* Enable transmit store-and-forward mode for simplicity. */
                  OP_MODE_21_TX_STORE_N_FORWARD |
                  /* Place the transmitter state machine in the Running
                     state. */
                  OP_MODE_13_START_TX |
                  /* Place the receiver state machine in the Running state. */
                  OP_MODE_1_START_RX);

  prot_domains_disable_mmio();

  printf(LOG_PFX "Enabled 100M full-duplex mode.\n");
}

/*---------------------------------------------------------------------------*/
/**
 * \brief           Poll for a received Ethernet frame.
 * \param frame_len Will be set to the size of the received Ethernet frame or
 *                  zero if no frame is available.
 *
 *                  If a frame is received, this procedure copies it into the
 *                  global uip_buf buffer.
 */
SYSCALLS_DEFINE_SINGLETON(quarkX1000_eth_poll, drv, uint16_t * frame_len)
{
  uint16_t *loc_frame_len;
  uint16_t frm_len = 0;
  quarkX1000_eth_rx_desc_t tmp_desc;
  quarkX1000_eth_meta_t ATTR_META_ADDR_SPACE *loc_meta =
    (quarkX1000_eth_meta_t ATTR_META_ADDR_SPACE *)PROT_DOMAINS_META(drv);

  PROT_DOMAINS_VALIDATE_PTR(loc_frame_len, frame_len, sizeof(*frame_len));

  META_READL(tmp_desc.rdes0, loc_meta->rx_desc.rdes0);

  /* Check whether the RX descriptor is still owned by the device.  If not,
   * process the received frame or an error that may have occurred.
   */
  if(tmp_desc.own == 0) {
    META_READL(tmp_desc.rdes1, loc_meta->rx_desc.rdes1);
    if(tmp_desc.err_summary) {
      fprintf(stderr,
              LOG_PFX "Error receiving frame: RDES0 = %08x, RDES1 = %08x.\n",
              tmp_desc.rdes0, tmp_desc.rdes1);
      assert(0);
    }

    frm_len = tmp_desc.frm_len;
    assert(frm_len <= UIP_BUFSIZE);
    MEMCPY_FROM_META(uip_buf, loc_meta->rx_buf, frm_len);

    /* Return ownership of the RX descriptor to the device. */
    tmp_desc.own = 1;

    META_WRITEL(loc_meta->rx_desc.rdes0, tmp_desc.rdes0);

    prot_domains_enable_mmio();

    /* Request that the device check for an available RX descriptor, since
     * ownership of the descriptor was just transferred to the device.
     */
    PCI_MMIO_WRITEL(drv, REG_ADDR_RX_POLL_DEMAND, 1);

    prot_domains_disable_mmio();
  }

  *loc_frame_len = frm_len;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Transmit the current Ethernet frame.
 *
 *        This procedure will block indefinitely until the Ethernet device is
 *        ready to accept a new outgoing frame.  It then copies the current
 *        Ethernet frame from the global uip_buf buffer to the device DMA
 *        buffer and signals to the device that a new frame is available to be
 *        transmitted.
 */
SYSCALLS_DEFINE_SINGLETON(quarkX1000_eth_send, drv)
{
  quarkX1000_eth_tx_desc_t tmp_desc;
  quarkX1000_eth_meta_t ATTR_META_ADDR_SPACE *loc_meta =
    (quarkX1000_eth_meta_t ATTR_META_ADDR_SPACE *)PROT_DOMAINS_META(drv);

  /* Wait until the TX descriptor is no longer owned by the device. */
  do {
    META_READL(tmp_desc.tdes0, loc_meta->tx_desc.tdes0);
  } while(tmp_desc.own == 1);

  META_READL(tmp_desc.tdes1, loc_meta->tx_desc.tdes1);

  /* Check whether an error occurred transmitting the previous frame. */
  if(tmp_desc.err_summary) {
    fprintf(stderr,
            LOG_PFX "Error transmitting frame: TDES0 = %08x, TDES1 = %08x.\n",
            tmp_desc.tdes0, tmp_desc.tdes1);
    assert(0);
  }

  /* Transmit the next frame. */
  assert(uip_len <= UIP_BUFSIZE);
  MEMCPY_TO_META(loc_meta->tx_buf, uip_buf, uip_len);

  tmp_desc.tx_buf1_sz = uip_len;

  META_WRITEL(loc_meta->tx_desc.tdes1, tmp_desc.tdes1);

  tmp_desc.own = 1;

  META_WRITEL(loc_meta->tx_desc.tdes0, tmp_desc.tdes0);

  prot_domains_enable_mmio();

  /* Request that the device check for an available TX descriptor, since
   * ownership of the descriptor was just transferred to the device.
   */
  PCI_MMIO_WRITEL(drv, REG_ADDR_TX_POLL_DEMAND, 1);

  prot_domains_disable_mmio();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize the first Quark X1000 Ethernet MAC.
 *
 *        This procedure assumes that an MMIO range for the device was
 *        previously assigned, e.g. by firmware.
 */
void
quarkX1000_eth_init(void)
{
  pci_config_addr_t pci_addr = { .raw = 0 };

  /* PCI address from section 15.4 of Intel Quark SoC X1000 Datasheet. */

  pci_addr.dev = 20;
  pci_addr.func = 6;

  /* Activate MMIO and DMA access. */
  pci_command_enable(pci_addr, PCI_CMD_2_BUS_MST_EN | PCI_CMD_1_MEM_SPACE_EN);

  printf(LOG_PFX "Activated MMIO and DMA access.\n");

  pci_addr.reg_off = PCI_CONFIG_REG_BAR0;

  PROT_DOMAINS_INIT_ID(drv);
  /* Configure the device MMIO range and initialize the driver structure. */
  pci_init(&drv, pci_addr, MMIO_SZ,
           (uintptr_t)&meta, sizeof(quarkX1000_eth_meta_t));
  SYSCALLS_INIT(quarkX1000_eth_setup);
  SYSCALLS_AUTHZ(quarkX1000_eth_setup, drv);
  SYSCALLS_INIT(quarkX1000_eth_poll);
  SYSCALLS_AUTHZ(quarkX1000_eth_poll, drv);
  SYSCALLS_INIT(quarkX1000_eth_send);
  SYSCALLS_AUTHZ(quarkX1000_eth_send, drv);

  quarkX1000_eth_setup(prot_domains_lookup_meta_phys_base(&drv));
}
/*---------------------------------------------------------------------------*/
