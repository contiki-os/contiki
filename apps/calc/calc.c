/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This an example program for the Contiki desktop OS
 *
 *
 */

#include <stddef.h>

#include "contiki.h"
#include "ctk/ctk.h"

static struct ctk_window window;

static char input[16];
static struct ctk_label inputlabel =
  {CTK_LABEL(0, 0, 12, 1, input)};

static struct ctk_button button7 =
  {CTK_BUTTON(0, 3, 1, "7")};
static struct ctk_button button8 =
  {CTK_BUTTON(3, 3, 1, "8")};
static struct ctk_button button9 =
  {CTK_BUTTON(6, 3, 1, "9")};
static struct ctk_button button4 =
  {CTK_BUTTON(0, 4, 1, "4")};
static struct ctk_button button5 =
  {CTK_BUTTON(3, 4, 1, "5")};
static struct ctk_button button6 =
  {CTK_BUTTON(6, 4, 1, "6")};
static struct ctk_button button1 =
  {CTK_BUTTON(0, 5, 1, "1")};
static struct ctk_button button2 =
  {CTK_BUTTON(3, 5, 1, "2")};
static struct ctk_button button3 =
  {CTK_BUTTON(6, 5, 1, "3")};
static struct ctk_button button0 =
  {CTK_BUTTON(0, 6, 3, " 0 ")};

static struct ctk_button cbutton =
  {CTK_BUTTON(0, 2, 1, "C")};

static struct ctk_button divbutton =
  {CTK_BUTTON(9, 2, 1, "/")};
static struct ctk_button mulbutton =
  {CTK_BUTTON(9, 3, 1, "*")};

static struct ctk_button subbutton =
  {CTK_BUTTON(9, 4, 1, "-")};
static struct ctk_button addbutton =
  {CTK_BUTTON(9, 5, 1, "+")};
static struct ctk_button calcbutton =
  {CTK_BUTTON(9, 6, 1, "=")};

PROCESS(calc_process, "Calculator");

AUTOSTART_PROCESSES(&calc_process);

static unsigned long operand1, operand2;
static unsigned char op;
#define OP_ADD 1
#define OP_SUB 2
#define OP_MUL 3
#define OP_DIV 4

