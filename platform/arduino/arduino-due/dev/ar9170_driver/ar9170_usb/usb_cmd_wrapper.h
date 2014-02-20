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
#include "ar9170.h"
#include "fwcmd.h"
#include "ar9170_state.h"


#ifndef USB_CMD_WRAPPER_H_
#define USB_CMD_WRAPPER_H_


#define AR9170_MAX_CMD_BUFFER_LEN	8

bool ar9170_echo_test(struct ar9170*);
bool ar9170_eject_mass_storage_device_cmd(void);

bool __ar9170_exec_cmd(struct ar9170_cmd* cmd);
//************************************
// Method:    ar9170_exec_cmd						synchronous command; blocks until the device gets the response
// FullName:  ar9170_exec_cmd
// Access:    public 
// Returns:   bool									returns TRUE if the command has been executed successfully, otherwise false
// Qualifier:
// Parameter: struct ar9170 * ar					the ar9170 device structure
// Parameter: const enum carl9170_cmd_oids cmd		the Command identification number
// Parameter: unsigned int plen						the payload length of the command
// Parameter: void * payload						buffer pointing to the payload of the command
// Parameter: unsigned int outlen					the length of the intended response to this command
// Parameter: void * out							buffer pointing to the response of the command
//************************************
bool ar9170_exec_cmd(struct ar9170* ar, const enum carl9170_cmd_oids cmd, unsigned int plen, void *payload, unsigned int outlen, void* out);


void ar9170_cmd_callback(struct ar9170 *ar, uint32_t len, void *buffer);
int ar9170_read_reg(struct ar9170 *ar, U32 reg, U32 *val);
int ar9170_read_mreg(struct ar9170 *ar, const int nregs, const U32 *regs, U32 *out);
struct ar9170_cmd *ar9170_cmd_buf(struct ar9170 *ar, const enum carl9170_cmd_oids cmd, const unsigned int len);
int ar9170_reboot(struct ar9170 *ar);


//************************************
// Method:    ar9170_write_rgstr
// FullName:  ar9170_write_rgstr
// Access:    public 
// Returns:   int					returns TRUE if the write register command returns successfully
// Qualifier:
// Parameter: struct ar9170 * ar	the ar9170 device structure
// Parameter: const U32 reg			the ar9170 register to write in
// Parameter: const U32 val			the uint32_t value to write in the selected register
//************************************
int ar9170_write_rgstr(struct ar9170 *ar, const U32 reg, const U32 val);
int ar9170_rx_filter(struct ar9170* ar, const unsigned int _rx_filter);
int ar9170_flush_cab(struct ar9170 *ar,const unsigned int vif_id);
int ar9170_bcn_ctrl(struct ar9170 *ar, const unsigned int vif_id, const U32 mode, const U32 addr, const U32 len);
int ar9170_powersave(struct ar9170 *ar, const bool ps);
int ar9170_collect_tally(struct ar9170 *ar);


/*
 * Macros to facilitate writing multiple registers in a single
 * write-combining USB command. Note that when the first group
 * fails the whole thing will fail without any others attempted,
 * but you won't know which write in the group failed.
 */
#define ar9170_regwrite_begin(ar)					\
do {									\
	int __nreg = 0, __err = 0;					\
	struct ar9170 *__ar = ar;

#define ar9170_regwrite(r, v) do {					\
	__ar->cmd_buf[2 * __nreg + 1] = cpu_to_le32(r);			\
	__ar->cmd_buf[2 * __nreg + 2] = cpu_to_le32(v);			\
	__nreg++;							\
	if ((__nreg >= PAYLOAD_MAX / 2)) {				\
		if (IS_ACCEPTING_CMD(__ar))				\
			__err = !ar9170_exec_cmd(__ar,			\
				CARL9170_CMD_WREG, 8 * __nreg,		\
				(U8 *) &__ar->cmd_buf[1], 0, NULL);	\
		else							\
			goto __regwrite_out;				\
									\
		__nreg = 0;						\
		if (__err)						\
			goto __regwrite_out;				\
	}								\
} while (0)

#define ar9170_regwrite_finish()					\
__regwrite_out :							\
	if (__err == 0 && __nreg) {					\
		if (IS_ACCEPTING_CMD(__ar))				\
			__err = !ar9170_exec_cmd(__ar,			\
				CARL9170_CMD_WREG, 8 * __nreg,		\
				(U8 *) &__ar->cmd_buf[1], 0, NULL);	\
		__nreg = 0;						\
	}

#define ar9170_regwrite_result()					\
	__err;								\
} while (0)


#define ar9170_async_regwrite_get_buf()				\
do {									\
	__nreg = 0;							\
	__cmd = ar9170_cmd_buf(__carl, CARL9170_CMD_WREG_ASYNC,	\
				 AR9170_MAX_CMD_PAYLOAD_LEN);		\
	if (__cmd == NULL) {						\
		__err = -ENOMEM;					\
		goto __async_regwrite_out;				\
	}								\
} while (0)

#define ar9170_async_regwrite_begin(carl)				\
do {									\
	struct ar9170 *__carl = carl;					\
	struct ar9170_cmd *__cmd;					\
	unsigned int __nreg;						\
	int  __err = 0;							\
	ar9170_async_regwrite_get_buf();				\

#define ar9170_async_regwrite_flush()					\
do {									\
	if (__cmd == NULL || __nreg == 0)				\
		break;							\
									\
	if (IS_ACCEPTING_CMD(__carl) && __nreg) {			\
		__cmd->hdr.len = 8 * __nreg;				\
		__err = !__ar9170_exec_cmd(__cmd);	\
		__cmd = NULL;						\
		break;							\
	}								\
	goto __async_regwrite_out;					\
} while (0)

#define ar9170_async_regwrite(r, v) do {				\
	if (__cmd == NULL)						\
		ar9170_async_regwrite_get_buf();			\
	__cmd->wreg.regs[__nreg].addr = cpu_to_le32(r);			\
	__cmd->wreg.regs[__nreg].val = cpu_to_le32(v);			\
	__nreg++;							\
	if ((__nreg >= PAYLOAD_MAX / 2))				\
		ar9170_async_regwrite_flush();			\
} while (0)

#define ar9170_async_regwrite_finish() do {				\
__async_regwrite_out:							\
	if (__cmd != NULL && __err == 0)				\
		ar9170_async_regwrite_flush();			\
	free(__cmd);							\
} while (0)								\

#define ar9170_async_regwrite_result()				\
	__err;								\
} while (0)





#endif /* USB_CMD_WRAPPER_H_ */