/**
 * Copyright (c) 2013, Calipso project consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the Calipso nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/
#include "usb_fw_wrapper.h"
#include "usb_wrapper.h"
#include "usb_lock.h"
#include "uhi_vendor.h"
#include "sleepmgr.h"
#include "delay.h"
#include <stddef.h>
#include <sys\errno.h>
#include "cfg80211.h"
#include "mac80211.h"
#include "fwdesc.h"
#include "bitops.h"
#include "ar9170_debug.h"


static const U8 otus_magic[4] = { OTUS_MAGIC };
	
bool ar9170_upload_firmware( struct ar9170* ar )
{	 
	bool transfer_result;
	#if USB_FW_WRAPPER_DEBUG
	int ii;
	printf("DEBUG : Uploading Firmware...\n");
	#endif
	
	bool firmware_upload_completed_flag = false; 
	
	int remaining_length = ar->fw.fw->size - FW_MINIBOOT_SIZE;//firmware_image_len - FW_MINIBOOT_SIZE;
	int firmware_index = FW_MINIBOOT_SIZE;
	
	int transfer_size;
	uint32_t addr = 0x200000; // Initial address offset
	
	while(firmware_upload_completed_flag == false && remaining_length > 0)  {
		
		// Decide which data chunk to send
		transfer_size = min(remaining_length, FW_CHUNK_SIZE);
		#if USB_FW_WRAPPER_DEBUG_DEEP
		printf("DEBUG: Transferring %d bytes: \n",transfer_size);		
		for (ii=0; ii<transfer_size; ii++) {
			printf("%2x ",firmware_image[firmware_index + ii]);
		}
		printf(" \n");
		#endif
		
		// Check critical section
		while (ar9170_usb_is_semaphore_locked()) {
			sleepmgr_enter_sleep();
		}	
		// Send the data chunk
		transfer_result = ar9170_upload_firmware_chunk(&ar->fw.fw->data[firmware_index], transfer_size, cpu_to_le16(addr >> 8));
		
		// Investigate result of transfer
		if (transfer_result == false) {
			printf("ERROR: Firmware could not be uploaded.\n");
			return false;
		}

		delay_ms(50);
		
		// Update firmware image index
		firmware_index += transfer_size;
		// Update remaining length
		remaining_length -= transfer_size;
		// Update address offset
		addr += transfer_size;
		#if USB_FW_WRAPPER_DEBUG
		printf("DEBUG: Remaining Length: %d. Address Offset: %6x. Truncated: %4x.\n", remaining_length, (unsigned int)addr, (unsigned int)(cpu_to_le16(addr >> 8)));
		#endif
	}
	
	// Start waiting for device to boot
	__start(&ar->fw_boot_wait);
	
	// Send "firmware complete" message
	ar9170_usb_ctrl_out_lock();
	transfer_result = uhi_vendor_control_out_firmware_upload(NULL, 0, 0x31, 0, ar9170_control_out_transfer_done);
	  
	  //uhi_vendor_control_out_firmware_upload(&firmware_image[0]    , 4096, 0x30, 0x00, my_callback_control_out_done);
	  //uhi_vendor_control_out_firmware_upload(&firmware_image[4096] , 4096, 0x30, 0x10, my_callback_control_out_done);
	  //uhi_vendor_control_out_firmware_upload(&firmware_image[8192] , 4096, 0x30, 0x20, my_callback_control_out_done);
	  //uhi_vendor_control_out_firmware_upload(&firmware_image[12288],  756, 0x30, 0x30, my_callback_control_out_done);
	  // There is an offset of 12 bytes present in the Linux Driver to avoid uploading a Mini Boot image... (?)
	  // 	  uhi_vendor_control_out_firmware_upload(&firmware_image[12]   , 4096, 0x30, 0x00, my_callback_control_out_done);
	  // 	  uhi_vendor_control_out_firmware_upload(&firmware_image[4108] , 4096, 0x30, 0x10, my_callback_control_out_done);
	  // 	  uhi_vendor_control_out_firmware_upload(&firmware_image[8204] , 4096, 0x30, 0x20, my_callback_control_out_done);
	  // 	  uhi_vendor_control_out_firmware_upload(&firmware_image[12300],  744, 0x30, 0x30, my_callback_control_out_done);
	  //uhi_vendor_control_out_firmware_upload(NULL, 0, 0x31, 0, my_callback_control_out_done);
	if (transfer_result == true) {
		#if USB_FW_WRAPPER_DEBUG
		printf("DEBUG : Firmware Uploaded to device.\n");
		#endif
	}		
	else
		printf("ERROR: Firmware complete command returned errors.\n");
	
	// Wait for device response
	__wait_for_completion(&ar->fw_boot_wait);
	
	return transfer_result;
}
  
  
  
bool ar9170_upload_firmware_chunk(const uint8_t* buffer, int length, le16_t fw_addr) {
	  
	// Enter critical section
	ar9170_usb_ctrl_out_lock();
	  
	// Send a block of data through the control endpoint	  
	return uhi_vendor_control_out_firmware_upload( buffer, length, 0x30, fw_addr, ar9170_control_out_transfer_done);
  }
  


struct ar9170fw_desc_head *
ar9170_find_fw_desc(struct ar9170 *ar, const U8 *fw_data, const size_t len)
{
         int scan = 0, found = 0;

         if (!ar9170fw_size_check(len)) {
                 printf("ERROR: Firmware size is out of bound!\n");
                 return NULL;
         }

         while (scan < len - sizeof(struct ar9170fw_desc_head)) {
                 if (fw_data[scan++] == otus_magic[found])
                         found++;
                 else
                         found = 0;

                 if (scan >= len)
                         break;

                 if (found == sizeof(otus_magic))
                         break;
         }

         if (found != sizeof(otus_magic))
                 return NULL;

         return (void *)&fw_data[scan - found];
}


int ar9170_parse_firmware(struct ar9170 *ar)
{
	const struct ar9170fw_desc_head *fw_desc = NULL;
	const struct firmware *fw = ar->fw.fw;
	unsigned long header_offset = 0;
	int err;
	
	if (!fw) {
		printf("ERROR: Firmware struct is null.\n");
        return -EINVAL;
	}	
	fw_desc = ar9170_find_fw_desc(ar, fw->data, fw->size);
	
	if (!fw_desc) {
		printf("ERROR: Unsupported firmware.\n");
        return -ENODATA;
	}
	
	header_offset = (unsigned long)fw_desc - (unsigned long)fw->data;
	
	err = ar9170_fw_verify_descs(ar, fw_desc, fw->size - header_offset);
	if (err) {
		printf("ERROR: damaged firmware (%d).\n", err);
		return err;
	}
	
    ar->fw.desc = fw_desc;
	
	ar9170_fw_info(ar);
	
	err = ar9170_fw(ar, fw->data, fw->size);
	if (err) {
		printf("ERROR: failed to parse firmware.\n");
		return err;
	}
	 
	return 0;
}


int ar9170_fw_verify_descs(struct ar9170 *ar,
const struct ar9170fw_desc_head *head, unsigned int max_len)
{
	const struct ar9170fw_desc_head *pos;
    unsigned long pos_addr, end_addr;
    unsigned int pos_length;

    if (max_len < sizeof(*pos))
            return -ENODATA;

    max_len = min((unsigned int)CARL9170FW_DESC_MAX_LENGTH, (unsigned int)max_len);

    pos = head;
    pos_addr = (unsigned long) pos;
    end_addr = pos_addr + max_len;

    while (pos_addr < end_addr) {
       if (pos_addr + sizeof(*head) > end_addr)
            return -E2BIG;

       pos_length = le16_to_cpu(pos->length);

       if (pos_length < sizeof(*head))
            return -EBADMSG;

       if (pos_length > max_len)
            return -EOVERFLOW;

       if (pos_addr + pos_length > end_addr)
			return -EMSGSIZE;

       if (ar9170fw_desc_cmp(pos, LAST_MAGIC,
                                  CARL9170FW_LAST_DESC_SIZE,
                                  CARL9170FW_LAST_DESC_CUR_VER))
			return 0;

       pos_addr += pos_length;
       pos = (void *)pos_addr;
       max_len -= pos_length;
   }
   return -EINVAL;
}



void ar9170_fw_info(struct ar9170 *ar)
{
         const struct carl9170fw_motd_desc *motd_desc;
         unsigned int str_ver_len;
         U32 fw_date;

         printf("DEBUG: driver   API: %s 2%03d-%02d-%02d [%d-%d]\n",
                 CARL9170FW_VERSION_GIT, CARL9170FW_VERSION_YEAR,
                 CARL9170FW_VERSION_MONTH, CARL9170FW_VERSION_DAY,
                 AR9170FW_API_MIN_VER, AR9170FW_API_MAX_VER);

         motd_desc = ar9170_fw_find_desc(ar, MOTD_MAGIC,
                sizeof(*motd_desc), CARL9170FW_MOTD_DESC_CUR_VER);
        if (motd_desc) {
                 str_ver_len = strnlen(motd_desc->release,
                         CARL9170FW_MOTD_RELEASE_LEN);

                 fw_date = le32_to_cpu(motd_desc->fw_year_month_day);
                 printf("DEBUG: firmware API: %.*s 2%03d-%02d-%02d\n",
                          str_ver_len, motd_desc->release,
                          CARL9170FW_GET_YEAR(fw_date),
                          CARL9170FW_GET_MONTH(fw_date),
                          CARL9170FW_GET_DAY(fw_date));

                 strlcpy(ar->hw->wiphy->fw_version, motd_desc->release,
                         sizeof(ar->hw->wiphy->fw_version));
         }
}  


const void *ar9170_fw_find_desc(struct ar9170 *ar, const U8 descid[4],
        const unsigned int len, const U8 compatible_revision)
{
         const struct ar9170fw_desc_head *iter;

         carl9170fw_for_each_hdr(iter, ar->fw.desc) {
                 if (ar9170fw_desc_cmp(iter, descid, len,
                                         compatible_revision))
                         return (void *)iter;
         }

         /* needed to find the LAST desc */
         if (ar9170fw_desc_cmp(iter, descid, len,
                                 compatible_revision))
                 return (void *)iter;

         return NULL;
}



