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
#include "compiler.h"
#include "fwdesc.h"
#include "ar9170.h"



#ifndef USB_FW_WRAPPER_H_
#define USB_FW_WRAPPER_H_

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


#define FW_CHUNK_SIZE  4096
#define FW_MINIBOOT_SIZE 12

struct firmware {
	size_t size;
	const U8 *data;
	struct page **pages;
	/* firmware loader private fields */
	void *priv;
};

#define CARL9170FW_MIN_SIZE     32
#define CARL9170FW_MAX_SIZE     16384

inline bool ar9170fw_size_check(unsigned int len)
{
	return (len <= CARL9170FW_MAX_SIZE && len >= CARL9170FW_MIN_SIZE);
}

inline bool ar9170fw_desc_cmp(const struct ar9170fw_desc_head *head,
                                        const U8 descid[CARL9170FW_MAGIC_SIZE],
                                        U16 min_len, U8 compatible_revision)
{
	if (descid[0] == head->magic[0] && descid[1] == head->magic[1] &&
     descid[2] == head->magic[2] && descid[3] == head->magic[3] &&
       !CHECK_HDR_VERSION(head, compatible_revision) &&
         (le16_to_cpu(head->length) >= min_len))
             return true;

    return false;
}

bool ar9170_upload_firmware(struct ar9170* ar);
bool ar9170_upload_firmware_chunk(const uint8_t*, int, le16_t);
struct ar9170fw_desc_head * ar9170_find_fw_desc(struct ar9170 *ar, const U8 *fw_data, const size_t len);
int ar9170_fw_verify_descs(struct ar9170 *ar, const struct ar9170fw_desc_head *head, unsigned int max_len);
void ar9170_fw_info(struct ar9170 *ar);
const void *ar9170_fw_find_desc(struct ar9170 *ar, const U8 descid[4], const unsigned int len, const U8 compatible_revision);
int ar9170_parse_firmware(struct ar9170 *ar);
int ar9170_fw(struct ar9170 *ar, const U8 *data, size_t len);
int ar9170_fw_checksum(struct ar9170 *ar, const U8 *data, size_t len);
int ar9170_fw_tx_sequence(struct ar9170 *ar);
bool valid_dma_addr(const U32 address);
bool valid_cpu_addr(const U32 address);
#endif /* USB_FW_WRAPPER_H_ */
