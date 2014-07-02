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
    para->p[4] = 0xFF;
    para->p[3] = 0x7F;
    para->p[2] = 0xFF;
    para->p[1] = 0xFF;
    para->p[0] = 0xFF;
    
    memset(para->omega, 0, NUMWORDS);
    para->omega[0] = 0x01;
    para->omega[3] = 0x80;
    
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
    para->E.a[4] = 0xFF;
    para->E.a[3] = 0x7F;
    para->E.a[2] = 0xFF;
    para->E.a[1] = 0xFF;
    para->E.a[0] = 0xFC;
	
    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[20] = 0x00;
    para->E.b[19] = 0x1C;
    para->E.b[18] = 0x97;
    para->E.b[17] = 0xBE;
    para->E.b[16] = 0xFC;
    para->E.b[15] = 0x54;
    para->E.b[14] = 0xBD;
    para->E.b[13] = 0x7A;
    para->E.b[12] = 0x8B;
    para->E.b[11] = 0x65;
    para->E.b[10] = 0xAC;
    para->E.b[9] = 0xF8;
    para->E.b[8] = 0x9F;
    para->E.b[7] = 0x81;
    para->E.b[6] = 0xD4;
    para->E.b[5] = 0xD4;
    para->E.b[4] = 0xAD;
    para->E.b[3] = 0xC5;
    para->E.b[2] = 0x65;
    para->E.b[1] = 0xFA;
    para->E.b[0] = 0x45;
    
    //base point
    para->G.x[20] = 0x00;
    para->G.x[19] =  0x4A;
    para->G.x[18] =  0x96;
    para->G.x[17] =  0xB5;
    para->G.x[16] =  0x68;
    para->G.x[15] =  0x8E;
    para->G.x[14] =  0xF5;
    para->G.x[13] =  0x73;
    para->G.x[12] =  0x28;
    para->G.x[11] =  0x46;
    para->G.x[10] =  0x64;
    para->G.x[9] =  0x69;
    para->G.x[8] =  0x89;
    para->G.x[7] =  0x68;
    para->G.x[6] =  0xC3;
    para->G.x[5] =  0x8B;
    para->G.x[4] =  0xB9;
    para->G.x[3] =  0x13;
    para->G.x[2] =  0xCB;
    para->G.x[1] =  0xFC;
    para->G.x[0] =  0x82;
    
    para->G.y[20] = 0x00;
    para->G.y[19] =  0x23;
    para->G.y[18] =  0xA6;
    para->G.y[17] =  0x28;
    para->G.y[16] =  0x55;
    para->G.y[15] =  0x31;
    para->G.y[14] =  0x68;
    para->G.y[13] =  0x94;
    para->G.y[12] =  0x7D;
    para->G.y[11] =  0x59;
    para->G.y[10] =  0xDC;
    para->G.y[9] =  0xC9;
    para->G.y[8] =  0x12;
    para->G.y[7] =  0x04;
    para->G.y[6] =  0x23;
    para->G.y[5] =  0x51;
    para->G.y[4] =  0x37;
    para->G.y[3] =  0x7A;
    para->G.y[2] =  0xC5;
    para->G.y[1] =  0xFB;
    para->G.y[0] =  0x32;
    	
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
    para->r[10] = 0x01;
    para->r[9] = 0xF4;
    para->r[8] = 0xC8;
    para->r[7] = 0xF9;
    para->r[6] = 0x27;
    para->r[5] = 0xAE;
    para->r[4] = 0xD3;
    para->r[3] = 0xCA;
    para->r[2] = 0x75;
    para->r[1] = 0x22;
    para->r[0] = 0x57;
     
    /* EIGHT_BIT_PROCESSOR */
