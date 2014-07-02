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
    para->p[8] = 0xFE;
    para->p[7] = 0xFF;
    para->p[6] = 0xFF;
    para->p[5] = 0xFF;
    para->p[4] = 0xFF;
    para->p[3] = 0xFF;
    para->p[2] = 0xFF;
    para->p[1] = 0xFF;
    para->p[0] = 0xFF;

    para->omega[0] = 0x01;
    para->omega[8] = 0x01;
    
    //cure that will be used
    //a
    para->E.a[24] =  0x0;
    para->E.a[23] =  0xFF;
    para->E.a[22] =  0xFF;
    para->E.a[21] =  0xFF;
    para->E.a[20] =  0xFF;
    para->E.a[19] =  0xFF;
    para->E.a[18] =  0xFF;
    para->E.a[17] =  0xFF;
    para->E.a[16] =  0xFF;
    para->E.a[15] =  0xFF;
    para->E.a[14] =  0xFF;
    para->E.a[13] =  0xFF;
    para->E.a[12] =  0xFF;
    para->E.a[11] =  0xFF;
    para->E.a[10] =  0xFF;
    para->E.a[9] =  0xFF;
    para->E.a[8] =  0xFE;
    para->E.a[7] =  0xFF;
    para->E.a[6] =  0xFF;
    para->E.a[5] =  0xFF;
    para->E.a[4] =  0xFF;
    para->E.a[3] =  0xFF;
    para->E.a[2] =  0xFF;
    para->E.a[1] =  0xFF;
    para->E.a[0] =  0xFC;
    
    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
    
    //b
    para->E.b[24] =  0x0;
    para->E.b[23] =  0x64;
    para->E.b[22] =  0x21;
    para->E.b[21] =  0x05;
    para->E.b[20] =  0x19;
    para->E.b[19] =  0xE5;
    para->E.b[18] =  0x9C;
    para->E.b[17] =  0x80;
    para->E.b[16] =  0xE7;
    para->E.b[15] =  0x0F;
    para->E.b[14] =  0xA7;
    para->E.b[13] =  0xE9;
    para->E.b[12] =  0xAB;
    para->E.b[11] =  0x72;
    para->E.b[10] =  0x24;
    para->E.b[9] =  0x30;
    para->E.b[8] =  0x49;
    para->E.b[7] =  0xFE;
    para->E.b[6] =  0xB8;
    para->E.b[5] =  0xDE;
    para->E.b[4] =  0xEC;
    para->E.b[3] =  0xC1;
    para->E.b[2] =  0x46;
    para->E.b[1] =  0xB9;
    para->E.b[0] =  0xB1;

    //base point
    para->G.x[24] =  0x0;
    para->G.x[23] =  0x18;
    para->G.x[22] =  0x8D;
    para->G.x[21] =  0xA8;
    para->G.x[20] =  0x0E;
    para->G.x[19] =  0xB0;
    para->G.x[18] =  0x30;
    para->G.x[17] =  0x90;
    para->G.x[16] =  0xF6;
    para->G.x[15] =  0x7C;
    para->G.x[14] =  0xBF;
    para->G.x[13] =  0x20;
    para->G.x[12] =  0xEB;
    para->G.x[11] =  0x43;
    para->G.x[10] =  0xA1;
    para->G.x[9] =  0x88;
    para->G.x[8] =  0x00;
    para->G.x[7] =  0xF4;
    para->G.x[6] =  0xFF;
    para->G.x[5] =  0x0A;
    para->G.x[4] =  0xFD;
    para->G.x[3] =  0x82;
    para->G.x[2] =  0xFF;
    para->G.x[1] =  0x10;
    para->G.x[0] =  0x12;
	
    para->G.y[24] =  0x0;
    para->G.y[23] =  0x07;
    para->G.y[22] =  0x19;
    para->G.y[21] =  0x2b;
    para->G.y[20] =  0x95;
    para->G.y[19] =  0xff;
    para->G.y[18] =  0xc8;
    para->G.y[17] =  0xda;
    para->G.y[16] =  0x78;
    para->G.y[15] =  0x63;
    para->G.y[14] =  0x10;
    para->G.y[13] =  0x11;
    para->G.y[12] =  0xed;
    para->G.y[11] =  0x6b;
    para->G.y[10] =  0x24;
    para->G.y[9] =  0xcd;
    para->G.y[8] =  0xd5;
    para->G.y[7] =  0x73;
    para->G.y[6] =  0xf9;
    para->G.y[5] =  0x77;
    para->G.y[4] =  0xa1;
    para->G.y[3] =  0x1e;
    para->G.y[2] =  0x79;
    para->G.y[1] =  0x48;
    para->G.y[0] =  0x11;
	
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
    para->r[12] = 0xFF;
    para->r[11] = 0x99;
    para->r[10] = 0xDE;
    para->r[9] = 0xF8;
    para->r[8] = 0x36;
    para->r[7] = 0x14;
    para->r[6] = 0x6B;
    para->r[5] = 0xC9;
    para->r[4] = 0xB1;
    para->r[3] = 0xB4;
    para->r[2] = 0xD2;
    para->r[1] = 0x28;
    para->r[0] = 0x31;	
     
    /* EIGHT_BIT_PROCESSOR */
