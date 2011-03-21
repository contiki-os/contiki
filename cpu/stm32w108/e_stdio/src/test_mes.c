
#define FLOATING
#define PRINT
#define SCANF


int main() {

char ent_char		;
float flottant		;

#ifndef FLOATING
#ifdef PRINT
 ent_char='a';
#endif

#ifdef SCANF
  scanf("%c" , &ent_char);
#endif

#ifdef PRINT
  printf("%c\n", ent_char);
#endif



#else
 #ifdef PRINT
 flottant = 1.456789;
#endif

#ifdef SCANF
  scanf("%f" , &flottant);
#endif

#ifdef PRINT
  printf("%f\n", flottant);
#endif

#endif
  return 0;

 }

