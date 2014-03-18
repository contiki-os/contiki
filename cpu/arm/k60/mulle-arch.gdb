target remote :3333
set architecture arm
set arm force-mode thumb
set remote hardware-breakpoint-limit 6
set remote hardware-watchpoint-limit 4
load
