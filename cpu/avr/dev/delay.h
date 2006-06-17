#define F_CPU 15000000
 
#define K_DELAY_100us	F_CPU/61349
#define K_DELAY_1ms		F_CPU/6013
#define K_DELAY_10ms	F_CPU/600

void Delay_100us(unsigned char t);
void Delay_1ms(unsigned char t);
void Delay_10ms(unsigned char t);
