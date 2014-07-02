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
    para->p[16] = 0x00;
    para->p[15] = 0xFF;
    para->p[14] = 0xFF;
    para->p[13] = 0xFF;
    para->p[12] = 0xFD;
    para->p[11] = 0xFF;
    para->p[10] = 0xFF;
    para->p[9] = 0xFF;
    para->p[8] = 0xFF;
    para->p[7] = 0xFF;
    para->p[6] = 0xFF;
    para->p[5] = 0xFF;
    para->p[4] = 0xFF;
    para->p[3] = 0xFF;
    para->p[2] = 0xFF;
    para->p[1] = 0xFF;
    para->p[0] = 0xFF;
    
    memset(para->omega, 0, NUMWORDS*NN_DIGIT_LEN);
    para->omega[0] = 0x01;
    para->omega[12] = 0x02;	  
    //cure that will be used
    //a
    para->E.a[16] = 0x00;
    para->E.a[15] = 0xFF;
    para->E.a[14] = 0xFF;
    para->E.a[13] = 0xFF;
    para->E.a[12] = 0xFD;
    para->E.a[11] = 0xFF;
    para->E.a[10] = 0xFF;
    para->E.a[9] = 0xFF;
    para->E.a[8] = 0xFF;
    para->E.a[7] = 0xFF;
    para->E.a[6] = 0xFF;
    para->E.a[5] = 0xFF;
    para->E.a[4] = 0xFF;
    para->E.a[3] = 0xFF;
    para->E.a[2] = 0xFF;
    para->E.a[1] = 0xFF;
    para->E.a[0] = 0xFC;

    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[16] = 0x00;
    para->E.b[15] = 0xE8;
    para->E.b[14] = 0x75;
    para->E.b[13] = 0x79;
    para->E.b[12] = 0xC1;
    para->E.b[11] = 0x10;
    para->E.b[10] = 0x79;
    para->E.b[9] = 0xF4;
    para->E.b[8] = 0x3D;
    para->E.b[7] = 0xD8;
    para->E.b[6] = 0x24;
    para->E.b[5] = 0x99;
    para->E.b[4] = 0x3C;
    para->E.b[3] = 0x2C;
    para->E.b[2] = 0xEE;
    para->E.b[1] = 0x5E;
    para->E.b[0] = 0xD3;
       
    //base point
    para->G.x[16] = 0x00;
    para->G.x[15] =  0x16;
    para->G.x[14] =  0x1F;
    para->G.x[13] =  0xF7;
    para->G.x[12] =  0x52;
    para->G.x[11] =  0x8B;
    para->G.x[10] =  0x89;
    para->G.x[9] =  0x9B;
    para->G.x[8] =  0x2D;
    para->G.x[7] =  0x0C;
    para->G.x[6] =  0x28;
    para->G.x[5] =  0x60;
    para->G.x[4] =  0x7C;
    para->G.x[3] =  0xA5;
    para->G.x[2] =  0x2C;
    para->G.x[1] =  0x5B;
    para->G.x[0] =  0x86;

    para->G.y[16] = 0x00;
    para->G.y[15] =  0xCF;
    para->G.y[14] =  0x5A;
    para->G.y[13] =  0xC8;
    para->G.y[12] =  0x39;
    para->G.y[11] =  0x5B;
    para->G.y[10] =  0xAF;
    para->G.y[9] =  0xEB;
    para->G.y[8] =  0x13;
    para->G.y[7] =  0xC0;
    para->G.y[6] =  0x2D;
    para->G.y[5] =  0xA2;
    para->G.y[4] =  0x92;
    para->G.y[3] =  0xDD;
    para->G.y[2] =  0xED;
    para->G.y[1] =  0x7A;
    para->G.y[0] =  0x83;
       	
    //prime divide the number of points
    para->r[16] = 0x00;
    para->r[15] = 0xFF;
    para->r[14] = 0xFF;
    para->r[13] = 0xFF;
    para->r[12] = 0xFE;
    para->r[11] = 0x0;
    para->r[10] = 0x0;
    para->r[9] = 0x0;
    para->r[8] = 0x0;
    para->r[7] = 0x75;
    para->r[6] = 0xA3;
    para->r[5] = 0x0D;
    para->r[4] = 0x1B;
    para->r[3] = 0x90;
    para->r[2] = 0x38;
    para->r[1] = 0xA1;
    para->r[0] = 0x15;

     
    /* EIGHT_BIT_PROCESSOR */
