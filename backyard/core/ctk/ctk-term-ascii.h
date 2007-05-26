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
 * @(#)$Id: ctk-term-ascii.h,v 1.1 2007/05/26 21:54:33 oliverschmidt Exp $
 */
/*   ascii_control.h
 *
 *   ASCII CONTROL CHARACTERS
 *   from American National Standard Code for Information Interchange X3.4-1977
 *
 *   Abbreviations
 *   
 *   CC:    communication control
 *   FE:    format effector
 *   IS:    information separator
 *   Delim: delimiter
 *   Intro: introducer
 */

/*   ----------------------------------------------------------------------- */
/*   C0 (7-bit) set                                                          */
/*   ----------------------------------------------------------------------- */
/*      mnemonic   octal      decimal   C     meaning                        */
/*   ----------------------------------------------------------------------- */

#define ASCII_NUL  (000)    /*      0 '\0'    Null                           */
#define ASCII_SOH  (001)    /*      1         Start of Heading (CC)          */
#define ASCII_STX  (002)    /*      2         Start of Text (CC)             */
#define ASCII_ETX  (003)    /*      3         End of Text (CC)               */
#define ASCII_EOT  (004)    /*      4         End of Transmission (CC)       */
#define ASCII_ENQ  (005)    /*      5         Enquiry (CC)                   */
#define ASCII_ACK  (006)    /*      6         Acknowledge (CC)               */
#define ASCII_BEL  (007)    /*      7 '\a'    Bell / Alert                   */
#define ASCII_BS   (010)    /*      8 '\b'    Backspace (FE)                 */
#define ASCII_HT   (011)    /*      9 '\t'    Horizontal Tabulation (FE)     */
#define ASCII_LF   (012)    /*     10 '\n'    Line Feed / Newline (FE)       */
#define ASCII_VT   (013)    /*     11 '\v'    Vertical Tabulation (FE)       */
#define ASCII_FF   (014)    /*     12 '\f'    Form Feed (FE)                 */
#define ASCII_CR   (015)    /*     13 '\r'    Carriage Return (FE)           */
#define ASCII_SO   (016)    /*     14         Shift Out                      */
#define ASCII_SI   (017)    /*     15         Shift In                       */
#define ASCII_DLE  (020)    /*     16         Data Link Escape (CC)          */
#define ASCII_DC1  (021)    /*     17         Device Control 1  XON          */
#define ASCII_DC2  (022)    /*     18         Device Control 2               */
#define ASCII_DC3  (023)    /*     19         Device Control 3  XOFF         */
#define ASCII_DC4  (024)    /*     20         Device Control 4               */
#define ASCII_NAK  (025)    /*     21         Negative Acknowledge (CC)      */
#define ASCII_SYN  (026)    /*     22         Synchronous Idle (CC)          */
#define ASCII_ETB  (027)    /*     23         End of Transmission Block (CC) */
#define ASCII_CAN  (030)    /*     24         Cancel                         */
#define ASCII_EM   (031)    /*     25         End of Medium                  */
#define ASCII_SUB  (032)    /*     26         Substitute                     */
#define ASCII_ESC  (033)    /*     27         Escape                         */
#define ASCII_FS   (034)    /*     28         File Separator                 */
#define ASCII_GS   (035)    /*     29         Group Separator                */
#define ASCII_RS   (036)    /*     30         Record Separator               */
#define ASCII_US   (037)    /*     31         Unit Separator                 */

#define ASCII_DEL (0177)    /*    127         Delete                         */

#define ASCII_NL  ASCII_LF  /*     10         alias for Newline              */

/*   ----------------------------------------------------------------------- */
/*   C1 (8-bit) set (which have equivalent 7-bit multi-char sequences)       */
/*   ----------------------------------------------------------------------- */
/*      mnemonic   octal      decimal  7-bit  meaning                        */
/*   ----------------------------------------------------------------------- */

#define ASCII_IND (0204)    /*    132  Esc D  Index (FE)                     */
#define ASCII_NEL (0205)    /*    133  Esc E  Next Line (FE)                 */
#define ASCII_SSA (0206)    /*    134  Esc F  Start Selected Area            */
#define ASCII_ESA (0207)    /*    135  Esc G  End Selected Area              */
#define ASCII_HTS (0210)    /*    136  Esc H  Horizontal Tab Set (FE)        */
#define ASCII_HTJ (0211)    /*    137  Esc I  Horizontal Tab w/Justificat(FE)*/
#define ASCII_VTS (0212)    /*    138  Esc J  Vertical Tab Set (FE)          */
#define ASCII_PLD (0213)    /*    138  Esc K  Partial Line Down (FE)         */
#define ASCII_PLU (0214)    /*    140  Esc L  Partial Line Up (FE)           */
#define ASCII_RI  (0215)    /*    141  Esc M  Reverse Index (FE)             */
#define ASCII_SS2 (0216)    /*    142  Esc N  Single Shift G2 (Intro)        */
#define ASCII_SS3 (0217)    /*    143  Esc O  Single Shift G3 (Intro)        */
#define ASCII_DCS (0220)    /*    144  Esc P  Device Control String (Delim)  */
#define ASCII_PU1 (0221)    /*    145  Esc Q  Private Use 1                  */
#define ASCII_PU2 (0222)    /*    146  Esc R  Private Use 2                  */
#define ASCII_STS (0223)    /*    147  Esc S  Set Transmit State             */
#define ASCII_CCH (0224)    /*    148  Esc T  Cancel Previous Character      */
#define ASCII_MW  (0225)    /*    149  Esc U  Message Waiting                */
#define ASCII_SPA (0226)    /*    150  Esc V  Start Protected Area           */
#define ASCII_EPA (0227)    /*    151  Esc W  End Protected Area             */

#define ASCII_CSI (0233)    /*    155  Esc [  Control Sequence Introducer    */
#define ASCII_ST  (0234)    /*    156  Esc \  String Terminator (Delim)      */
#define ASCII_OSC (0235)    /*    157  Esc ]  Operating System Control (Delim*/
#define ASCII_PM  (0236)    /*    158  Esc ^  Privacy Message (Delim)        */
#define ASCII_APC (0237)    /*    159  Esc _  Application Program Command (De*/

/* ------------------------------------------------------------------------- */
