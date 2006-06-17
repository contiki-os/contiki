/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 *
 * @(#)$Id: init.c,v 1.1 2006/06/17 22:41:35 adamdunkels Exp $
 */
#include "init.h"
#include "random.h"

void
init(void)
{
  int x, y;

  ether_set_strength(21);

  main_add_node(1, 1);
  main_add_node(1, 20);
  main_add_node(20, 1);

  /*for(x = 1; x < 201; x += 20) {
    for(y = 1; y < 201; y += 20) {
	main_add_node(x, y);
    }
  }*/

  
  /*  main_add_node(10, 10);
  main_add_node(11, 11);
  main_add_node(12, 12);
  main_add_node(13, 13);
  main_add_node(14, 14);
  main_add_node(15, 15);
  main_add_node(16, 16); */
  
  /*  for(x = 10; x < 370; x += 20) {
    for(y = 10; y < 370; y += 20) {
      main_add_node(x + (random_rand() % 20) - 10,
		    y + (random_rand() % 20) - 10);
    }
    }*/
  
  /*  main_add_base(1, 1);*/
}
