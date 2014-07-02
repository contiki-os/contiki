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
    
    para->omega[0] = 0x01;
    para->omega[12] = 0x02;	  
    //cure that will be used
    //a
    para->E.a[15] = 0xD6;
    para->E.a[14] = 0x03;
    para->E.a[13] = 0x19;
    para->E.a[12] = 0x98;
    para->E.a[11] = 0xD1;
    para->E.a[10] = 0xB3;
    para->E.a[9] = 0xBB;
    para->E.a[8] = 0xFE;
    para->E.a[7] = 0xBF;
    para->E.a[6] = 0x59;
    para->E.a[5] = 0xCC;
    para->E.a[4] = 0x9B;
    para->E.a[3] = 0xBF;
    para->E.a[2] = 0xF9;
    para->E.a[1] = 0xAE;
    para->E.a[0] = 0xE1;
   
    para->E.a_minus3 = FALSE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[15] = 0x5E;
    para->E.b[14] = 0xEE;
    para->E.b[13] = 0xFC;
    para->E.b[12] = 0xA3;
    para->E.b[11] = 0x80;
    para->E.b[10] = 0xD0;
    para->E.b[9] = 0x29;
    para->E.b[8] = 0x19;
    para->E.b[7] = 0xDC;
    para->E.b[6] = 0x2C;
    para->E.b[5] = 0x65;
    para->E.b[4] = 0x58;
    para->E.b[3] = 0xBB;
    para->E.b[2] = 0x6D;
    para->E.b[1] = 0x8A;
    para->E.b[0] = 0x5D;
          
    //base point
    para->G.x[15] =  0x7B;
    para->G.x[14] =  0x6A;
    para->G.x[13] =  0xA5;
    para->G.x[12] =  0xD8;
    para->G.x[11] =  0x5E;
    para->G.x[10] =  0x57;
    para->G.x[9] =  0x29;
    para->G.x[8] =  0x83;
    para->G.x[7] =  0xE6;
    para->G.x[6] =  0xFB;
    para->G.x[5] =  0x32;
    para->G.x[4] =  0xA7;
    para->G.x[3] =  0xCD;
    para->G.x[2] =  0xEB;
    para->G.x[1] =  0xC1;
    para->G.x[0] =  0x40;
   
    para->G.y[15] =  0x27;
    para->G.y[14] =  0xB6;
    para->G.y[13] =  0x91;
    para->G.y[12] =  0x6A;
    para->G.y[11] =  0x89;
    para->G.y[10] =  0x4D;
    para->G.y[9] =  0x3A;
    para->G.y[8] =  0xEE;
    para->G.y[7] =  0x71;
    para->G.y[6] =  0x06;
    para->G.y[5] =  0xFE;
    para->G.y[4] =  0x80;
    para->G.y[3] =  0x5F;
    para->G.y[2] =  0xC3;
    para->G.y[1] =  0x4B;
    para->G.y[0] =  0x44;
          	
    //prime divide the number of points
    para->r[15] = 0x3F;
    para->r[14] = 0xFF;
    para->r[13] = 0xFF;
    para->r[12] = 0xFF;
    para->r[11] = 0x7F;
    para->r[10] = 0xFF;
    para->r[9] = 0xFF;
    para->r[8] = 0xFF;
    para->r[7] = 0xBE;
    para->r[6] = 0x00;
    para->r[5] = 0x24;
    para->r[4] = 0x72;
    para->r[3] = 0x06;
    para->r[2] = 0x13;
    para->r[1] = 0xB5;
    para->r[0] = 0xA3;

     
    /* EIGHT_BIT_PROCESSOR */
DOMAIN_PARAM SIXTEEN_BIT_PROCESSOR

    //init parameters
    //prime
    para->p[7] = 0xFFFF;
    para->p[6] = 0xFFFD;
    para->p[5] = 0xFFFF;
    para->p[4] = 0xFFFF;
    para->p[3] = 0xFFFF;
    para->p[2] = 0xFFFF;
    para->p[1] = 0xFFFF;
    para->p[0] = 0xFFFF;
       
    para->omega[0] = 0x0001;
    para->omega[6] = 0x0002;  
    //cure that will be used
    //a
    para->E.a[7] = 0xD603;
    para->E.a[6] = 0x1998;
    para->E.a[5] = 0xD1B3;
    para->E.a[4] = 0xBBFE;
    para->E.a[3] = 0xBF59;
    para->E.a[2] = 0xCC9B;
    para->E.a[1] = 0xBFF9;
    para->E.a[0] = 0xAEE1;
      
    para->E.a_minus3 = FALSE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[7] = 0x5EEE;
    para->E.b[6] = 0xFCA3;
    para->E.b[5] = 0x80D0;
    para->E.b[4] = 0x2919;
    para->E.b[3] = 0xDC2C;
    para->E.b[2] = 0x6558;
    para->E.b[1] = 0xBB6D;
    para->E.b[0] = 0x8A5D;
             
    //base point
    para->G.x[7] =  0x7B6A;
    para->G.x[6] =  0xA5D8;
    para->G.x[5] =  0x5E57;
    para->G.x[4] =  0x2983;
    para->G.x[3] =  0xE6FB;
    para->G.x[2] =  0x32A7;
    para->G.x[1] =  0xCDEB;
    para->G.x[0] =  0xC140;

    para->G.y[7] =  0x27B6;
    para->G.y[6] =  0x916A;
    para->G.y[5] =  0x894D;
    para->G.y[4] =  0x3AEE;
    para->G.y[3] =  0x7106;
    para->G.y[2] =  0xFE80;
    para->G.y[1] =  0x5FC3;
    para->G.y[0] =  0x4B44;
             	
    //prime divide the number of points
    para->r[7] = 0x3FFF;
    para->r[6] = 0xFFFF;
    para->r[5] = 0x7FFF;
    para->r[4] = 0xFFFF;
    para->r[3] = 0xBE00;
    para->r[2] = 0x2472;
    para->r[1] = 0x0613;
    para->r[0] = 0xB5A3;
          
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
    para->E.a[3] = 0xD6031998;
    para->E.a[2] = 0xD1B3BBFE;
    para->E.a[1] = 0xBF59CC9B;
    para->E.a[0] = 0xBFF9AEE1;
      
    para->E.a_minus3 = FALSE;
    para->E.a_zero = FALSE;
   
    //b
    para->E.b[4] = 0x00000000;
    para->E.b[3] = 0x5EEEFCA3;
    para->E.b[2] = 0x80D02919;
    para->E.b[1] = 0xDC2C6558;
    para->E.b[0] = 0xBB6D8A5D;
             
    //base point
    para->G.x[4] = 0x00000000;
    para->G.x[3] = 0x7B6AA5D8;
    para->G.x[2] = 0x5E572983;
    para->G.x[1] = 0xE6FB32A7;
    para->G.x[0] = 0xCDEBC140;

    para->G.y[4] = 0x00000000;
    para->G.y[3] = 0x27B6916A;
    para->G.y[2] = 0x894D3AEE;
    para->G.y[1] = 0x7106FE80;
    para->G.y[0] = 0x5FC34B44;
             	
    //prime divide the number of points
    para->r[4] = 0x00000000;
    para->r[3] = 0x3FFFFFFF;
    para->r[2] = 0x7FFFFFFF;
    para->r[1] = 0xBE002472;
    para->r[0] = 0x0613B5A3;
    
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
