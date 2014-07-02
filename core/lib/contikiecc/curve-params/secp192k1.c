#include "ecc.h"
#include <string.h>

#define TRUE 1
#define FALSE 0

void 
get_curve_param(curve_params_t *para)
{
    
#ifdef EIGHT_BIT_PROCESSOR
    //init parameters
    //prime
    para->p[24] = 0x00;
    para->p[23] = 0xFF;
    para->p[22] = 0xFF;
    para->p[21] = 0xFF;
    para->p[20] = 0xFF;
    para->p[19] = 0xFF;
    para->p[18] = 0xFF;
    para->p[17] = 0xFF;
    para->p[16] = 0xFF;
    para->p[15] = 0xFF;
    para->p[14] = 0xFF;
    para->p[13] = 0xFF;
    para->p[12] = 0xFF;
    para->p[11] = 0xFF;
    para->p[10] = 0xFF;
    para->p[9] = 0xFF;
    para->p[8] = 0xFF;
    para->p[7] = 0xFF;
    para->p[6] = 0xFF;
    para->p[5] = 0xFF;
    para->p[4] = 0xFE;
    para->p[3] = 0xFF;
    para->p[2] = 0xFF;
    para->p[1] = 0xEE;
    para->p[0] = 0x37;
    
    para->omega[0] = 0xC9;
    para->omega[1] = 0x11;
    para->omega[4] = 0x01;
          
    //cure that will be used
    //a
    memset(para->E.a, 0, 25);
    
    para->E.a_minus3 = FALSE;
    para->E.a_zero = TRUE;
    
    //b
    memset(para->E.b, 0, 25);
    para->E.b[0] =  0x03;

    //base point
    para->G.x[24] =  0x0;
    para->G.x[23] =  0xDB;
    para->G.x[22] =  0x4F;
    para->G.x[21] =  0xF1;
    para->G.x[20] =  0x0E;
    para->G.x[19] =  0xC0;
    para->G.x[18] =  0x57;
    para->G.x[17] =  0xE9;
    para->G.x[16] =  0xAE;
    para->G.x[15] =  0x26;
    para->G.x[14] =  0xB0;
    para->G.x[13] =  0x7D;
    para->G.x[12] =  0x02;
    para->G.x[11] =  0x80;
    para->G.x[10] =  0xB7;
    para->G.x[9] =  0xF4;
    para->G.x[8] =  0x34;
    para->G.x[7] =  0x1D;
    para->G.x[6] =  0xA5;
    para->G.x[5] =  0xD1;
    para->G.x[4] =  0xB1;
    para->G.x[3] =  0xEA;
    para->G.x[2] =  0xE0;
    para->G.x[1] =  0x6C;
    para->G.x[0] =  0x7D;
	
    para->G.y[24] =  0x0;
    para->G.y[23] =  0x9B;
    para->G.y[22] =  0x2F;
    para->G.y[21] =  0x2F;
    para->G.y[20] =  0x6D;
    para->G.y[19] =  0x9C;
    para->G.y[18] =  0x56;
    para->G.y[17] =  0x28;
    para->G.y[16] =  0xA7;
    para->G.y[15] =  0x84;
    para->G.y[14] =  0x41;
    para->G.y[13] =  0x63;
    para->G.y[12] =  0xD0;
    para->G.y[11] =  0x15;
    para->G.y[10] =  0xBE;
    para->G.y[9] =  0x86;
    para->G.y[8] =  0x34;
    para->G.y[7] =  0x40;
    para->G.y[6] =  0x82;
    para->G.y[5] =  0xAA;
    para->G.y[4] =  0x88;
    para->G.y[3] =  0xD9;
    para->G.y[2] =  0x5E;
    para->G.y[1] =  0x2F;
    para->G.y[0] =  0x9D;
     	
    //prime divide the number of points
    para->r[24] = 0x00;
    para->r[23] = 0xFF;
    para->r[22] = 0xFF;
    para->r[21] = 0xFF;
    para->r[20] = 0xFF;
    para->r[19] = 0xFF;
    para->r[18] = 0xFF;
    para->r[17] = 0xFF;
    para->r[16] = 0xFF;
    para->r[15] = 0xFF;
    para->r[14] = 0xFF;
    para->r[13] = 0xFF;
    para->r[12] = 0xFE;
    para->r[11] = 0x26;
    para->r[10] = 0xF2;
    para->r[9] = 0xFC;
    para->r[8] = 0x17;
    para->r[7] = 0x0F;
    para->r[6] = 0x69;
    para->r[5] = 0x46;
    para->r[4] = 0x6A;
    para->r[3] = 0x74;
    para->r[2] = 0xDE;
    para->r[1] = 0xFD;
    para->r[0] = 0x8D;	
     
    /* EIGHT_BIT_PROCESSOR */
DOMAIN_PARAM SIXTEEN_BIT_PROCESSOR

    //init parameters
    //prime
    memset(para->p, 0, NUMWORDS*NN_DIGIT_LEN);
    para->p[11] = 0xFFFF;
    para->p[10] = 0xFFFF;
    para->p[9] = 0xFFFF;
    para->p[8] = 0xFFFF;
    para->p[7] = 0xFFFF;
    para->p[6] = 0xFFFF;
    para->p[5] = 0xFFFF;
    para->p[4] = 0xFFFF;
    para->p[3] = 0xFFFF;
    para->p[2] = 0xFFFE;
    para->p[1] = 0xFFFF;
    para->p[0] = 0xEE37;
         
    memset(para->omega, 0, NUMWORDS*NN_DIGIT_LEN);
    para->omega[0] = 0x11C9;
    para->omega[2] = 0x0001;     
    //cure that will be used
    //a
    memset(para->E.a, 0, NUMWORDS*NN_DIGIT_LEN);         
    para->E.a_minus3 = FALSE;
    para->E.a_zero = TRUE;
    
    //b
    memset(para->E.b, 0, NUMWORDS*NN_DIGIT_LEN);
    para->E.b[0] =  0x0003;
     
    //base point
    memset(para->G.x, 0, NUMWORDS*NN_DIGIT_LEN);
    para->G.x[11] =  0xDB4F;
    para->G.x[10] =  0xF10E;
    para->G.x[9] =  0xC057;
    para->G.x[8] =  0xE9AE;
    para->G.x[7] =  0x26B0;
    para->G.x[6] =  0x7D02;
    para->G.x[5] =  0x80B7;
    para->G.x[4] =  0xF434;
    para->G.x[3] =  0x1DA5;
    para->G.x[2] =  0xD1B1;
    para->G.x[1] =  0xEAE0;
    para->G.x[0] =  0x6C7D;
     
    memset(para->G.y, 0, NUMWORDS*NN_DIGIT_LEN);
    para->G.y[11] =  0x9B2F;
    para->G.y[10] =  0x2F6D;
    para->G.y[9] =  0x9C56;
    para->G.y[8] =  0x28A7;
    para->G.y[7] =  0x8441;
    para->G.y[6] =  0x63D0;
    para->G.y[5] =  0x15BE;
    para->G.y[4] =  0x8634;
    para->G.y[3] =  0x4082;
    para->G.y[2] =  0xAA88;
    para->G.y[1] =  0xD95E;
    para->G.y[0] =  0x2F9D;
         	
    //prime divide the number of points
    memset(para->r, 0, NUMWORDS*NN_DIGIT_LEN);
    para->r[11] = 0xFFFF;
    para->r[10] = 0xFFFF;
    para->r[9] = 0xFFFF;
    para->r[8] = 0xFFFF;
    para->r[7] = 0xFFFF;
    para->r[6] = 0xFFFE;
    para->r[5] = 0x26F2;
    para->r[4] = 0xFC17;
    para->r[3] = 0x0F69;
    para->r[2] = 0x466A;
    para->r[1] = 0x74DE;
    para->r[0] = 0xFD8D;	
          
   /* SIXTEEN_BIT_PROCESSOR */
DOMAIN_PARAM THIRTYTWO_BIT_PROCESSOR

     //init parameters
    //prime
    memset(para->p, 0, NUMWORDS*NN_DIGIT_LEN);
    para->p[5] = 0xFFFFFFFF;
    para->p[4] = 0xFFFFFFFF;
    para->p[3] = 0xFFFFFFFF;
    para->p[2] = 0xFFFFFFFF;
    para->p[1] = 0xFFFFFFFE;
    para->p[0] = 0xFFFFEE37;
         
    memset(para->omega, 0, NUMWORDS*NN_DIGIT_LEN);
    para->omega[0] = 0x000011C9;
    para->omega[1] = 0x00000001;     
    //cure that will be used
    //a
    memset(para->E.a, 0, NUMWORDS*NN_DIGIT_LEN);         
    para->E.a_minus3 = FALSE;
    para->E.a_zero = TRUE;
    
    //b
    memset(para->E.b, 0, NUMWORDS*NN_DIGIT_LEN);
    para->E.b[0] =  0x00000003;
     
    //base point
    memset(para->G.x, 0, NUMWORDS*NN_DIGIT_LEN);
    para->G.x[5] =  0xDB4FF10E;
    para->G.x[4] =  0xC057E9AE;
    para->G.x[3] =  0x26B07D02;
    para->G.x[2] =  0x80B7F434;
    para->G.x[1] =  0x1DA5D1B1;
    para->G.x[0] =  0xEAE06C7D;
     
    memset(para->G.y, 0, NUMWORDS*NN_DIGIT_LEN);
    para->G.y[5] = 0x9B2F2F6D;
    para->G.y[4] =  0x9C5628A7;
    para->G.y[3] =  0x844163D0;
    para->G.y[2] =  0x15BE8634;
    para->G.y[1] =  0x4082AA88;
    para->G.y[0] =  0xD95E2F9D;
         	
    //prime divide the number of points
    memset(para->r, 0, NUMWORDS*NN_DIGIT_LEN);
    para->r[5] =0xFFFFFFFF;
    para->r[4] = 0xFFFFFFFF;
    para->r[3] = 0xFFFFFFFE;
    para->r[2] = 0x26F2FC17;
    para->r[1] = 0x0F69466A;
    para->r[0] = 0x74DEFD8D;

#endif /* THIRTYTWO_BIT_PROCESSOR */

  }
  
