
int main (){
double f;

/*double f4;
long double f1;
float f2, f3;
float flottant;*/

 f = 48656568.256479123456789123456789;
/*
f1= 48656568.256479123456789123456789;
f2 = 456.45366;
f3 = 456.45362; */
//printf("Test du printf");

/*Simple test of %f format */
printf("double    :%lf\n",f);

/* Test with format specifying first number is equal to minimal number
   of caracter to be print the second one is number of digits */
/*
printf("LONG DOUBLE  :%Lf - %20.10Lf - %20.15Lf - %20.20Lf - %30.30Lf\n", f1,  f1, f1, f1, f1);
printf("float2    :%4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);*/

/*Note: the output should be float2: 3.14 +3e+000 3.141600E+000*/
/*
printf("float3    :%7.3f\n", 1.2345);
printf("float3bis :%7.3lf\n",4865.256479 );
printf("float4    :%10.3f\n", 1.2345e3);
printf("float5    :%10.3f\n", 1.2345e7);
printf("float6    :%12.4e\n", 1.2345);
printf("float7    :%12.4e\n", 123.456789e8);
printf("float8    :%15.5lf\n",48656568.256479 );
printf("float9    :%15.6lf\n",48656568.256479 - 48656568.256478 );
printf("float9bis :%15.2lf\n",f2*f3 );*/
/*Note : the outputs shoud be
for 1.2345, ^^1.235
for 1.2345e5, ^^1234.500
for 1.2345e7, 12345000.000
for 1.2345, ^^1.2345e+00
for 123.456789e8, ^^1.2346e+10
for 48656568.256479 - 48656568.2563,^^^^^^^^0.00001
for f2*f3 ,^^^^^^208349,92
^ is equal to a space */

/*
printf("float10   :01234567 \n" );
printf("float11   :%8g|\n", 12.34 );
printf("float12   :%8g|\n", 1234.5678 );
printf("float13   :%8g|\n", 0.0478 );
printf("float14   :%8g|\n", 422121.0 );
printf("float15   :%8g|\n", 422121234.345345 );*/
/*Note : outputs should be
01234567
   12.34|
 1234.57|
  0.0478|
  422121|
4.22121e+08|
*/
/*

printf("float16   :%.0f|\n",  1000.123456789123456789 );
printf("float17   :%.1f|\n",  2000.123456789123456789 );
printf("float18   :%.2f|\n",  3000.123456789123456789 );
printf("float19   :%.10f|\n", 4000.123456789123456789 );
printf("float20   :%.30f|\n", 5000.123456789123456789 );
printf("float21   :%f|\n",    6000.123456789123456789 );
printf("float22   :%.f|\n",   7000.123456789123456789 );
*/
/*Note : outputs should be
1000|
2000.1|
3000.12|
4000.1234567891|
5000.12345678912333823973312939761|
6000.123457|
7000|
*/


//while(1);

}


