static int interrupt_disabler_counter = 0;

void enable_interrupts(void)
{
    interrupt_disabler_counter--;
    if(interrupt_disabler_counter == 0)
    {
        asm(" CPSIE i");
    }
}

void disable_interrupts(void)
{
    interrupt_disabler_counter++;
    if(interrupt_disabler_counter > 0)
    {
        asm(" CPSID i");
    }
}