int ar9170_fw(struct ar9170 *ar, const U8 *data, size_t len)
{
	const struct carl9170fw_otus_desc *otus_desc;
    int err;
    U16 if_comb_types;

    err = ar9170_fw_checksum(ar, data, len);
    if (err)
		return err;

         otus_desc = ar9170_fw_find_desc(ar, OTUS_MAGIC,
                 sizeof(*otus_desc), CARL9170FW_OTUS_DESC_CUR_VER);
         if (!otus_desc) {
                 return -ENODATA;
         }

#define SUPP(feat)                                              \
         (ar9170fw_supports(otus_desc->feature_set, feat))

         if (!SUPP(CARL9170FW_DUMMY_FEATURE)) {
                 printf("ERROR: invalid firmware descriptor "
                         "format detected.\n");
                 return -EINVAL;
         }

         ar->fw.api_version = otus_desc->api_ver;

         if (ar->fw.api_version < AR9170FW_API_MIN_VER ||
             ar->fw.api_version > AR9170FW_API_MAX_VER) {
                 printf("ERROR: unsupported firmware api version.\n");
                 return -EINVAL;
         }

         if (!SUPP(CARL9170FW_COMMAND_PHY) || SUPP(CARL9170FW_UNUSABLE) ||
             !SUPP(CARL9170FW_HANDLE_BACK_REQ)) {
                 printf("ERROR: firmware does support "
                         "mandatory features.\n");
                 return -ECANCELED;
         }

         if (ilog2(le32_to_cpu(otus_desc->feature_set)) >=
                 __CARL9170FW_FEATURE_NUM) {
                 printf("WARNING: driver does not support all "
                          "firmware features.\n");
         }

         if (!SUPP(CARL9170FW_COMMAND_CAM)) {
                 printf("DEBUG: crypto offloading is disabled "
                          "by firmware.\n");
                 ar->disable_offload = true;
         }

         if (SUPP(CARL9170FW_PSM) && SUPP(CARL9170FW_FIXED_5GHZ_PSM))
                 ar->hw->flags |= IEEE80211_HW_SUPPORTS_PS;

         if (!SUPP(CARL9170FW_USB_INIT_FIRMWARE)) {
                 printf("ERROR: firmware does not provide "
                         "mandatory interfaces.\n");
                 return -EINVAL;
         }

         if (SUPP(CARL9170FW_MINIBOOT))
                 ar->fw.offset = le16_to_cpu(otus_desc->miniboot_size);
         else
                 ar->fw.offset = 0;

         if (SUPP(CARL9170FW_USB_DOWN_STREAM)) {
                 ar->hw->extra_tx_headroom += sizeof(struct ar9170_stream);
                 ar->fw.tx_stream = true;
         }

         if (SUPP(CARL9170FW_USB_UP_STREAM))
                 ar->fw.rx_stream = true;

         if (SUPP(CARL9170FW_RX_FILTER)) {
                 ar->fw.rx_filter = true;
                 ar->rx_filter_caps = FIF_FCSFAIL | FIF_PLCPFAIL |
                         FIF_CONTROL | FIF_PSPOLL | FIF_OTHER_BSS |
                         FIF_PROMISC_IN_BSS;
         }

         if (SUPP(CARL9170FW_HW_COUNTERS))
                 ar->fw.hw_counters = true;

         if (SUPP(CARL9170FW_WOL))
                 // FIXME device_set_wakeup_enable(&ar->udev->dev, true);

         if (SUPP(CARL9170FW_RX_BA_FILTER))
                 ar->fw.ba_filter = true;

         if_comb_types = BIT(NL80211_IFTYPE_STATION) |
                         BIT(NL80211_IFTYPE_P2P_CLIENT);

         ar->fw.vif_num = otus_desc->vif_num;
         ar->fw.cmd_bufs = otus_desc->cmd_bufs;
         ar->fw.address = le32_to_cpu(otus_desc->fw_address);
         ar->fw.rx_size = le16_to_cpu(otus_desc->rx_max_frame_len);
         ar->fw.mem_blocks = min((unsigned int)otus_desc->tx_descs, (unsigned int)0xfe);
         ar->mem_free_blocks = ar->fw.mem_blocks;//atomic_set(&ar->mem_free_blocks, ar->fw.mem_blocks);
         ar->fw.mem_block_size = le16_to_cpu(otus_desc->tx_frag_len);

         if (ar->fw.vif_num >= AR9170_MAX_VIRTUAL_MAC || !ar->fw.vif_num ||
             ar->fw.mem_blocks < 16 || !ar->fw.cmd_bufs ||
             ar->fw.mem_block_size < 64 || ar->fw.mem_block_size > 512 ||
             ar->fw.rx_size > 32768 || ar->fw.rx_size < 4096 ||
             !valid_cpu_addr(ar->fw.address)) {
                 printf("ERROR: firmware shows obvious signs of "
                         "malicious tampering.\n");
                 return -EINVAL;
         }

         ar->fw.beacon_addr = le32_to_cpu(otus_desc->bcn_addr);
         ar->fw.beacon_max_len = le16_to_cpu(otus_desc->bcn_len);

         if (valid_dma_addr(ar->fw.beacon_addr) && ar->fw.beacon_max_len >=
             AR9170_MAC_BCN_LENGTH_MAX) {
                 ar->hw->wiphy->interface_modes |= BIT(NL80211_IFTYPE_ADHOC);

                 if (SUPP(CARL9170FW_WLANTX_CAB)) {
                         if_comb_types |=
                                 BIT(NL80211_IFTYPE_AP) |
                                 BIT(NL80211_IFTYPE_P2P_GO);

#ifdef CONFIG_MAC80211_MESH
                         if_comb_types |=
                                 BIT(NL80211_IFTYPE_MESH_POINT);
#endif /* CONFIG_MAC80211_MESH */
                 }
         }

         ar->if_comb_limits[0].max = ar->fw.vif_num;
         ar->if_comb_limits[0].types = if_comb_types;

         ar->if_combs[0].num_different_channels = 1;
         ar->if_combs[0].max_interfaces = ar->fw.vif_num;
         ar->if_combs[0].limits = ar->if_comb_limits;
         ar->if_combs[0].n_limits = ARRAY_SIZE(ar->if_comb_limits);

         ar->hw->wiphy->iface_combinations = ar->if_combs;
         ar->hw->wiphy->n_iface_combinations = ARRAY_SIZE(ar->if_combs);

         ar->hw->wiphy->interface_modes |= if_comb_types;

         ar->hw->wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;

#undef SUPPORTED
         return ar9170_fw_tx_sequence(ar);
}