DOMAIN_PARAM SIXTEEN_BIT_PROCESSOR

     //init parameters
    //prime
    para->p[8] = 0x0000;
    para->p[7] = 0xFFFF;
    para->p[6] = 0xFFFD;
    para->p[5] = 0xFFFF;
    para->p[4] = 0xFFFF;
    para->p[3] = 0xFFFF;
    para->p[2] = 0xFFFF;
    para->p[1] = 0xFFFF;
    para->p[0] = 0xFFFF;
   
    memset(para->omega, 0, NUMWORDS*NN_DIGIT_LEN);
    para->omega[0] = 0x0001;
    para->omega[6] = 0x0002;	  
    //cure that will be used
    //a
    para->E.a[8] = 0x0000;
    para->E.a[7] = 0xFFFF;
    para->E.a[6] = 0xFFFD;
    para->E.a[5] = 0xFFFF;
    para->E.a[4] = 0xFFFF;
    para->E.a[3] = 0xFFFF;
    para->E.a[2] = 0xFFFF;
    para->E.a[1] = 0xFFFF;
    para->E.a[0] = 0xFFFC;
   
    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[8] = 0x0000;
    para->E.b[7] = 0xE875;
    para->E.b[6] = 0x79C1;
    para->E.b[5] = 0x1079;
    para->E.b[4] = 0xF43D;
    para->E.b[3] = 0xD824;
    para->E.b[2] = 0x993C;
    para->E.b[1] = 0x2CEE;
    para->E.b[0] = 0x5ED3;
          
    //base point
    para->G.x[8] =  0x0000;
    para->G.x[7] =  0x161F;
    para->G.x[6] =  0xF752;
    para->G.x[5] =  0x8B89;
    para->G.x[4] =  0x9B2D;
    para->G.x[3] =  0x0C28;
    para->G.x[2] =  0x607C;
    para->G.x[1] =  0xA52C;
    para->G.x[0] =  0x5B86;
   
    para->G.y[8] =  0x0000;
    para->G.y[7] =  0xCF5A;
    para->G.y[6] =  0xC839;
    para->G.y[5] =  0x5BAF;
    para->G.y[4] =  0xEB13;
    para->G.y[3] =  0xC02D;
    para->G.y[2] =  0xA292;
    para->G.y[1] =  0xDDED;
    para->G.y[0] =  0x7A83;
          	
    //prime divide the number of points
    para->r[8] = 0x0000;
    para->r[7] = 0xFFFF;
    para->r[6] = 0xFFFE;
    para->r[5] = 0x0000;
    para->r[4] = 0x0000;
    para->r[3] = 0x75A3;
    para->r[2] = 0x0D1B;
    para->r[1] = 0x9038;
    para->r[0] = 0xA115;
          
   /* SIXTEEN_BIT_PROCESSOR */
DOMAIN_PARAM THIRTYTWO_BIT_PROCESSOR

   //init parameters
    //prime
    para->p[4] = 0x00000000;
    para->p[3] = 0xFFFFFFFD;
    para->p[2] = 0xFFFFFFFF;
    para->p[1] = 0xFFFFFFFF;
    para->p[0] = 0xFFFFFFFF;
    
    memset(para->omega, 0, NUMWORDS*NN_DIGIT_LEN);
    para->omega[0] = 0x00000001;
    para->omega[3] = 0x00000002;	  
    //cure that will be used
    //a
    para->E.a[4] = 0x00000000;
    para->E.a[3] = 0xFFFFFFFD;
    para->E.a[2] = 0xFFFFFFFF;
    para->E.a[1] = 0xFFFFFFFF;
    para->E.a[0] = 0xFFFFFFFC;
   
    para->E.a_minus3 = TRUE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[4] = 0x00000000;
    para->E.b[3] = 0xE87579C1;
    para->E.b[2] = 0x1079F43D;
    para->E.b[1] = 0xD824993C;
    para->E.b[0] = 0x2CEE5ED3;
          
    //base point
    para->G.x[4] =  0x00000000;
    para->G.x[3] =  0x161FF752;
    para->G.x[2] =  0x8B899B2D;
    para->G.x[1] =  0x0C28607C;
    para->G.x[0] =  0xA52C5B86;
   
    para->G.y[4] =  0x00000000;
    para->G.y[3] =  0xCF5AC839;
    para->G.y[2] =  0x5BAFEB13;
    para->G.y[1] =  0xC02DA292;
    para->G.y[0] =  0xDDED7A83;
          	
    //prime divide the number of points
    para->r[4] = 0x00000000;
    para->r[3] = 0xFFFFFFFE;
    para->r[2] = 0x00000000;
    para->r[1] = 0x75A30D1B;
    para->r[0] = 0x9038A115;
    
    printf("32 bit initialized\n");
#endif /* THIRTYTWO_BIT_PROCESSOR */

  }
  
NN_UINT 
omega_mul(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *omega, NN_UINT digits)
{
  
#ifdef EIGHT_BIT_PROCESSOR
    NN_Assign(a, b, digits);
    a[digits+12] += NN_AddDigitMult(&a[12], &a[12], omega[12], b, digits);
    return (digits+13);
  /* EIGHT_BIT_PROCESSOR */  
DOMAIN_PARAM SIXTEEN_BIT_PROCESSOR
    //memset(a, 0, digits*NN_DIGIT_LEN);
    NN_Assign(a, b, digits);
    a[digits+6] += NN_AddDigitMult(&a[6], &a[6], omega[6], b, digits);
    return (digits+7);

    /* SIXTEEN_BIT_PROCESSOR */  
DOMAIN_PARAM THIRTYTWO_BIT_PROCESSOR

  NN_Assign(a, b, digits);
  a[digits+3] += NN_AddDigitMult(&a[3], &a[3], omega[3], b, digits);
  return (digits+4);
  
#endif /* THIRTYTWO_BIT_PROCESSOR */
  
}