/*-----------------------------------------------------------------------------------*/
static void
calc_quit(void)
{
  process_exit(&calc_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
add_to_input(char c)
{
  unsigned char i;
    
  for(i = 0; i < 11; ++i) {
    input[i] = input[i + 1];
  }
  input[11] = c;
}
/*-----------------------------------------------------------------------------------*/
static void
clear_input(void)
{
  unsigned char i;
  
  for(i = 0; i < sizeof(input); ++i) {
    input[i] = ' ';
  }      
}
/*-----------------------------------------------------------------------------------*/
static void
input_to_operand1(void)
{
  unsigned int m;
  unsigned char i;

  operand1 = 0;
  for(m = 1, i = 11;
      i > 7; --i, m *= 10) {
    if(input[i] >= '0' &&
       input[i] <= '9') {
      operand1 += (input[i] - '0') * m;
    }
  }
  clear_input();
}
/*-----------------------------------------------------------------------------------*/
static void
operand2_to_input(void)
{
  unsigned char i;
  
  input[7] = (char)((operand2/10000) % 10) + '0';
  input[8] = (char)((operand2/1000) % 10) + '0';
  input[9] = (char)((operand2/100) % 10) + '0';
  input[10] = (char)((operand2/10) % 10) + '0';
  input[11] = (char)(operand2 % 10) + '0';

  for(i = 0; i < 4; ++i) {
    if(input[7 + i] == '0') {
      input[7 + i] = ' ';
    } else {
      break;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static void
calculate(void)
{
  operand2 = operand1;
  input_to_operand1();
  switch(op) {
  case OP_ADD:
    operand2 = operand2 + operand1;
    break;
  case OP_SUB:
    operand2 = operand2 - operand1;
    break;
  case OP_MUL:
    operand2 = operand2 * operand1;
    break;
  case OP_DIV:
    operand2 = operand2 / operand1;
    break;
  }
  operand2_to_input();      
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(calc_process, ev, data)
{
  PROCESS_BEGIN();
  
  ctk_window_new(&window, 12, 7, "Calc");

  CTK_WIDGET_ADD(&window, &inputlabel);
  CTK_WIDGET_SET_FLAG(&inputlabel, CTK_WIDGET_FLAG_MONOSPACE);

  CTK_WIDGET_ADD(&window, &cbutton);
    
  CTK_WIDGET_ADD(&window, &divbutton);
	    
  CTK_WIDGET_ADD(&window, &button7);
  CTK_WIDGET_ADD(&window, &button8);
  CTK_WIDGET_ADD(&window, &button9);

  CTK_WIDGET_ADD(&window, &mulbutton);

   

  CTK_WIDGET_ADD(&window, &button4);
  CTK_WIDGET_ADD(&window, &button5);
  CTK_WIDGET_ADD(&window, &button6);

  CTK_WIDGET_ADD(&window, &subbutton);

  CTK_WIDGET_ADD(&window, &button1);
  CTK_WIDGET_ADD(&window, &button2);
  CTK_WIDGET_ADD(&window, &button3);
    
  CTK_WIDGET_ADD(&window, &addbutton);
    
  CTK_WIDGET_ADD(&window, &button0);

  CTK_WIDGET_ADD(&window, &calcbutton);
	
  clear_input();
    
  ctk_window_open(&window);

  while(1) {

    PROCESS_WAIT_EVENT();

    if(ev == ctk_signal_keypress) {
      if((char)(size_t)data >= '0' &&
	 (char)(size_t)data <= '9') {
	add_to_input((char)(size_t)data);
      } else if((char)(size_t)data == ' ') {
	clear_input();
      } else if((char)(size_t)data == '+') {
	input_to_operand1();
	op = OP_ADD;
      } else if((char)(size_t)data == '-') {
	input_to_operand1();
	op = OP_SUB;
      } else if((char)(size_t)data == '*') {
	input_to_operand1();
	op = OP_MUL;
      } else if((char)(size_t)data == '/') {
	input_to_operand1();
	op = OP_DIV;
      } else if((char)(size_t)data == '=' ||
		(char)(size_t)data == CH_ENTER) {
	calculate();
      }
	
      CTK_WIDGET_REDRAW(&inputlabel);
    } else if(ev == ctk_signal_button_activate) {
      if(data == (process_data_t)&button0) {
	add_to_input('0');
      } else if(data == (process_data_t)&button1) {
	add_to_input('1');
      } else if(data == (process_data_t)&button2) {
	add_to_input('2');
      } else if(data == (process_data_t)&button3) {
	add_to_input('3');
      } else if(data == (process_data_t)&button4) {
	add_to_input('4');
      } else if(data == (process_data_t)&button5) {
	add_to_input('5');
      } else if(data == (process_data_t)&button6) {
	add_to_input('6');
      } else if(data == (process_data_t)&button7) {
	add_to_input('7');
      } else if(data == (process_data_t)&button8) {
	add_to_input('8');
      } else if(data == (process_data_t)&button9) {
	add_to_input('9');
      } else if(data == (process_data_t)&cbutton) {
	clear_input();
      } else if(data == (process_data_t)&calcbutton) {
	calculate();
      } else if(data == (process_data_t)&addbutton) {
	input_to_operand1();
	op = OP_ADD;
      } else if(data == (process_data_t)&subbutton) {
	input_to_operand1();
	op = OP_SUB;
      } else if(data == (process_data_t)&mulbutton) {
	input_to_operand1();
	op = OP_MUL;
      } else if(data == (process_data_t)&divbutton) {
	input_to_operand1();
	op = OP_DIV;
      }
      CTK_WIDGET_REDRAW(&inputlabel);
    } else if(ev == ctk_signal_window_close &&
	      data == (process_data_t)&window) {
      calc_quit();
    }
  }
  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
