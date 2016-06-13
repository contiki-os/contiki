/*
* Copyright (c) 2014, CETIC.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. Neither the name of the Institute nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/

/**
* \file
*         Simple CoAP Library
* \author
*         6LBR Team <6lbr@cetic.be>
*/

#ifndef REST_TYPE_SENML_H
#define REST_TYPE_SENML_H

#define REST_TYPE_SENML_TYPE_ID_JSON 50
#define REST_TYPE_SENML_TYPE_ID_LWM2M 1543

#ifndef REST_TYPE_SENML_TYPE_ID
#define REST_TYPE_SENML_TYPE_ID REST_TYPE_SENML_TYPE_ID_JSON
#endif

extern coap_data_format_t coap_data_format_senml;

#define coap_data_format_senml_REST_TYPE REST_TYPE_SENML_TYPE_ID_JSON


#define REST_FORMAT_BASETIME(buffer, size, pos) \
if (pos < size) { \
	pos += snprintf((char *)buffer + pos, size - pos, ",\"bt\":%u", coap_batch_basetime); \
	if (pos > size) pos = size; \
}

#define REST_FORMAT_TIMESTAMP \
	if(REST_TYPE_SENML_TIMESTAMP) { \
		pos += snprintf((char *)buffer + pos, REST_MAX_CHUNK_SIZE - pos, ",\"t\":%u", clock_seconds() - coap_batch_basetime); \
	} \
	pos += snprintf((char *)buffer + pos, REST_MAX_CHUNK_SIZE - pos, "}")


#define REST_FORMAT_SENML_END(buffer, size, pos) \
	REST_FORMAT_SENML_SQ_BRACKET_END(buffer, size, pos) \
	if(coap_batch_basetime) \
		REST_FORMAT_BASETIME(buffer, size, pos) \
	REST_FORMAT_SENML_CUR_BRACKET_END(buffer, size, pos)

#endif /* REST_TYPE_SENML_H */