int ar9170_fw_checksum(struct ar9170 *ar, const U8 *data,
                                 size_t len)
{
         const struct carl9170fw_otus_desc *otus_desc;
         const struct carl9170fw_last_desc *last_desc;
         const struct carl9170fw_chk_desc *chk_desc;
         unsigned long fin, diff;
         unsigned int dsc_len;
         U32 crc32;
		 UNUSED(dsc_len);
		 UNUSED(crc32);
         last_desc = ar9170_fw_find_desc(ar, LAST_MAGIC,
                 sizeof(*last_desc), CARL9170FW_LAST_DESC_CUR_VER);
         if (!last_desc)
                 return -EINVAL;

         otus_desc = ar9170_fw_find_desc(ar, OTUS_MAGIC,
                 sizeof(*otus_desc), CARL9170FW_OTUS_DESC_CUR_VER);
         if (!otus_desc) {
                 printf("ERROR:failed to find compatible firmware "
                         "descriptor.\n");
                 return -ENODATA;
         }

         chk_desc = ar9170_fw_find_desc(ar, CHK_MAGIC,
                 sizeof(*chk_desc), CARL9170FW_CHK_DESC_CUR_VER);

         if (!chk_desc) {
                 printf("WARNING: Unprotected firmware image.\n");
                 return 0;
         }

         dsc_len = min((unsigned int)len,
                         (unsigned long)chk_desc - (unsigned long)otus_desc);

         fin = (unsigned long) last_desc + sizeof(*last_desc);
         diff = fin - (unsigned long) otus_desc;

         if (diff < len)
                 len -= diff;

         if (len < 256)
                 return -EIO;
/*	FIXME - this is the checksum check and should normally be implemented
         crc32 = crc32_le(~0, data, len);
         if (cpu_to_le32(crc32) != chk_desc->fw_crc32) {
                 printf("ERROR: fw checksum test failed.\n");
                 return -ENOEXEC;
         }

         crc32 = crc32_le(crc32, (void *)otus_desc, dsc_len);
         if (cpu_to_le32(crc32) != chk_desc->hdr_crc32) {
                 printf("ERROR: descriptor check failed.\n");
                 return -EINVAL;
         }*/
         return 0;
}



int ar9170_fw_tx_sequence(struct ar9170 *ar)
{
         const struct carl9170fw_txsq_desc *txsq_desc;

         txsq_desc = ar9170_fw_find_desc(ar, TXSQ_MAGIC, sizeof(*txsq_desc),
                                           CARL9170FW_TXSQ_DESC_CUR_VER);
         if (txsq_desc) {
                 ar->fw.tx_seq_table = le32_to_cpu(txsq_desc->seq_table_addr);
                 if (!valid_cpu_addr(ar->fw.tx_seq_table))
                         return -EINVAL;
         } else {
                 ar->fw.tx_seq_table = 0;
         }

         return 0;
}

bool valid_dma_addr(const U32 address)
{
         if (address >= AR9170_SRAM_OFFSET &&
             address < (AR9170_SRAM_OFFSET + AR9170_SRAM_SIZE))
                 return true;

         return false;
}

bool valid_cpu_addr(const U32 address)
{
         if (valid_dma_addr(address) || (address >= AR9170_PRAM_OFFSET &&
             address < (AR9170_PRAM_OFFSET + AR9170_PRAM_SIZE)))
                 return true;

         return false;
}