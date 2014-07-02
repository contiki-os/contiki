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
    para->p[20] = 0x00;
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
    para->p[1] = 0xAC;
    para->p[0] = 0x73;

    memset(para->omega, 0, NUMWORDS);
    para->omega[0] = 0x8D;
    para->omega[1] = 0x53;
    para->omega[4] = 0x01;
	
    //cure that will be used
    //a
    para->E.a[20] = 0x00;
    para->E.a[19] = 0xFF;
    para->E.a[18] = 0xFF;
    para->E.a[17] = 0xFF;
    para->E.a[16] = 0xFF;
    para->E.a[15] = 0xFF;
    para->E.a[14] = 0xFF;
    para->E.a[13] = 0xFF;
    para->E.a[12] = 0xFF;
    para->E.a[11] = 0xFF;
    para->E.a[10] = 0xFF;
    para->E.a[9] = 0xFF;
    para->E.a[8] = 0xFF;
    para->E.a[7] = 0xFF;
    para->E.a[6] = 0xFF;
    para->E.a[5] = 0xFF;
    para->E.a[4] = 0xFE;
    para->E.a[3] = 0xFF;
    para->E.a[2] = 0xFF;
    para->E.a[1] = 0xAC;
    para->E.a[0] = 0x70;

    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[20] = 0x00;
    para->E.b[19] = 0xB4;
    para->E.b[18] = 0xE1;
    para->E.b[17] = 0x34;
    para->E.b[16] = 0xD3;
    para->E.b[15] = 0xFB;
    para->E.b[14] = 0x59;
    para->E.b[13] = 0xEB;
    para->E.b[12] = 0x8B;
    para->E.b[11] = 0xAB;
    para->E.b[10] = 0x57;
    para->E.b[9] = 0x27;
    para->E.b[8] = 0x49;
    para->E.b[7] = 0x04;
    para->E.b[6] = 0x66;
    para->E.b[5] = 0x4D;
    para->E.b[4] = 0x5A;
    para->E.b[3] = 0xF5;
    para->E.b[2] = 0x03;
    para->E.b[1] = 0x88;
    para->E.b[0] = 0xBA;
        
    //base point
    para->G.x[20] = 0x00;
    para->G.x[19] =  0x52;
    para->G.x[18] =  0xDC;
    para->G.x[17] =  0xB0;
    para->G.x[16] =  0x34;
    para->G.x[15] =  0x29;
    para->G.x[14] =  0x3A;
    para->G.x[13] =  0x11;
    para->G.x[12] =  0x7E;
    para->G.x[11] =  0x1F;
    para->G.x[10] =  0x4F;
    para->G.x[9] =  0xF1;
    para->G.x[8] =  0x1B;
    para->G.x[7] =  0x30;
    para->G.x[6] =  0xF7;
    para->G.x[5] =  0x19;
    para->G.x[4] =  0x9D;
    para->G.x[3] =  0x31;
    para->G.x[2] =  0x44;
    para->G.x[1] =  0xCE;
    para->G.x[0] =  0x6D;
        
    para->G.y[20] = 0x00;
    para->G.y[19] =  0xFE;
    para->G.y[18] =  0xAF;
    para->G.y[17] =  0xFE;
    para->G.y[16] =  0xF2;
    para->G.y[15] =  0xE3;
    para->G.y[14] =  0x31;
    para->G.y[13] =  0xF2;
    para->G.y[12] =  0x96;
    para->G.y[11] =  0xE0;
    para->G.y[10] =  0x71;
    para->G.y[9] =  0xFA;
    para->G.y[8] =  0x0D;
    para->G.y[7] =  0xF9;
    para->G.y[6] =  0x98;
    para->G.y[5] =  0x2C;
    para->G.y[4] =  0xFE;
    para->G.y[3] =  0xA7;
    para->G.y[2] =  0xD4;
    para->G.y[1] =  0x3F;
    para->G.y[0] =  0x2E;

       	
    //prime divide the number of points
    para->r[20] = 0x01;
    para->r[19] = 0x0;
    para->r[18] = 0x0;
    para->r[17] = 0x0;
    para->r[16] = 0x0;
    para->r[15] = 0x0;
    para->r[14] = 0x0;
    para->r[13] = 0x0;
    para->r[12] = 0x0;
    para->r[11] = 0x0;
    para->r[10] = 0x0;
    para->r[9] = 0x35;
    para->r[8] = 0x1E;
    para->r[7] = 0xE7;
    para->r[6] = 0x86;
    para->r[5] = 0xA8;
    para->r[4] = 0x18;
    para->r[3] = 0xF3;
    para->r[2] = 0xA1;
    para->r[1] = 0xA1;
    para->r[0] = 0x6B;
     
    /* EIGHT_BIT_PROCESSOR */
