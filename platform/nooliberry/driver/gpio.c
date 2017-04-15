/*
 * Copyright (c) 2013, NooliTIC
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
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file gpio.c
 *  Provides the functionality for GPIOs
 *
 * \author
 *  Ludovic WIART <ludovic.wiart@noolitic.biz>
 *
 *  History:
 *     19/03/2012 L. Wiart - Created
 *     08/02/2013 S. Dawans - Code Style & Integration in Contiki fork
 */

#include <avr/sfr_defs.h>
#include "contiki-conf.h"
#include <avr/io.h>
#include <gpio.h>

/*---------------------------------------------------------------------------*/
void
gpio_set_output(int gpio)
{
  switch (gpio) {
  case 0:
    sbi(GPIO_0_DDR, GPIO_0_BIT);        //GPIO_0_DDR |= GPIO_0_BIT;
    break;
  case 1:
    sbi(GPIO_1_DDR, GPIO_1_BIT);        //GPIO_1_DDR |= GPIO_1_BIT;
    break;
  case 2:
    sbi(GPIO_2_DDR, GPIO_2_BIT);        //GPIO_2_DDR |= GPIO_2_BIT;
    break;
  case 3:
    sbi(GPIO_3_DDR, GPIO_3_BIT);        //GPIO_3_DDR |= GPIO_3_BIT;
    break;
  case 4:
    sbi(GPIO_4_DDR, GPIO_4_BIT);        //GPIO_4_DDR |= GPIO_4_BIT;
    break;
  case 5:
    sbi(GPIO_5_DDR, GPIO_5_BIT);        //GPIO_5_DDR |= GPIO_5_BIT;
    break;
  case 6:
    sbi(GPIO_6_DDR, GPIO_6_BIT);        //GPIO_6_DDR |= GPIO_6_BIT;
    break;
  case 7:
    sbi(GPIO_7_DDR, GPIO_7_BIT);        //GPIO_7_DDR |= GPIO_7_BIT;
    break;
  case 8:
    sbi(GPIO_8_DDR, GPIO_8_BIT);        //GPIO_8_DDR |= GPIO_8_BIT;
    break;
  case 9:
    sbi(GPIO_1WR_DDR, GPIO_1WR_BIT);    //GPIO_8_DDR |= GPIO_8_BIT;
    break;
  }
}
/*---------------------------------------------------------------------------*/
void
gpio_set_input(int gpio)
{
  switch (gpio) {
  case 0:
    cbi(GPIO_0_DDR, GPIO_0_BIT);        //GPIO_0_DDR += ~GPIO_0_BIT;
    break;
  case 1:
    cbi(GPIO_1_DDR, GPIO_1_BIT);        //GPIO_1_DDR += ~GPIO_1_BIT;
    break;
  case 2:
    cbi(GPIO_2_DDR, GPIO_2_BIT);        //GPIO_2_DDR += ~GPIO_2_BIT;
    break;
  case 3:
    cbi(GPIO_3_DDR, GPIO_3_BIT);        //GPIO_3_DDR &= ~GPIO_3_BIT;
    break;
  case 4:
    cbi(GPIO_4_DDR, GPIO_4_BIT);        //GPIO_4_DDR += ~GPIO_4_BIT;
    break;
  case 5:
    cbi(GPIO_5_DDR, GPIO_5_BIT);        //GPIO_5_DDR += ~GPIO_5_BIT;
    break;
  case 6:
    cbi(GPIO_6_DDR, GPIO_6_BIT);        //GPIO_6_DDR += ~GPIO_6_BIT;
    break;
  case 7:
    cbi(GPIO_7_DDR, GPIO_7_BIT);        //GPIO_7_DDR += ~GPIO_7_BIT;
    break;
  case 8:
    cbi(GPIO_8_DDR, GPIO_8_BIT);        //GPIO_8_DDR += ~GPIO_8_BIT;
    break;
  case 9:
    cbi(GPIO_1WR_DDR, GPIO_1WR_BIT);    //GPIO_8_DDR += ~GPIO_8_BIT;
    break;
  }
}
/*---------------------------------------------------------------------------*/
int
gpio_get(int gpio)
{
  switch (gpio) {
  case 0:
    return bit_is_set(GPIO_0_PIN, GPIO_0_BIT);  //!((GPIO_0_PIN & GPIO_0_BIT)==0);
    break;
  case 1:
    return bit_is_set(GPIO_1_PIN, GPIO_1_BIT);  // !((GPIO_1_PIN & GPIO_1_BIT)==0);
    break;
  case 2:
    return bit_is_set(GPIO_2_PIN, GPIO_2_BIT);  // !((GPIO_2_PIN & GPIO_2_BIT)==0);
    break;
  case 3:
    return bit_is_set(GPIO_3_PIN, GPIO_3_BIT);  // !((GPIO_3_PIN & GPIO_3_BIT)==0);
    break;
  case 4:
    return bit_is_set(GPIO_4_PIN, GPIO_4_BIT);  // !((GPIO_4_PIN & GPIO_4_BIT)==0);
    break;
  case 5:
    return bit_is_set(GPIO_5_PIN, GPIO_5_BIT);  // !((GPIO_5_PIN & GPIO_5_BIT)==0);
    break;
  case 6:
    return bit_is_set(GPIO_6_PIN, GPIO_6_BIT);  // !((GPIO_6_PIN & GPIO_6_BIT)==0);
    break;
  case 7:
    return bit_is_set(GPIO_7_PIN, GPIO_7_BIT);  // !((GPIO_7_PIN & GPIO_7_BIT)==0);
    break;
  case 8:
    return bit_is_set(GPIO_8_PIN, GPIO_8_BIT);  //  !((GPIO_8_PIN & GPIO_8_BIT)==0);
    break;
  case 9:
    return bit_is_set(GPIO_1WR_PIN, GPIO_1WR_BIT);
    break;
  }
}
/*---------------------------------------------------------------------------*/
void
gpio_set_on(int gpio)
{
  switch (gpio) {
  case 0:
    sbi(GPIO_0_PORT, GPIO_0_BIT);       //GPIO_0_PORT |= GPIO_0_BIT;
    break;
  case 1:
    sbi(GPIO_1_PORT, GPIO_1_BIT);       //GPIO_1_PORT |= GPIO_1_BIT;
    break;
  case 2:
    sbi(GPIO_2_PORT, GPIO_2_BIT);       //GPIO_2_PORT |= GPIO_2_BIT;
    break;
  case 3:
    sbi(GPIO_3_PORT, GPIO_3_BIT);       //GPIO_3_PORT |= GPIO_3_BIT;
    break;
  case 4:
    sbi(GPIO_4_PORT, GPIO_4_BIT);       //GPIO_4_PORT |= GPIO_4_BIT;
    break;
  case 5:
    sbi(GPIO_5_PORT, GPIO_5_BIT);       //GPIO_5_PORT |= GPIO_5_BIT;
    break;
  case 6:
    sbi(GPIO_6_PORT, GPIO_6_BIT);       //GPIO_6_PORT |= GPIO_6_BIT;
    break;
  case 7:
    sbi(GPIO_7_PORT, GPIO_7_BIT);       //GPIO_7_PORT |= GPIO_7_BIT;
    break;
  case 8:
    sbi(GPIO_8_PORT, GPIO_8_BIT);       //GPIO_8_PORT |= GPIO_8_BIT;
    break;
  case 9:
    sbi(GPIO_1WR_PORT, GPIO_1WR_BIT);
    break;
  }
}
/*---------------------------------------------------------------------------*/
void
gpio_set_off(int gpio)
{
  switch (gpio) {
  case 0:
    cbi(GPIO_0_PORT, GPIO_0_BIT);       //GPIO_0_PORT ^= GPIO_0_BIT;
    break;
  case 1:
    cbi(GPIO_1_PORT, GPIO_1_BIT);       //GPIO_1_PORT ^= GPIO_1_BIT;
    break;
  case 2:
    cbi(GPIO_2_PORT, GPIO_2_BIT);       //GPIO_2_PORT ^= GPIO_2_BIT;
    break;
  case 3:
    cbi(GPIO_3_PORT, GPIO_3_BIT);       //GPIO_3_PORT ^= GPIO_3_BIT;
    break;
  case 4:
    cbi(GPIO_4_PORT, GPIO_4_BIT);       //GPIO_4_PORT ^= GPIO_4_BIT;
    break;
  case 5:
    cbi(GPIO_5_PORT, GPIO_5_BIT);       //GPIO_5_PORT ^= GPIO_5_BIT;
    break;
  case 6:
    cbi(GPIO_6_PORT, GPIO_6_BIT);       //GPIO_6_PORT ^= GPIO_6_BIT;
    break;
  case 7:
    cbi(GPIO_7_PORT, GPIO_7_BIT);       //GPIO_7_PORT ^= GPIO_7_BIT;
    break;
  case 8:
    cbi(GPIO_8_PORT, GPIO_8_BIT);       //GPIO_8_PORT ^= GPIO_8_BIT;
    break;
  case 9:
    cbi(GPIO_1WR_PORT, GPIO_1WR_BIT);
    break;
  }
}
/*---------------------------------------------------------------------------*/
void
gpio_toggle(int gpio)
{
  switch (gpio) {
  case 0:
    sbi(GPIO_0_PIN, GPIO_0_BIT);        //GPIO_0_PIN |= GPIO_0_BIT;
    break;
  case 1:
    sbi(GPIO_1_PIN, GPIO_1_BIT);        //GPIO_1_PIN |= GPIO_1_BIT;
    break;
  case 2:
    sbi(GPIO_2_PIN, GPIO_2_BIT);        //GPIO_2_PIN |= GPIO_2_BIT;
    break;
  case 3:
    sbi(GPIO_3_PIN, GPIO_3_BIT);        //GPIO_3_PIN |= GPIO_3_BIT;
    break;
  case 4:
    sbi(GPIO_4_PIN, GPIO_4_BIT);        //GPIO_4_PIN |= GPIO_4_BIT;
    break;
  case 5:
    sbi(GPIO_5_PIN, GPIO_5_BIT);        //GPIO_5_PIN |= GPIO_5_BIT;
    break;
  case 6:
    sbi(GPIO_6_PIN, GPIO_6_BIT);        //GPIO_6_PIN |= GPIO_6_BIT;
    break;
  case 7:
    sbi(GPIO_7_PIN, GPIO_7_BIT);        //GPIO_7_PIN |= GPIO_7_BIT;
    break;
  case 8:
    sbi(GPIO_8_PIN, GPIO_8_BIT);        //GPIO_8_PIN |= GPIO_8_BIT;
    break;
  case 9:
    sbi(GPIO_1WR_PIN, GPIO_1WR_BIT);    //GPIO_8_PIN |= GPIO_8_BIT;
    break;
  }
}
/*---------------------------------------------------------------------------*/
void
ngpio_init(void)
{

}

/*---------------------------------------------------------------------------*/
