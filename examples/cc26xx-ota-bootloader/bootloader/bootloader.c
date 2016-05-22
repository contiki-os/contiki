int
main(void)
{

  //  (1) Load address of reset function from the fixed location
  //      of the image's reset vector and jump.
  __asm("LDR R0, =0x1004"); //  RESET vector of target image
  __asm("LDR R1, [R0]");    //  Get the branch address
  __asm("ORR R1, #1");      //  Make sure the Thumb State bit is set.
  __asm("BX R1");           //  Branch execution

  return 0;
}
