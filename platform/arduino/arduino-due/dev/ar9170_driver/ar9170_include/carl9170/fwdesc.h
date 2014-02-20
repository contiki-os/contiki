/*
 * Atheros CARL9170 driver
 *
 * Copyright 2008, Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2009, 2010, Christian Lamparter <chunkeey@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, see
 * http://www.gnu.org/licenses/.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *    Copyright (c) 2007-2008 Atheros Communications, Inc.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "bitops.h"
#include "compiler.h"

#ifndef FWDESC_H_
#define FWDESC_H_

/* NOTE: Don't mess with the order of the flags! */
 enum carl9170fw_feature_list {
         /* Always set */
         CARL9170FW_DUMMY_FEATURE,
 
         /*
          * Indicates that this image has special boot block which prevents
          * legacy drivers to drive the firmware.
          */
         CARL9170FW_MINIBOOT,
 
         /* usb registers are initialized by the firmware */
         CARL9170FW_USB_INIT_FIRMWARE,
 
         /* command traps & notifications are send through EP2 */
         CARL9170FW_USB_RESP_EP2,
 
         /* usb download (app -> fw) stream */
         CARL9170FW_USB_DOWN_STREAM,
 
         /* usb upload (fw -> app) stream */
         CARL9170FW_USB_UP_STREAM,
 
         /* unusable - reserved to flag non-functional debug firmwares */
         CARL9170FW_UNUSABLE,
 
         /* AR9170_CMD_RF_INIT, AR9170_CMD_FREQ_START, AR9170_CMD_FREQUENCY */
         CARL9170FW_COMMAND_PHY,
 
         /* AR9170_CMD_EKEY, AR9170_CMD_DKEY */
         CARL9170FW_COMMAND_CAM,
 
         /* Firmware has a software Content After Beacon Queueing mechanism */
         CARL9170FW_WLANTX_CAB,
 
         /* The firmware is capable of responding to incoming BAR frames */
         CARL9170FW_HANDLE_BACK_REQ,
 
         /* GPIO Interrupt | CARL9170_RSP_GPIO */
         CARL9170FW_GPIO_INTERRUPT,
 
         /* Firmware PSM support | CARL9170_CMD_PSM */
         CARL9170FW_PSM,
 
         /* Firmware RX filter | CARL9170_CMD_RX_FILTER */
         CARL9170FW_RX_FILTER,
 
         /* Wake up on WLAN */
         CARL9170FW_WOL,
 
         /* Firmware supports PSM in the 5GHZ Band */
         CARL9170FW_FIXED_5GHZ_PSM,
 
         /* HW (ANI, CCA, MIB) tally counters */
         CARL9170FW_HW_COUNTERS,
 
         /* Firmware will pass BA when BARs are queued */
         CARL9170FW_RX_BA_FILTER,
 
         /* KEEP LAST */
         __CARL9170FW_FEATURE_NUM
};

#define CARL9170FW_SET_DAY(d) (((d) - 1) % 31)
#define CARL9170FW_SET_MONTH(m) ((((m) - 1) % 12) * 31)
#define CARL9170FW_SET_YEAR(y) (((y) - 10) * 372)

#define CARL9170FW_GET_DAY(d) (((d) % 31) + 1)
#define CARL9170FW_GET_MONTH(m) ((((m) / 31) % 12) + 1)
#define CARL9170FW_GET_YEAR(y) ((y) / 372 + 10)

#define CARL9170FW_MAGIC_SIZE			4

#define CARL9170FW_DESC_MAX_LENGTH		8192

#define CARL9170FW_LAST_DESC_CUR_VER                    2

#define LAST_MAGIC      "LAST"
#define OTUS_MAGIC      "OTAR"
#define MOTD_MAGIC      "MOTD"
#define FIX_MAGIC       "FIX\0"
#define DBG_MAGIC       "DBG\0"
#define CHK_MAGIC       "CHK\0"
#define TXSQ_MAGIC      "TXSQ"
#define WOL_MAGIC       "WOL\0"


struct ar9170fw_desc_head {
	U8	magic[CARL9170FW_MAGIC_SIZE];
	le16_t length;
	U8 min_ver;
	U8 cur_ver;
} __attribute__ ((packed));

