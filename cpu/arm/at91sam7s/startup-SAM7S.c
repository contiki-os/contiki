void
Reset(void)
{
  volatile int dummy =0;
}

static void
Vectors(void) __attribute__ ((naked, section(".vectrom")));

static void
Vectors(void)
{
  asm("ldr pc, %0\n"::"r" (Reset));
}



void foo_dummy()
{
  Vectors();
}
