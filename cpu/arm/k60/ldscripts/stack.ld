    .stack :
    {
        _heap_end = .;
        __heap_end = _heap_end;
        /* stack and heap meet at their ends */
        __stack_start = .;
        __stack_size = LENGTH(stack);
        . = . + __stack_size;
        . = ALIGN(8);
        __stack_end = .;
        /* Startup code want to know where the stack should begin... */
        __stack_top = .;
    } >stack