#else
  #ifdef SIXTEEN_BIT_PROCESSOR

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
    para->p[4] = 0xFFFE;
    para->p[3] = 0xFFFF;
    para->p[2] = 0xFFFF;
    para->p[1] = 0xFFFF;
    para->p[0] = 0xFFFF;
    
    memset(para->omega, 0, NUMWORDS*NN_DIGIT_LEN);
    para->omega[0] = 0x0001;
    para->omega[4] = 0x0001;     
    //cure that will be used
    //a
    memset(para->E.a, 0, NUMWORDS*NN_DIGIT_LEN);
    para->E.a[11] =  0xFFFF;
    para->E.a[10] =  0xFFFF;
    para->E.a[9] =  0xFFFF;
    para->E.a[8] =  0xFFFF;
    para->E.a[7] =  0xFFFF;
    para->E.a[6] =  0xFFFF;
    para->E.a[5] =  0xFFFF;
    para->E.a[4] =  0xFFFE;
    para->E.a[3] =  0xFFFF;
    para->E.a[2] =  0xFFFF;
    para->E.a[1] =  0xFFFF;
    para->E.a[0] =  0xFFFC;
         
    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
    
    //b
    memset(para->E.b, 0, NUMWORDS*NN_DIGIT_LEN);
    para->E.b[11] =  0x6421;
    para->E.b[10] =  0x0519;
    para->E.b[9] =  0xE59C;
    para->E.b[8] =  0x80E7;
    para->E.b[7] =  0x0FA7;
    para->E.b[6] =  0xE9AB;
    para->E.b[5] =  0x7224;
    para->E.b[4] =  0x3049;
    para->E.b[3] =  0xFEB8;
    para->E.b[2] =  0xDEEC;
    para->E.b[1] =  0xC146;
    para->E.b[0] =  0xB9B1;
     
    //base point
    memset(para->G.x, 0, NUMWORDS*NN_DIGIT_LEN);
    para->G.x[11] =  0x188D;
    para->G.x[10] =  0xA80E;
    para->G.x[9] =  0xB030;
    para->G.x[8] =  0x90F6;
    para->G.x[7] =  0x7CBF;
    para->G.x[6] =  0x20EB;
    para->G.x[5] =  0x43A1;
    para->G.x[4] =  0x8800;
    para->G.x[3] =  0xF4FF;
    para->G.x[2] =  0x0AFD;
    para->G.x[1] =  0x82FF;
    para->G.x[0] =  0x1012;

    memset(para->G.y, 0, NUMWORDS*NN_DIGIT_LEN);
    para->G.y[11] =  0x0719;
    para->G.y[10] =  0x2B95;
    para->G.y[9] =  0xFFC8;
    para->G.y[8] =  0xDA78;
    para->G.y[7] =  0x6310;
    para->G.y[6] =  0x11ED;
    para->G.y[5] =  0x6B24;
    para->G.y[4] =  0xCDD5;
    para->G.y[3] =  0x73F9;
    para->G.y[2] =  0x77A1;
    para->G.y[1] =  0x1E79;
    para->G.y[0] =  0x4811;
    	
    //prime divide the number of points
    memset(para->r, 0, NUMWORDS*NN_DIGIT_LEN);
    para->r[11] = 0xFFFF;
    para->r[10] = 0xFFFF;
    para->r[9] = 0xFFFF;
    para->r[8] = 0xFFFF;
    para->r[7] = 0xFFFF;
    para->r[6] = 0xFFFF;
    para->r[5] = 0x99DE;
    para->r[4] = 0xF836;
    para->r[3] = 0x146B;
    para->r[2] = 0xC9B1;
    para->r[1] = 0xB4D2;
    para->r[0] = 0x2831;	
  
   /* SIXTEEN_BIT_PROCESSOR */
   #else
    #ifdef THIRTYTWO_BIT_PROCESSOR

    //init parameters
    //prime (Group Prime/Irreducible Polynomial)
    memset(para->p, 0, NUMWORDS*NN_DIGIT_LEN);
    para->p[5] = 0xFFFFFFFF;
    para->p[4] = 0xFFFFFFFF;
    para->p[3] = 0xFFFFFFFF;
    para->p[2] = 0xFFFFFFFE;
    para->p[1] = 0xFFFFFFFF;
    para->p[0] = 0xFFFFFFFF;
    
    memset(para->omega, 0, NUMWORDS*NN_DIGIT_LEN);
    para->omega[0] = 0x0000001;
    para->omega[2] = 0x0000001;     
    //curve that will be used
    //a (is -3 in RFC4753)
    memset(para->E.a, 0, NUMWORDS*NN_DIGIT_LEN);
    para->E.a[5] =  0xFFFFFFFF;
    para->E.a[4] =  0xFFFFFFFF;
    para->E.a[3] =  0xFFFFFFFF;
    para->E.a[2] =  0xFFFFFFFE;
    para->E.a[1] =  0xFFFFFFFF;
    para->E.a[0] =  0xFFFFFFFC;
         
    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
    
    // b
    memset(para->E.b, 0, NUMWORDS*NN_DIGIT_LEN);
    para->E.b[5] =  0x64210519;
    para->E.b[4] =  0xE59C80E7;
    para->E.b[3] =  0x0FA7E9AB;
    para->E.b[2] =  0x72243049;
    para->E.b[1] =  0xFEB8DEEC;
    para->E.b[0] =  0xC146B9B1;
     
    //base point (G)
    memset(para->G.x, 0, NUMWORDS*NN_DIGIT_LEN);
    para->G.x[5] =  0x188DA80E;
    para->G.x[4] =  0xB03090F6;
    para->G.x[3] =  0x7CBF20EB;
    para->G.x[2] =  0x43A18800;
    para->G.x[1] =  0xF4FF0AFD;
    para->G.x[0] =  0x82FF1012;

    memset(para->G.y, 0, NUMWORDS*NN_DIGIT_LEN);
    para->G.y[5] =  0x07192B95;
    para->G.y[4] =  0xFFC8DA78;
    para->G.y[3] =  0x631011ED;
    para->G.y[2] =  0x6B24CDD5;
    para->G.y[1] =  0x73F977A1;
    para->G.y[0] =  0x1E794811;
    	
    //prime divide the number of points (Group Order)
    memset(para->r, 0, NUMWORDS*NN_DIGIT_LEN);
    para->r[5] = 0xFFFFFFFF;
    para->r[4] = 0xFFFFFFFF;
    para->r[3] = 0xFFFFFFFF;
    para->r[2] = 0x99DEF836;
    para->r[1] = 0x146BC9B1;
    para->r[0] = 0xB4D22831;
    #else
      #error "secp192r1.c: Word size not defined"
    #endif
  #endif
#endif /* THIRTYTWO_BIT_PROCESSOR */
}
  
NN_UINT 
omega_mul(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *omega, NN_UINT digits)
{
  
#ifdef EIGHT_BIT_PROCESSOR

    NN_Assign(a, b, digits);
    NN_Add(a+8, a+8, b, digits+1);
    return (digits+9);
    /* EIGHT_BIT_PROCESSOR */  
#else
  #ifdef SIXTEEN_BIT_PROCESSOR

    NN_Assign(a, b, digits);
    NN_Add(a+4, a+4, b, digits+1);
    return (digits+5);

    /* SIXTEEN_BIT_PROCESSOR */  
  #else
    #ifdef THIRTYTWO_BIT_PROCESSOR

    NN_Assign(a, b, digits);
    NN_Add(&a[2], &a[2], b, digits+1);
    return (digits+3);
   #endif
 #endif
#endif /* THIRTYTWO_BIT_PROCESSOR */
  
}