NN_UINT 
omega_mul(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *omega, NN_UINT digits)
{
  
#ifdef EIGHT_BIT_PROCESSOR

#ifdef CPU_AVR
     uint8_t n_d;
    if (digits % 4 == 0)
      n_d = digits/4;
    else
      n_d = digits/4 + 1;
    
    //r2~r10
    //r11~r14
    //r15 c[0]
    //r16 i
    //r17 c[1]
    //r19 0
    //r21:r20 b
    //r23:r22 c
    //r25 d
    asm volatile (//"push r0 \n\t"
		  "push r1 \n\t"
		  "push r28 \n\t"
		  "push r29 \n\t"
		  "clr r2 \n\t"  //init 9 registers for accumulator
		  "clr r3 \n\t"
		  "clr r4 \n\t"
		  "clr r5 \n\t"
		  "clr r6 \n\t"
		  "clr r7 \n\t"
		  "clr r8 \n\t"
		  "clr r9 \n\t"
		  "clr r10 \n\t"  //end of init
		  "clr r19 \n\t"  //zero
		  "ldi r25, 4 \n\t"  //d=4
		  "dec %3 \n\t"
		  "ldi r16, 0 \n\t"  //i
		  "movw r28, %A2 \n\t"  //load c
		  "ld r15, Y+ \n\t"  //c[0]
		  "ld r17, Y \n\t"  //c[3]
		  "OMEGA_LOOP1: mul r16, r25 \n\t"  //i*d
		  "add r0, r25 \n\t"
		  "movw r26, %A1 \n\t"
		  "add r26, r0 \n\t"
		  "adc r27, r1 \n\t"  //load b, (i-j+1)*d-1
		  "movw r28, %A2 \n\t"  //load c
		  "ld r14, -X \n\t"  //load b0~b(d-1)
		  "ld r13, -X \n\t"
		  "ld r12, -X \n\t"
		  "ld r11, -X \n\t"
		  "mul r11, r15 \n\t"  //t=0
		  "add r2, r0 \n\t"
		  "adc r3, r1 \n\t"
		  "brcc OMEGA_T01 \n\t"
		  "adc r4, r19 \n\t"
		  "brcc OMEGA_T01 \n\t"
		  "adc r5, r19 \n\t"
		  "adc r6, r19 \n\t"
		  "adc r7, r19 \n\t"
		  "adc r8, r19 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T01: mul r12, r15 \n\t"  //t=1
		  "add r3, r0 \n\t"
		  "adc r4, r1 \n\t"
		  "brcc OMEGA_T02 \n\t"
		  "adc r5, r19 \n\t"
		  "brcc OMEGA_T02 \n\t"
		  "adc r6, r19 \n\t"
		  "adc r7, r19 \n\t"
		  "adc r8, r19 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T02: mul r13, r15 \n\t"  //t=2
		  "add r4, r0 \n\t"
		  "adc r5, r1 \n\t"
		  "brcc OMEGA_T03 \n\t"
		  "adc r6, r19 \n\t"
		  "brcc OMEGA_T03 \n\t"
		  "adc r7, r19 \n\t"
		  "adc r8, r19 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T03: mul r14, r15 \n\t"  //t=3
		  "add r5, r0 \n\t"
		  "adc r6, r1 \n\t"
		  "brcc OMEGA_T24 \n\t"
		  "adc r7, r19 \n\t"
		  "brcc OMEGA_T24 \n\t"
		  "adc r8, r19 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T24: mul r11, r17 \n\t" //t=0, b0*c
		  "add r3, r0 \n\t"
		  "adc r4, r1 \n\t"
		  "brcc OMEGA_T31 \n\t"
		  "adc r5, r19 \n\t"
		  "brcc OMEGA_T31 \n\t"
		  "adc r6, r19 \n\t"
		  "adc r7, r19 \n\t"
		  "adc r8, r19 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T31: mul r12, r17 \n\t"  //t=1
		  "add r4, r0 \n\t"
		  "adc r5, r1 \n\t"
		  "brcc OMEGA_T32 \n\t"
		  "adc r6, r19 \n\t"
		  "brcc OMEGA_T32 \n\t"
		  "adc r7, r19 \n\t"
		  "adc r8, r19 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T32: mul r13, r17 \n\t"  //t=2
		  "add r5, r0 \n\t"
		  "adc r6, r1 \n\t"
		  "brcc OMEGA_T33 \n\t"
		  "adc r7, r19 \n\t"
		  "brcc OMEGA_T33 \n\t"
		  "adc r8, r19 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T33: mul r14, r17 \n\t"  //t=3
		  "add r6, r0 \n\t"
		  "adc r7, r1 \n\t"
		  "brcc OMEGA_T34 \n\t"
		  "adc r8, r19 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T34: st Z+, r2 \n\t"  //a[i*d] = r2
		  "st Z+, r3 \n\t"
		  "st Z+, r4 \n\t"
		  "st Z+, r5 \n\t"
		  "movw r2, r6 \n\t"  //can be speed up use movw
		  "movw r4, r8 \n\t"
		  "mov r6, r10 \n\t"  //can be remove
		  "clr r7 \n\t"
		  "clr r8 \n\t"
		  "clr r9 \n\t"
		  "clr r10 \n\t"
		  "cp r16, %3 \n\t"  //i == 4?
		  "breq OMEGA_LOOP1_EXIT \n\t"
		  "inc r16 \n\t"
		  "jmp OMEGA_LOOP1 \n\t"
		  "OMEGA_LOOP1_EXIT: st Z+, r2 \n\t"
		  "st Z+, r3 \n\t"
		  "st Z+, r4 \n\t"
		  "st Z+, r5 \n\t"
		  "pop r29 \n\t"
		  "pop r28 \n\t"
		  "pop r1 \n\t"
		  //"pop r0 \n\t"
		  :
		  :"z"(a),"a"(b),"a"(omega),"r"(n_d)
		  :"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12","r13","r14","r15","r16","r17","r19","r25","r26","r27","r28","r29"
		  );

    NN_Add(a+4, a+4, b, digits+1);
    return (digits+5);
    /* CPU_AVR */
#else
    //memset(a, 0, digits*NN_DIGIT_LEN);
    a[digits] += NN_AddDigitMult(a, a, omega[0], b, digits);
    a[digits+1] += NN_AddDigitMult(&a[1], &a[1], omega[1], b, digits);
    NN_Add(&a[4], &a[4], b, digits+1);
    return (digits+5);
    
#endif
    /* EIGHT_BIT_PROCESSOR */  
DOMAIN_PARAM SIXTEEN_BIT_PROCESSOR

    //memset(a, 0, digits*NN_DIGIT_LEN);
    a[digits] += NN_AddDigitMult(a, a, omega[0], b, digits);
    NN_Add(&a[2], &a[2], b, digits+1);
    return (digits+3);

    /* SIXTEEN_BIT_PROCESSOR */  
DOMAIN_PARAM THIRTYTWO_BIT_PROCESSOR

    a[digits] += call NN_AddDigitMult(a, a, omega[0], b, digits);
    NN_Add(&a[1], &a[1], b, digits+1);
    return (digits+2);
  
#endif /* THIRTYTWO_BIT_PROCESSOR */
  
}