#else
  #ifdef SIXTEEN_BIT_PROCESSOR

    //init parameters
    //prime
    para->p[9] = 0xFFFF;
    para->p[8] = 0xFFFF;
    para->p[7] = 0xFFFF;
    para->p[6] = 0xFFFF;
    para->p[5] = 0xFFFF;
    para->p[4] = 0xFFFF;
    para->p[3] = 0xFFFF;
    para->p[2] = 0xFFFF;
    para->p[1] = 0x7FFF;
    para->p[0] = 0xFFFF;
        
    para->omega[0] = 0x0001;
    para->omega[1] = 0x8000;
    
    //cure that will be used
    //a
    para->E.a[9] = 0xFFFF;
    para->E.a[8] = 0xFFFF;
    para->E.a[7] = 0xFFFF;
    para->E.a[6] = 0xFFFF;
    para->E.a[5] = 0xFFFF;
    para->E.a[4] = 0xFFFF;
    para->E.a[3] = 0xFFFF;
    para->E.a[2] = 0xFFFF;
    para->E.a[1] = 0x7FFF;
    para->E.a[0] = 0xFFFC;
    	
    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[9] = 0x1C97;
    para->E.b[8] = 0xBEFC;
    para->E.b[7] = 0x54BD;
    para->E.b[6] = 0x7A8B;
    para->E.b[5] = 0x65AC;
    para->E.b[4] = 0xF89F;
    para->E.b[3] = 0x81D4;
    para->E.b[2] = 0xD4AD;
    para->E.b[1] = 0xC565;
    para->E.b[0] = 0xFA45;
        
    //base point
    para->G.x[9] =  0x4A96;
    para->G.x[8] =  0xB568;
    para->G.x[7] =  0x8EF5;
    para->G.x[6] =  0x7328;
    para->G.x[5] =  0x4664;
    para->G.x[4] =  0x6989;
    para->G.x[3] =  0x68C3;
    para->G.x[2] =  0x8BB9;
    para->G.x[1] =  0x13CB;
    para->G.x[0] =  0xFC82;

    para->G.y[9] =  0x23A6;
    para->G.y[8] =  0x2855;
    para->G.y[7] =  0x3168;
    para->G.y[6] =  0x947D;
    para->G.y[5] =  0x59DC;
    para->G.y[4] =  0xC912;
    para->G.y[3] =  0x0423;
    para->G.y[2] =  0x5137;
    para->G.y[1] =  0x7AC5;
    para->G.y[0] =  0xFB32;
        	
    //prime divide the number of points
    para->r[10] = 0x0001;
    para->r[9] = 0x0000;
    para->r[8] = 0x0000;
    para->r[7] = 0x0000;
    para->r[6] = 0x0000;
    para->r[5] = 0x0001;
    para->r[4] = 0xF4C8;
    para->r[3] = 0xF927;
    para->r[2] = 0xAED3;
    para->r[1] = 0xCA75;
    para->r[0] = 0x2257;
          
   /* SIXTEEN_BIT_PROCESSOR */
  #else
    #ifdef THIRTYTWO_BIT_PROCESSOR

    //init parameters
    //prime
    para->p[5] = 0x00000000;
    para->p[4] = 0xFFFFFFFF;
    para->p[3] = 0xFFFFFFFF;
    para->p[2] = 0xFFFFFFFF;
    para->p[1] = 0xFFFFFFFF;
    para->p[0] = 0x7FFFFFFF;
    memset(para->omega, 0, NUMWORDS);        
    para->omega[0] = 0x80000001;
    
    //cure that will be used
    //a
    para->E.a[5] = 0x00000000;
    para->E.a[4] = 0xFFFFFFFF;
    para->E.a[3] = 0xFFFFFFFF;
    para->E.a[2] = 0xFFFFFFFF;
    para->E.a[1] = 0xFFFFFFFF;
    para->E.a[0] = 0x7FFFFFFC;
    	
    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[5] = 0x00000000;
    para->E.b[4] = 0x1C97BEFC;
    para->E.b[3] = 0x54BD7A8B;
    para->E.b[2] = 0x65ACF89F;
    para->E.b[1] = 0x81D4D4AD;
    para->E.b[0] = 0xC565FA45;
        
    //base point
    para->G.x[5] = 0x00000000;
    para->G.x[4] = 0x4A96B568;
    para->G.x[3] = 0x8EF57328;
    para->G.x[2] = 0x46646989;
    para->G.x[1] = 0x68C38BB9;
    para->G.x[0] = 0x13CBFC82;

    para->G.y[5] = 0x00000000;
    para->G.y[4] = 0x23A62855;
    para->G.y[3] = 0x3168947D;
    para->G.y[2] = 0x59DCC912;
    para->G.y[1] = 0x04235137;
    para->G.y[0] = 0x7AC5FB32;
        	
    //prime divide the number of points
    para->r[5] = 0x00000001;
    para->r[4] = 0x00000000;
    para->r[3] = 0x00000000;
    para->r[2] = 0x0001F4C8;
    para->r[1] = 0xF927AED3;
    para->r[0] = 0xCA752257;
    #endif /* THIRTYTWO_BIT_PROCESSOR */
  #endif
#endif
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
    //r17 c[3]
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
		  "ld r15, Y \n\t"  //c[0]
		  "adiw r28, 3 \n\t"
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
		  "add r5, r0 \n\t"
		  "adc r6, r1 \n\t"
		  "brcc OMEGA_T31 \n\t"
		  "adc r7, r19 \n\t"
		  "brcc OMEGA_T31 \n\t"
		  "adc r8, r19 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T31: mul r12, r17 \n\t"  //t=1
		  "add r6, r0 \n\t"
		  "adc r7, r1 \n\t"
		  "brcc OMEGA_T32 \n\t"
		  "adc r8, r19 \n\t"
		  "brcc OMEGA_T32 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T32: mul r13, r17 \n\t"  //t=2
		  "add r7, r0 \n\t"
		  "adc r8, r1 \n\t"
		  "brcc OMEGA_T33 \n\t"
		  "adc r9, r19 \n\t"
		  "adc r10, r19 \n\t"
		  "OMEGA_T33: mul r14, r17 \n\t"  //t=3
		  "add r8, r0 \n\t"
		  "adc r9, r1 \n\t"
		  "adc r10, r19 \n\t"
		  "st Z+, r2 \n\t"  //a[i*d] = r2
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

    return (digits+4);
    /* CPU_AVR */
#else
    //memset(a, 0, digits*NN_DIGIT_LEN);
    NN_Assign(a, b, digits);
    a[digits+3] += NN_AddDigitMult(&a[3], &a[3], omega[3], b, digits);
    return (digits+4);    
    
#endif
    /* EIGHT_BIT_PROCESSOR */  
#else
  #ifdef SIXTEEN_BIT_PROCESSOR

    //memset(a, 0, digits*NN_DIGIT_LEN);
    NN_Assign(a, b, digits);
    a[digits+1] += NN_AddDigitMult(&a[1], &a[1], omega[1], b, digits);
    return (digits+2);

    /* SIXTEEN_BIT_PROCESSOR */  
  #else
    #ifdef THIRTYTWO_BIT_PROCESSOR
  // bug fix:
  //NN_Assign(a, b, digits);
  a[digits] += NN_AddDigitMult(&a[0], &a[0], omega[0], b, digits);
  return (digits+1);
    #endif
  #endif
#endif /* THIRTYTWO_BIT_PROCESSOR */
  
}
