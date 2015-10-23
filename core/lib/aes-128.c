/* --COPYRIGHT--,BSD
 * Copyright (c) 2011, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/*
 * TI_aes_128_encr_only.c
 *
 *  Created on: Nov 3, 2011
 *      Author: Eric Peeters
 * Updated on: Jun 25, 2015
 * Updated by: Raul Fuentes (RAFS)
 */

/**
 * \file
 *         Wrapped AES-128 implementation from Texas Instruments. 
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 *         Raul Fuentes <ra.fuentess.sam+contiki@gmail.com>
 */

#include "lib/aes-128.h"
#include <string.h>


static const uint8_t sbox[256] =   { 
0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe,
0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4,
0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7,
0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3,
0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 0x09,
0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3,
0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe,
0x39, 0x4a, 0x4c, 0x58, 0xcf, 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92,
0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c,
0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19,
0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14,
0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2,
0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5,
0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, 0xba, 0x78, 0x25,
0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86,
0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e,
0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42,
0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

static const uint8_t inv_sbox[256] = {
0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81,
0xF3, 0xD7, 0xFB, 0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E,
0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB, 0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23,
0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E, 0x08, 0x2E, 0xA1, 0x66,
0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25, 0x72,
0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65,
0xB6, 0x92, 0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46,
0x57, 0xA7, 0x8D, 0x9D, 0x84, 0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A,
0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06, 0xD0, 0x2C, 0x1E, 0x8F, 0xCA,
0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B, 0x3A, 0x91,
0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6,
0x73, 0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8,
0x1C, 0x75, 0xDF, 0x6E, 0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F,
0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B, 0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2,
0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4, 0x1F, 0xDD, 0xA8,
0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93,
0xC9, 0x9C, 0xEF, 0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB,
0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61, 0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6,
0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};


static uint8_t round_keys[11][AES_128_KEY_LENGTH];
static uint8_t Cipher_Block[AES_128_BLOCK_SIZE];

/*---------------------------------------------------------------------------*/
/**
 *  \brief	 Multiplies by 2 in GF(2).
 *  \param value Binary value
 *  \return	 Return the result of GF(Value X2)
 * 
 *               This is a quick operation  for Galois fields GF(2^n).
 *               Created by Konrad Krentz (untouched)
 */
static uint8_t
galois_mul2(uint8_t value)
{
  uint8_t xor_val = (value >> 7) * 0x1b;
  return ((value << 1) ^ xor_val);
}
/*---------------------------------------------------------------------------*/
/**
 *  \brief	 Populated the key slots for the AES-128 operations
 *  \param key   The original key
 * 
 *               Populated the key slots for the AES-128 operations.
 *               Must be called once time  and before the transmission begins. 
 *               Created by Konrad Krentz (untouched). 
 */ 
static void
set_key(const uint8_t *key)
{
  uint8_t i;
  uint8_t j;
  uint8_t rcon;
  
  rcon = 0x01;
  memcpy(round_keys[0], key, AES_128_KEY_LENGTH);
  for(i = 1; i <= 10; i++) {
    round_keys[i][0] = sbox[round_keys[i - 1][13]] ^ round_keys[i - 1][0] ^ rcon;
    round_keys[i][1] = sbox[round_keys[i - 1][14]] ^ round_keys[i - 1][1];
    round_keys[i][2] = sbox[round_keys[i - 1][15]] ^ round_keys[i - 1][2];
    round_keys[i][3] = sbox[round_keys[i - 1][12]] ^ round_keys[i - 1][3];
    for(j = 4; j < AES_128_BLOCK_SIZE; j++) {
      round_keys[i][j] = round_keys[i - 1][j] ^ round_keys[i][j - 4];
    }
    rcon = galois_mul2(rcon);
  }
}
/*---------------------------------------------------------------------------*/
/**
 *  \brief	      Testing of the AES-128. 
 *  \param state      The current 16-bytes block being encrypt.
 *  \param round      Current round. 
 *  \param Event      (String) ID of the event. 
 *  \param key_cipher To display the current state or the current key.
 * 
 *               This function is used for debugging the cipher block procces. 
 *               The Event IDs are the one defined in the Appendix C - Example
 *               Vectors (AES FIP 197).  
 *               This function will only be implemented if the varible 
 *               DEBUG_AES128 is defined in the project-conf.h
 * 
 *               Created by Raul Fuentes. 
 */ 
#ifdef DEBUG_AES128
static void
 debug_rounds(uint8_t *state, uint8_t Round, uint8_t *Event, uint8_t Key_Cipher ){

   uint8_t iC; 
   
    printf("\t Round[%d].%s :\t\t [ ",  Round, Event);  
      for(iC=0; iC < AES_128_BLOCK_SIZE; iC++){
	if (Key_Cipher == 0)
	  printf("%02X", state[iC]);
	else
	  printf("%02X", round_keys[Round][iC]);
      }
    printf(" ]\n");
   return;
}
#endif
/*---------------------------------------------------------------------------*/
/**
 *  \brief	 The encryption process 
 *  \param state The State (block of 16 bytes to encrypt)
 * 
 *               Created by Konrad Krentz, only updated partially by Raul 
 *               Fuentes with the lines of debug_rounds.
 *               The variables names are accord to the AES FIP 197 doc.
 */ 
static void
encrypt(uint8_t *state)
{
  uint8_t buf1, buf2, buf3, buf4, round, i;
  
  #ifdef DEBUG_AES128  
  debug_rounds(state, 0, "input",0);
  #endif
  
  /* round 0 */
  /* AddRoundKey */  
  for(i = 0; i < AES_128_BLOCK_SIZE; i++) {
    state[i] = state[i] ^ round_keys[0][i];
  }
  
  #ifdef DEBUG_AES128  
  debug_rounds(state, 0, "k_sch",1);
  #endif
  
  for(round = 1; round <= 10; round++) {
    
    #ifdef DEBUG_AES128
    debug_rounds(state, round, "start",0);
    #endif
    
    /* ByteSub */
    for(i = 0; i < AES_128_BLOCK_SIZE; i++) {
      state[i] = sbox[state[i]];
    }
    
    #ifdef DEBUG_AES128
    debug_rounds(state, round, "s_box",0);
    #endif
    
    /* ShiftRow 
     */
    buf1 = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = buf1;

    buf1 = state[2];
    buf2 = state[6];
    state[2] = state[10];
    state[6] = state[14];
    state[10] = buf1;
    state[14] = buf2;

    buf1 = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = state[3];
    state[3] = buf1;
    
    #ifdef DEBUG_AES128
    debug_rounds(state, round, "s_row",0);
    #endif
    
    /* last round skips MixColumn */
    if(round < 10) {
      /* MixColumn 
       NOTE RAFS: This is inspired in the work: 
	"Lightweight MixColumn Implementation for AES"
       http://www.wseas.us/e-library/conferences/2009/moscow/AIC/AIC44.pdf*/
      for(i = 0; i < 4; i++) {
        buf4 = (i << 2);
        buf1 = state[buf4] ^ state[buf4 + 1] ^ state[buf4 + 2] ^ state[buf4 + 3];
        buf2 = state[buf4];
        buf3 = state[buf4] ^ state[buf4 + 1];
        buf3 = galois_mul2(buf3);
        
        state[buf4] = state[buf4] ^ buf3 ^ buf1;
        
        buf3 = state[buf4 + 1] ^ state[buf4 + 2];
        buf3 = galois_mul2(buf3);
        state[buf4 + 1] = state[buf4 + 1] ^ buf3 ^ buf1;
        
        buf3 = state[buf4 + 2] ^ state[buf4 + 3];
        buf3 = galois_mul2(buf3);
        state[buf4 + 2] = state[buf4 + 2] ^ buf3 ^ buf1;
        
        buf3 = state[buf4 + 3] ^ buf2;
        buf3 = galois_mul2(buf3);
        state[buf4 + 3] = state[buf4 + 3] ^ buf3 ^ buf1;
      }
      #ifdef DEBUG_AES128
      debug_rounds(state, round, "m_col",0);
      #endif
    }
    
    /* AddRoundKey */
    for(i = 0; i < AES_128_BLOCK_SIZE; i++) {
      state[i] = state[i] ^ round_keys[round][i];
    }
    
    #ifdef DEBUG_AES128
    debug_rounds(state, round, "k_sch",1);
    #endif
  }
  
   #ifdef DEBUG_AES128
   debug_rounds(state, 10, "output",0);
  #endif
}
/*---------------------------------------------------------------------------*/
/**
 *  \brief	 The decryption process 
 *  \param state The State (block of 16 bytes to encrypt)
 * 
 *               Created by Raul Fuentes.
 *               The variables names are accord to the AES FIP 197 doc.
 * 
 *               NOTE: Although exist another alternative to do it,  
 *               the Inverse Cipher approach was used for avoiding modify
 *               the set_key() function. 
 */
static void
decrypt(uint8_t *state)
{
  uint8_t buf1, buf2, buf3, buf4, round, i;
  uint8_t sp1, sp2, sp3;
  
  /*  Inverse Cipher approach (Avoiding touch the expansion key) */
  
   #ifdef DEBUG_AES128
  debug_rounds(state, 10, "input",0);
   #endif
  
    /* AddRoundKey (Initial Round)*/
    for(i = 0; i < AES_128_BLOCK_SIZE; i++) {
      state[i] = state[i] ^ round_keys[10][i];
    }
  
  #ifdef DEBUG_AES128
  debug_rounds(state, 10, "ik_sch",1);
  #endif
  
  //Round 10 - 1
  for(round = 1; round <= 10; round++) {
      
    #ifdef DEBUG_AES128
    debug_rounds(state, 10 - round, "istart",0);
    #endif
    
    /* InvShiftRows 
	The InvShiftRows operation performs right (left in ShiftRows) circular 
	shifts of rows 1, 2, and 3 by 1, 2, and 3.
	*/      
      //Shift one places
      buf1 = state[13];
      state[13] = state[9];
      state[9] = state[5];
      state[5] = state[1];
      state[1] = buf1;
  
      //Shift two places
      buf1 = state[10];
      buf2 = state[14];
      state[10] = state[2];
      state[14] = state[6];
      state[2] = buf1;
      state[6] = buf2;
      
      //Shift three places
      buf1 = state[3];
      state[3] = state[7];
      state[7] = state[11];
      state[11] = state[15];
      state[15] = buf1;
      
      #ifdef DEBUG_AES128
      debug_rounds(state, 10 - round, "is_row",0);
      #endif
      
      /* Inverse ByteSub */
      for(i = 0; i < AES_128_BLOCK_SIZE; i++) {
	state[i] = inv_sbox[state[i]];
      }
      
      #ifdef DEBUG_AES128
      debug_rounds(state, 10 - round, "is_box",0);
      #endif
      
    /* AddRoundKey */
    for(i = 0; i < AES_128_BLOCK_SIZE; i++) {
    state[i] = state[i] ^ round_keys[10 - round][i];
    }
    
    #ifdef DEBUG_AES128
    debug_rounds(state, 10 - round, "ik_sch",1);
    debug_rounds(state, 10 - round, "ik_add",0);
    #endif
    
      /* last round (Round 10) 
       *skips MixColumn */
      if(round < 10) {
	  /* MixColumn based in the Lightweight MixColumn paper
	    */
	  for(i = 0; i < 4; i++) {

	  buf4 = (i << 2); //Recorre la columna 
	  
	  // Tmp*9 =  (((Tmpx2)x2)x2)+Tmp
	  buf1  = state[buf4] ^ state[buf4 + 1] ^ state[buf4 + 2] ^ state[buf4 + 3]; 
	  buf2 = galois_mul2(buf1);
	  buf2 = galois_mul2(buf2);
	  buf1 = galois_mul2(buf2) ^ buf1;

	  //First Row: 
	  //2 X (2 x (state[buf4+0] ^ state[buf4+2] )  )
	  buf2 = state[buf4] ^ state[buf4 + 2];
	  buf2 = galois_mul2(buf2);
	  buf2 = galois_mul2(buf2);
	  
	  // 2 x ( state[buf4+0] ^ state[buf4+1] )
	  buf3 =  state[buf4] ^ state[buf4 + 1];
	  buf3 =  galois_mul2(buf3);
	  
	  //state[buf4] = state[buf4] ^ buf1 ^ buf2 ^ buf3;
	  sp1 = state[buf4] ^ buf1 ^ buf2 ^ buf3;
	  
	  //Second row 
	  buf2 = state[buf4 + 1] ^ state[buf4+3];
	  buf2 = galois_mul2(buf2);
	  buf2 = galois_mul2(buf2);
	  
	  buf3 =  state[buf4 + 1] ^ state[buf4 + 2];
	  buf3 =  galois_mul2(buf3);
	  
	  //state[buf4 + 1] = state[buf4 + 1] ^ buf1 ^ buf2 ^ buf3;
	  sp2 = state[buf4 + 1] ^ buf1 ^ buf2 ^ buf3;
	  
	  //Third row 
	  buf2 = state[buf4] ^ state[buf4 + 2];
	  buf2 = galois_mul2(buf2);
	  buf2 = galois_mul2(buf2);
	  
	  buf3 =  state[buf4 + 2] ^ state[buf4 + 3];
	  buf3 =  galois_mul2(buf3);
	  
	  //state[buf4 + 2] = state[buf4 + 2] ^ buf1 ^ buf2 ^ buf3;
	  sp3 = state[buf4 + 2] ^ buf1 ^ buf2 ^ buf3;
	  
	  //Fourth row 
	  buf2 = state[buf4 + 1] ^ state[buf4+3];
	  buf2 = galois_mul2(buf2);
	  buf2 = galois_mul2(buf2);
	  
	  buf3 =  state[buf4 + 3] ^ state[buf4];
	  buf3 =  galois_mul2(buf3);
	  
	  state[buf4 + 3] = state[buf4 + 3] ^ buf1 ^ buf2 ^ buf3;
	  state[buf4 + 2] = sp3;
	  state[buf4 + 1] = sp2;
	  state[buf4] = sp1;
	  
	}//end-for (MixColumn)
	
      }//end-if
 
  }//end-for (round)
  
  #ifdef DEBUG_AES128
  debug_rounds(state, 0, "output",0);
  #endif
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  \brief	 The encryption process 
 *  \param data  The data to decrypt.
 *  \param len   Data size (in bytes)
 * 
 *               Created by Raul Fuentes.
 * 
 *               NOTE: Although the first lines seem to be for padding aproach 
 *               They are most inclined to simulated the CC2420 hardware 
 *               implementation (Passing the data to a specific 16-bytes register 
 *               and executing the AES-128 process there).
 *               WARNING: Padding was not implemented succesfully, the data 
 *               size must be multiple of 16-bytes
 * 
 */
static void 
data_encrypt(uint8_t *data, int len){
  
  //NOTE: Remember this AES by software not by Hardware. 
  //memset and memcpy only copy partially how the cc24 work
  uint8_t i;
  
  //Break everything in 16 bytes
  for(i = 0; i < len; i = i + AES_128_BLOCK_SIZE) {
    
    
    memset(Cipher_Block, 0x00,  AES_128_BLOCK_SIZE);
    memcpy(Cipher_Block, data +i,MIN(len -i ,AES_128_BLOCK_SIZE));
    
    encrypt(Cipher_Block);
    memcpy(data +i, Cipher_Block, MIN(len -i ,AES_128_BLOCK_SIZE));
    
  }
  return;
}

/*---------------------------------------------------------------------------*/
/**
 *  \brief	 The encryption process 
 *  \param data  The data to decrypt.
 *  \param len   Data size (in bytes)
 * 
 *               Created by Raul Fuentes.
 *               
 *               NOTE: Although the first lines seem to be for padding aproach 
 *               They are most inclined to simulated the CC2420 hardware 
 *               implementation (Passing the data to a specific 16-bytes register 
 *               and executing the AES-128 process there).
 *               WARNING: Padding was not implemented succesfully, the data 
 *               size must be multiple of 16-bytes
 * 
 */
static void 
data_decrypt(uint8_t *data, int len){
  //NOTE: Remember this AES by software not by Hardware. 

  //Break everything in 16 bytes
  uint8_t i;
  for(i = 0; i < len; i = i + AES_128_BLOCK_SIZE) {
    
    memset(Cipher_Block, 0x00,  AES_128_BLOCK_SIZE);
    memcpy(Cipher_Block, data +i, MIN(len -i ,AES_128_BLOCK_SIZE));

    decrypt(Cipher_Block);
    memcpy(data +i, Cipher_Block,  MIN(len -i ,AES_128_BLOCK_SIZE));
    
  }
  return;
}

/*---------------------------------------------------------------------------*/
/**
 *  \brief	 Pads the plaintext with zeroes before calling AES_128.encrypt 
 *  \param data  The data to decrypt.
 *  \param len   Data size (in bytes)
 * 
 *               Original code (untouched by Raul Fuentes)
 *               WARNING: This original aproach for padding will not be 
 *               succesfully if the data size is not multiple of 16-bytes.
 * 
 */
void
aes_128_padded_encrypt(uint8_t *plaintext_and_result, uint8_t plaintext_len)
{
  uint8_t block[AES_128_BLOCK_SIZE];
  
  memset(block, 0, AES_128_BLOCK_SIZE);
  memcpy(block, plaintext_and_result, plaintext_len);
  AES_128.encrypt(block);
  memcpy(plaintext_and_result, block, plaintext_len);
}
/*------------------------------------------------------- --------------------*/
/**
 *  \brief	 Pads the key with zeroes before calling AES_128.set_key  
 *  \param data  The data to decrypt.
 *  \param len   Data size (in bytes)
 * 
 *               Original code (untouched by Raul Fuentes)
 *               WARNING: This original aproach for padding will not be 
 *               succesfully if the data size is not multiple of 16-bytes.
 */
void
aes_128_set_padded_key(uint8_t *key, uint8_t key_len)
{
  uint8_t block[AES_128_BLOCK_SIZE];
  
  memset(block, 0, AES_128_BLOCK_SIZE);
  memcpy(block, key, key_len);
  AES_128.set_key(block);
}


/*---------------------------------------------------------------------------*/
/**
 *  \brief	 AES  functions   
 * 
 *               This structure has all the functions neccesary for  the AES-128 
 *               operations. 
 *               NOTE: data_encrypt and data_decrypt are implemented by  
 *               Raul Fuentes. 
 * 
 *               WARNING: Padding was not implemented succesfully, the data 
 *               size must be multiple of 16-bytes, otherwise the last block 
 *               will not be decry            
 */
const struct aes_128_driver aes_128_driver = {
  set_key,
  encrypt,
  decrypt,
  data_encrypt,
  data_decrypt
};
/*---------------------------------------------------------------------------*/
