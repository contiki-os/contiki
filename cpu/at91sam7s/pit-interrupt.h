volatile unsigned int pit_count = 0;

static void NACKEDFUNC ATTR system_int (void) {        /* System Interrupt Handler */

  ISR_ENTRY();

  if (*AT91C_PITC_PISR & AT91C_PITC_PITS) {  /* Check PIT Interrupt */
    pit_count++;
    /*
    if ((pit_count % 100) == 0) {
      unsigned int led_state = (pit_count % 300) / 100;
      *AT91C_PIOA_ODSR = ~(1<<led_state);
    }
    */

    *AT91C_AIC_EOICR = *AT91C_PITC_PIVR;     /* Ack & End of Interrupt */
  } else {
    *AT91C_AIC_EOICR = 0;                   /* End of Interrupt */
  }
  
  ISR_EXIT();
}