#define CARL9170FW_LAST_DESC_MIN_VER                    1
#define CARL9170FW_LAST_DESC_CUR_VER                    2

struct carl9170fw_last_desc {
	struct ar9170fw_desc_head head;
} __attribute__ ((packed));
#define CARL9170FW_LAST_DESC_SIZE                       \
(sizeof(struct carl9170fw_fix_desc))

#define CARL9170FW_CHK_DESC_MIN_VER                     1
#define CARL9170FW_CHK_DESC_CUR_VER                     2
struct carl9170fw_chk_desc {
        struct ar9170fw_desc_head head;
        le32_t fw_crc32;
        le32_t hdr_crc32;
} __attribute__ ((packed));

#define CARL9170FW_CHK_DESC_SIZE                        \
        (sizeof(struct carl9170fw_chk_desc))

#define CARL9170FW_TXSQ_DESC_CUR_VER                    1
struct carl9170fw_txsq_desc {
         struct ar9170fw_desc_head head;
         le32_t seq_table_addr;
} __attribute__ ((packed));
#define CARL9170FW_TXSQ_DESC_SIZE                       \
        (sizeof(struct carl9170fw_txsq_desc))

#define CARL9170FW_OTUS_DESC_MIN_VER            6
#define CARL9170FW_OTUS_DESC_CUR_VER            7
struct carl9170fw_otus_desc {
         struct ar9170fw_desc_head head;
         le32_t feature_set;
         le32_t fw_address;
         le32_t bcn_addr;
         le16_t bcn_len;
         le16_t miniboot_size;
         le16_t tx_frag_len;
         le16_t rx_max_frame_len;
         U8 tx_descs;
         U8 cmd_bufs;
         U8 api_ver;
         U8 vif_num;
} __attribute__ ((packed));

#define CARL9170FW_OTUS_DESC_SIZE                       \
        (sizeof(struct carl9170fw_otus_desc))

#define CARL9170FW_MOTD_STRING_LEN                      24
#define CARL9170FW_MOTD_RELEASE_LEN                     20
#define CARL9170FW_MOTD_DESC_MIN_VER                    1
#define CARL9170FW_MOTD_DESC_CUR_VER                    2
struct carl9170fw_motd_desc {
         struct ar9170fw_desc_head head;
         le32_t fw_year_month_day;
         char desc[CARL9170FW_MOTD_STRING_LEN];
         char release[CARL9170FW_MOTD_RELEASE_LEN];
} __attribute__ ((packed));


#define CARL9170FW_LAST_DESC_SIZE                       \
         (sizeof(struct carl9170fw_fix_desc))

#define CHECK_HDR_VERSION(head, _min_ver)                               \
         (((head)->cur_ver < _min_ver) || ((head)->min_ver > _min_ver))  \
		 

#define CARL9170FW_FIX_DESC_MIN_VER                     1
#define CARL9170FW_FIX_DESC_CUR_VER                     2

struct carl9170fw_fix_entry {
         le32_t address;
         le32_t mask;
         le32_t value;
} __attribute__ ((packed));
		 
struct carl9170fw_fix_desc {
         struct ar9170fw_desc_head head;
         struct carl9170fw_fix_entry data[0];
} __attribute__ ((packed));

#define CARL9170FW_DESC_HEAD_SIZE                       \
         (sizeof(struct ar9170fw_desc_head))

#define carl9170fw_for_each_hdr(desc, fw_desc)                          \
         for (desc = fw_desc;                                            \
              memcmp(desc->magic, LAST_MAGIC, CARL9170FW_MAGIC_SIZE) &&  \
              le16_to_cpu(desc->length) >= CARL9170FW_DESC_HEAD_SIZE &&  \
              le16_to_cpu(desc->length) < CARL9170FW_DESC_MAX_LENGTH;    \
              desc = (void *)((unsigned long)desc + le16_to_cpu(desc->length)))		 


inline bool ar9170fw_supports(le32_t list, U8 feature)
{
	return le32_to_cpu(list) & BIT(feature);
}


#endif /* FWDESC_H_ */