DOMAIN_PARAM SIXTEEN_BIT_PROCESSOR

    //init parameters
    //prime
    para->p[9] = 0xFFFF;
    para->p[8] = 0xFFFF;
    para->p[7] = 0xFFFF;
    para->p[6] = 0xFFFF;
    para->p[5] = 0xFFFF;
    para->p[4] = 0xFFFF;
    para->p[3] = 0xFFFF;
    para->p[2] = 0xFFFE;
    para->p[1] = 0xFFFF;
    para->p[0] = 0xAC73;
      
    para->omega[0] = 0x538D;
    para->omega[2] = 0x0001;
	
    //cure that will be used
    //a
    para->E.a[9] = 0xFFFF;
    para->E.a[8] = 0xFFFF;
    para->E.a[7] = 0xFFFF;
    para->E.a[6] = 0xFFFF;
    para->E.a[5] = 0xFFFF;
    para->E.a[4] = 0xFFFF;
    para->E.a[3] = 0xFFFF;
    para->E.a[2] = 0xFFFE;
    para->E.a[1] = 0xFFFF;
    para->E.a[0] = 0xAC70;
    
    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[9] = 0xB4E1;
    para->E.b[8] = 0x34D3;
    para->E.b[7] = 0xFB59;
    para->E.b[6] = 0xEB8B;
    para->E.b[5] = 0xAB57;
    para->E.b[4] = 0x2749;
    para->E.b[3] = 0x0466;
    para->E.b[2] = 0x4D5A;
    para->E.b[1] = 0xF503;
    para->E.b[0] = 0x88BA;
            
    //base point
    para->G.x[9] =  0x52DC;
    para->G.x[8] =  0xB034;
    para->G.x[7] =  0x293A;
    para->G.x[6] =  0x117E;
    para->G.x[5] =  0x1F4F;
    para->G.x[4] =  0xF11B;
    para->G.x[3] =  0x30F7;
    para->G.x[2] =  0x199D;
    para->G.x[1] =  0x3144;
    para->G.x[0] =  0xCE6D;

    para->G.y[9] =  0xFEAF;
    para->G.y[8] =  0xFEF2;
    para->G.y[7] =  0xE331;
    para->G.y[6] =  0xF296;
    para->G.y[5] =  0xE071;
    para->G.y[4] =  0xFA0D;
    para->G.y[3] =  0xF998;
    para->G.y[2] =  0x2CFE;
    para->G.y[1] =  0xA7D4;
    para->G.y[0] =  0x3F2E;
     	
    //prime divide the number of points
    para->r[10] = 0x0001;
    para->r[9] = 0x0000;
    para->r[8] = 0x0000;
    para->r[7] = 0x0000;
    para->r[6] = 0x0000;
    para->r[5] = 0x0000;
    para->r[4] = 0x351E;
    para->r[3] = 0xE786;
    para->r[2] = 0xA818;
    para->r[1] = 0xF3A1;
    para->r[0] = 0xA16B;
          
   /* SIXTEEN_BIT_PROCESSOR */
DOMAIN_PARAM THIRTYTWO_BIT_PROCESSOR

    //init parameters
    //prime
    para->p[4] = 0xFFFFFFFF;
    para->p[3] = 0xFFFFFFFF;
    para->p[2] = 0xFFFFFFFF;
    para->p[1] = 0xFFFFFFFE;
    para->p[0] = 0xFFFFAC73;
      
    para->omega[0] = 0x0000538D;
    para->omega[1] = 0x00000001;
	
    //cure that will be used
    //a
    para->E.a[4] = 0xFFFFFFFF;
    para->E.a[3] = 0xFFFFFFFF;
    para->E.a[2] = 0xFFFFFFFF;
    para->E.a[1] = 0xFFFFFFFE;
    para->E.a[0] = 0xFFFFAC70;
    
    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[4] = 0xB4E134D3;
    para->E.b[3] = 0xFB59EB8B;
    para->E.b[2] = 0xAB572749;
    para->E.b[1] = 0x04664D5A;
    para->E.b[0] = 0xF50388BA;
            
    //base point
    para->G.x[4] =  0x52DCB034;
    para->G.x[3] =  0x293A117E;
    para->G.x[2] =  0x1F4FF11B;
    para->G.x[1] =  0x30F7199D;
    para->G.x[0] =  0x3144CE6D;

    para->G.y[4] =  0xFEAFFEF2;
    para->G.y[3] =  0xE331F296;
    para->G.y[2] =  0xE071FA0D;
    para->G.y[1] =  0xF9982CFE;
    para->G.y[0] =  0xA7D43F2E;
     	
    //prime divide the number of points
    para->r[5] = 0x00000001;
    para->r[4] = 0x00000000;
    para->r[3] = 0x00000000;
    para->r[2] = 0x0000351E;
    para->r[1] = 0xE786A818;
    para->r[0] = 0xF3A1A16B;

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

    a[digits] += NN_AddDigitMult(a, a, omega[0], b, digits);
    NN_Add(&a[1], &a[1], b, digits+1);
    return (digits+2);
  
#endif /* THIRTYTWO_BIT_PROCESSOR */
  
}
