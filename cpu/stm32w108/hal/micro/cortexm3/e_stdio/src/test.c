
#include "UART0_stdio.h"

int main() {

char ent_char					;
int ent_dec_pos				;
int ent_dec_neg 				;
int ent_hex						;
int ent_oct						;
int ent_oct_hex 				;
long ent_slong  				;
unsigned short ent_ushort	;
unsigned long ent_ulong		;
char tab_char[3]				;


char nom [50]					;
char mois[50]					;
int jour							;
int annee						;
int *ptr							;
char tab[3]						; 
long double fld				;
float flottant					;
double f							;
double f4						;
long double f1					;
float f2, f3					;


  printf("Entrez un char:");
  scanf("%c" , &ent_char);
  printf("%c\n", ent_char);



  printf("Entrez trois caracteres:\n");
  scanf("%3c" , tab_char);
  printf("1er char  :%c\n", tab_char[0]);
  printf("2eme char :%c\n", tab_char[1]);
  printf("3eme char :%c\n", tab_char[2]);
  
  printf("Entrez un nombre decimal positif: ");
  scanf("%d" , &ent_dec_pos);
  printf("%d\n", ent_dec_pos);
 
  printf("Entrez un nombre decimal negatif: ");
  scanf("%d" , &ent_dec_neg);
  printf("%d\n", ent_dec_neg);
 
  printf("Entrez un nombre decimal long: ");
  scanf("%ld" , &ent_slong);
  printf("%ld\n", ent_slong);
  
  printf("Entrez un nombre decimal unsigned long: ");
  scanf("%lu" , &ent_ulong);
  printf("valeur entree: %lu\n", ent_ulong);
 
  printf("Entrez un nombre decimal unsigned short: ");
  scanf("%hu" , &ent_ushort);
  printf("valeur entree: %lu\n", ent_ushort);
 
 
  printf("Entrez un nombre en hexa: ");
  scanf("%x" , &ent_hex);
  printf("%x\n", ent_hex);
 
  printf("Entrez un nombre en octal: ");
  scanf("%o" , &ent_oct);
  printf("%o \n", ent_oct);
  
  printf("Entrez un nombre en octal ou hexa (preceder de 0 pour octal et de 0x ou 0X pour hexa), ");
  scanf("%i" , &ent_oct_hex);
  printf("valeur entree en decimal : %i \n", ent_oct_hex); 
  
  printf("Entrez une chaine de caracteres: ");
  scanf("%s" , nom);
  printf("%s \n", nom); 
 
  printf("Entrez le jour,le mois et l'annee:\n");
 
  scanf("%d%s%d", &jour, mois, &annee);
  printf("\njour:%d \n",jour);
  printf("mois:%s \n",mois);
  printf("annee:%d \n",annee); 
  
  

  // Dans le cas du format %[...] : le scanf se termine lorsqu'un caractere n'appartenant pas a 
  // l'ensemble est detecte, inversement si on specifie %[^...] le scanf s'arrete lorsque'un 
  //caractere de l'ensembles a ete lu
 
  printf("Entrez une chaine de caracteres en majuscules: ");
  scanf("%[A-Z]" , nom);
  printf("%s \n", nom); 
 
  printf("Entrez une chaine de caracteres sans majuscules pour terminer le scanf entrez une majuscule: ");
  scanf("%[^A-Z]" , nom);
  printf("%s \n", nom); 
  
  printf("Entrez une adresse memoire quelconque \n");
  scanf("%p",&ptr);
  printf("L'adresse %p contient la valeur %d ",ptr,*ptr);
  
/* printf("Entrez un caractere: ");
  scanf("%c" , &ent_char);
  __io_ungetc(ent_char);
  scanf("%c" , &ent_char);
  printf("Apres un scanf suivi d'un ungetc et d'un scanf on a : %c \n", ent_char); 

  printf("Entrez une chaine de 2 caracteres\n ");
  scanf("%s" , nom);  
  printf("la chaine entree est %s \n",nom);
  ent_char = __io_ungetc(nom[0]); 
  scanf("%c" , nom[0]) ;
  printf("Apres un ungetc et d'un scanf on a : %s \n", nom); 
  */

 printf("Entrer un float:\n");
  scanf("%f",&flottant);
  printf("Le float entre est %f",flottant);
 
  printf("Entrer un double float:\n");
  scanf("%Lf",&f);
  printf("Le float entre est %Lf\n",f);


  printf("Entrer un nombre avec exposant :\n");  
  scanf("%le",&f);
  printf("Le float entre est %le\n",f);
  
 // Note : le format %g choisit en fonction de la valeur entree le format le plus
 // appropriée entre %e et %f 
  
  
  printf("Entrer un nombre avec exposant :\n");  
  scanf("%lg",&f);
  printf("Le float entre est %lg\n",f);
  
  printf("Entrer un nombre avec exposant :\n");  
  scanf("%Lg",&fld);
  printf("Le float entre est %Lg\n",fld);


f1 = 48656568.256479123456789123456789;
f = 48656568.256479123456789123456789;
f2 = 456.45366;
f3 = 456.45362;
printf("Test for Floating points numbers printf\n");

/*Simple test of %f format */
	printf("double :%lf\n",f);

/* Test with format specifying first number is equal to minimal number
   of caracter to be print the second one is number of digits */
   
printf("LONG DOUBLE  :%Lf - %20.10Lf - %20.15Lf - %20.20Lf - %30.30Lf\n", f1,  f1, f1, f1, f1);
printf("float2    :%4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);

/*Note: the output should be float2: 3.14 +3e+000 3.141600E+000*/

   printf("float3    :%7.3f\n", 1.2345);
	printf("float3bis :%7.3lf\n",4865.256479 );
	printf("float4    :%10.3f\n", 1.2345e3);
	printf("float5    :%10.3f\n", 1.2345e7);
	printf("float6    :%12.4e\n", 1.2345);
	printf("float7    :%12.4e\n", 123.456789e8);
	printf("float8    :%15.5lf\n",48656568.256479 );
	printf("float9    :%15.6lf\n",48656568.256479 - 48656568.256478 );
	printf("float9bis :%-15.6lf%7.4f\n",48656568.256479 - 48656568.256478,1.2345 ); 
	printf("float9ter :%15.2lf\n",f2*f3 );

/*Note : the outputs shoud be
for 1.2345, ^^1.235
for 1.2345e5, ^^1234.500
for 1.2345e7, 12345000.000
for 1.2345, ^^1.2345e+00
for 123.456789e8, ^^1.2346e+10

for float 9: 48656568.256479 - 48656568.2563,^^^^^^^^0.00001
for float 9bis: 48656568.256479 - 48656568.2563,0.00001^^^^^^^^1.2345
for f2*f3 ,^^^^^^208349,92
^ is equal to a space */


	printf("float10   :01234567 \n" );
	printf("float11   :%8g|\n", 12.34 );
	printf("float12   :%8g|\n", 1234.5678 );
	printf("float13   :%8g|\n", 0.0478 );
	printf("float14   :%8g|\n", 422121.0 );
	printf("float15   :%8g|\n", 422121234.345345 );
	
/*Note : outputs should be
01234567
   12.34|
 1234.57|
  0.0478|
  422121|
4.22121e+08|
*/


printf("float16   :%.0f|\n",  1000.123456789123456789 );
printf("float17   :%.1f|\n",  2000.123456789123456789 );
printf("float18   :%.2f|\n",  3000.123456789123456789 );
printf("float19   :%.10f|\n", 4000.123456789123456789 );
printf("float20   :%.30f|\n", 5000.123456789123456789 );
printf("float21   :%f|\n",    6000.123456789123456789 );
printf("float22   :%.f|\n",   7000.123456789123456789 ); 

/*Note : outputs should be
1000|
2000.1|
3000.12|
4000.1234567891|
5000.12345678912333823973312939761|
6000.123457|
7000|
*/
  
int    a      				;
char   c     				;
float  ft    				;
int    hex    				;
double db     				;
char   stg[50]="chaine" ;

a=1;
// Test du printf avec une suite de parametres int
printf("Test suite de int: \n a=%d\na+1=%d\na+2=%d\na+3=%d\na+4=%d\na+5=%d\na+6=%d\na+7=%d\na+8=%d\na=%d\n",a,a+1,a+2,a+3,a+4,a+5,a+6,a+7,a+8,a);

//Test du printf avec une suite de floats
 ft=1.589634 ;
 printf("Test suite de floats: \nft=%f\nft+0.1=%f\nft+0.01=%f\nft+0.001=%f\nft+0.0001=%f\nft+0.00001=%f\n",ft,ft+0.1,ft+0.01,ft+0.001,ft+0.0001,ft+0.00001);
  
// Test du printf avec un melange de formats  

a   = 1        ;
c   ='c'       ;
ft  = 1.963214 ;
db  = 1.589e+15;
hex = 0x0FA    ;

	printf("Test avec plusieurs formats:\na=%d\nc=%c\nstg=%s\nft=%6.5f\ndb=%10.2e\nhex=%x\n",a,c,stg,ft,db,hex); 
	printf("Entrez dans l'ordre un int\n un char\n une chaine\nun float\nun float avec exposant\nun hexa \n");
	scanf("%d%c%s%f%le%x",&a,&c,stg,&ft,&db,&hex);
	printf("Test avec plusieurs formats apres un scanf:\n a=%d\nc=%c\nstg=%s\nft=%6.5f\ndb=%10.2le\nhex=0x%x\n",a,c,stg,ft,db,hex);
	
   return 0;

 }


