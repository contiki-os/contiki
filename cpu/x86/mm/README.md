X86 Lightweight Protection Domain Support for Contiki
=====================================================

Introduction
------------

The X86 port of Contiki implements a simple, lightweight form of
protection domains using a pluggable framework.  Currently, the
following plugin is available:

 - Flat memory model with paging.

For an introduction to paging and possible ways in which it can be
used, refer to the following resources:

 - Intel Combined Manual (Intel 64 and IA-32 Architectures Software
   Developer's Manual), Vol. 3, Chapter 4
 - Programming the 80386, by John H. Crawford and Patrick
   P. Gelsinger, Chapter 5

The overall goal of a protection domain implementation within this
framework is to define a set of resources that should be accessible to
each protection domain and to prevent that protection domain from
accessing other resources.  The details of each implementation of
protection domains may differ substantially, but they should all be
guided by the principle of least privilege [1].  However, that
idealized principle is balanced against the practical objectives of
limiting the number of relatively time-consuming context switches and
minimizing changes to existing code.  In fact, no changes were made to
code outside of the CPU- and platform-specific code directories for
the initial plugin.

Each protection domain can optionally be associated with a metadata
and/or MMIO region.  The hardware can support additional regions per
protection domain, but that would increase complexity and is unneeded
for the existing protection domains.

After boot, all code runs in the context of some protection domain.
Two default protection domains are implemented:

- kern: Kernel protection domain that is more privileged than any
  other protection domain.  As little code as possible should be placed
  in this protection domain.
- app: Application protection domain used whenever special privileges
  are not required.

Additional protection domains are defined as needed.  For example,
each driver may reside in a separate protection domain, although not
all drivers require additional privileges beyond those available in
the relevant scheduling context in the app protection domain.  The
Ethernet and UART drivers are assigned separate protection domains.
Non-driver protection domains can also be defined.  Other drivers only
require access to programmed IO ports accessible via the IN* and OUT*
instructions, and such drivers do not require separate protection
domains.  They run in the Contiki preemptive scheduling context and
the kernel protection domain, both of which are granted access to all
IO ports.

Each protection domain may have associated system calls.  A system
call transfers control from a client protection domain to a defined
entrypoint in a server protection domain.  As their name suggests,
system calls adhere to a synchronous call-return model (rather than
some alternative such as an asynchronous message-passing model).  To
invoke a system call, the client provides two identifiers to the
system call dispatcher.  The first identifies the server domain and
the second identifies the system call to be invoked.  The protection
domain implementation should associate allowable system calls with
particular server protection domains and reject any system call
requests that are not within that set of allowable system calls.  The
system call implementations do not restrict the clients that are
permitted to invoke each system call.  No modifications that the
client can make to the server domain and system call identifiers can
open up new entrypoints into the server domain.  The entrypoints are
fixed at boot time.

However, if the identifiers were stored in shared memory, it may be
possible for a protection domain to influence the system calls issued
by some other protection domain, which may be undesirable.  Thus, the
server domain identifiers are stored in memory that can only be
written by the kernel protection domain and the system call
identifiers are embedded in the code.

The system call dispatcher is responsible for reconfiguring the system
to enforce the appropriate resource access controls for the server
protection domain.  It should then transfer control to the approved
entrypoint for the requested system call.

Contiki defines a process concept that is orthogonal to protection
domains [2].  A single Contiki process may run code in multiple
protection domains at various points in time.  Contiki processes run
in a cooperative scheduling context.  Contiki also defines a
preemptive scheduling context for interrupt handlers and real-time
timers.  When protection domain support is enabled, interrupts are
only enabled when the application protection domain is active and is
running code in the cooperative scheduling context.  Code running in
the preemptive context may also invoke multiple protection domains.
Contiki can also support preemptive multithreading, but support for
that has not yet been added to the X86 port so we do not discuss it
further.

A single stack is shared by all code that runs in the cooperative
scheduling context in all protection domains, and separate stacks are
defined for short interrupt dispatchers in the preemptive scheduling
context and for exception handlers and software system call
dispatchers.  Except for the interrupt dispatchers, code in the
preemptive scheduling context also shares the same stack with the
cooperative scheduling context.  All protection domains also share a
main data section, so similar considerations are also relevant to
that.

Introducing multi-core support would complicate things further, since
another core running a protection domain that the first core never
invoked could access data from the protection domain on the first
core. It may be possible to adequately address such concerns by
allocating per-core stacks.

Note that this stack arrangement means that a given protection domain
may read and write data written to the stack by some other protection
domain.  For example, a protection domain B may push data onto the
stack and later pop that data off of the stack, but a protection
domain A that invoked protection domain B may still be able to read
the data that was pushed and popped to and from the stack, since
popping the data off of the stack does not automatically erase that
stack memory location.  Another possibility is that protection domain
B may modify a stack entry pushed by protection domain A before it
invoked protection domain B, and protection domain A may later use the
modified value.  Permitting legitimate accesses to callers' stacks is
in fact the primary motivation for this stack arrangement, in that it
makes it simple for A to pass data to and from B (on the shared stack)
when requesting services from B.  A system call invocation is nearly
transparent to the developer, appearing almost identical to an
ordinary function call.  However, B can access any data on the stack.
The third case is that A can read data placed on the stack by B after
B returns, unless B wipes that data from the stack before returning.
A related sub-case is that if an interrupt handler is invoked, it
pushes the current contents of the general-purpose registers onto the
stack, which may then be revealed to other protection domains besides
the one that was interrupted.  However, interrupts are only actually
enabled in the application protection domain.

Similarly, register contents may be accessed and modified across
protection domain boundaries.

For the reasons described above, each protection domain should only
invoke other protection domains that it trusts to properly handle data
on the stack.

Design
------

### Boot Process

The system boots in the following phases.

#### UEFI Bootstrap

Primary implementation sources:

 - cpu/x86/uefi/bootstrap_uefi.c

When the OS is compiled as a UEFI binary, a short bootstrap phase that
is UEFI-compliant is run initially.  It simply performs a minimal set
of functions to exit the UEFI boot services and then transfer control
to the Multiboot bootstrap phase.

#### Multiboot Bootstrap

Primary implementation sources:

 - cpu/x86/bootstrap_quarkX1000.S

This phase disables interrupts, sets the stack pointer to the top of
the main stack, and then invokes boot stage 0.

#### Boot Stage 0

Primary implementation sources:

 - cpu/x86/init/common/cpu.c
 - cpu/x86/init/common/gdt.c

The UEFI firmware or Multiboot-compliant bootloader should have
configured an initial Global Descriptor Table (GDT) with flat segments
and configured the CPU to operate in protected mode with paging
disabled.  Flat segments each map the whole 4GiB physical memory
space.  This is the state of the system when the OS enters boot stage
0.  This stage is responsible for setting up a new GDT and loading the
segment registers with the appropriate descriptors from the new GDT to
enable boot stage 1 to run.

#### Boot Stage 1

Primary implementation sources:

 - cpu/x86/init/common/cpu.c
 - cpu/x86/init/common/idt.c
 - cpu/x86/mm/prot-domains.c

Boot stage 1 intializes the Interrupt Descriptor Table (IDT) and
installs a handler for double-fault exceptions.  Handlers for
additional interrupts and exceptions are installed later in boot
stages 1 and 2.

This stage also initializes protection domain support and enters the
kernel protection domain.

#### Boot Stage 2

Primary implementation sources:

 - cpu/x86/init/common/cpu.c
 - platform/galileo/contiki-main.c

The entrypoint for the kernel protection domain is 'main'.  Boot stage
2 initializes hardware devices and associated interrupts.  It then
transfers control to the application protection domain.  Note that
this is a transfer of control, not a call that would be matched with
some future return.  This is an important distinction, because
protection domains are not reentrant.  Thus, if the kernel protection
domain called the application protection domain, it would not be
possible to invoke any kernel system calls until the system is reset,
since the application protection domain never exits/returns while the
system is running.  There are not actually any kernel system calls
provided in the initial implementation of protection domains, but they
may be added in the future.

The core protection domain configuration (e.g. allowable system calls
and entrypoints, registered protection domains, etc.) is frozen by the
conclusion of boot stage 2 to help prevent erroneous changes that
could reduce the robustness of the system.  The way that it is frozen
is that there are no kernel system calls that are intended to permit
changes to the core protection domain configuration.  Thus, once the
kernel protection domain has exited, the only way the core protection
domain configuration can change would be due to undesirable memory
manipulations (e.g. due to a faulty device driver).

#### Boot Stage 3

Primary implementation sources:

 - platform/galileo/contiki-main.c

Boot stage 3 performs initialization procedures that are less
tightly-coupled to hardware.  For example, it launches Contiki
processes and invokes Contiki configuration routines.

### Privilege Levels

When protection domain support is inactive, all code runs at
ring/privilege level 0.  When protection domain support is active,
only exception handlers and system call dispatchers (including
dispatchers for system call returns) run at ring level 0.  Code in the
preemptive scheduling context runs at ring level 2 and code in the
cooperative scheduling context runs at ring level 3.  Ring levels with
higher numbers are less privileged than those with lower numbers.
Ring level 1 is unused.

### IO and Interrupt Privileges

The kernel protection domain cooperative scheduling context needs
access to IO ports, for device initialization.  Other protection
domains may also require such access.  The IO Privilege Level (IOPL)
that is assigned to a protection domain using the relevant bits in the
EFLAGS field could be set according to whether IO port access is
required in that protection domain.  However, this would introduce
additional complexity and overhead in the critical system call and
return dispatchers.  Instead, the IOPL is always set to block IO
access from the cooperative scheduling context.  Port IO instructions
in that context will then generate general protection faults, and the
exception handler decodes and emulates authorized port IO
instructions.

Interrupts are handled at ring level 2, since they do not use any
privileged instructions.  They do cause the interrupt flag to be
cleared as they are delivered.  The interrupt flag can only be
modified by instructions executing at a ring level that is numerically
less than or equal to the IOPL.  Each interrupt handler needs to set
the interrupt flag using the IRET instruction when it returns.
Protection domains that require access to port IO (currently just the
kernel protection domain) are configured with an IOPL of 3 whereas
others are configured with an IOPL of 2.  That is why interrupts are
configured to run at ring level 2.  Interrupts are only enabled in the
application protection domain.

Some interrupt handlers require access to port IO, and all are
permitted such access, since they need it anyway for restoring the
interrupt flag when returning.  IO port access is a very powerful
privilege, since it can be used to remap MMIO regions of PCI devices,
reconfigure PCI devices, etc.  Thus, further restricting access to IO
ports may improve the robustness of the system, but would increase
complexity and space requirements and possibly necessitate additional
context switches, since IO port access is controlled by the combined
settings of IOPL as well as an optional IO bitmap in the TSS.

### Interrupt and Exception Dispatching

Primary implementation sources:
 - cpu/x86/init/common/interrupt.h

Separate stacks are allocated for dispatching interrupts and
exceptions.  However, to save space, the main bodies of some interrupt
and exception handlers are run on the main stack.  A handler may
expect to have access to data from the interrupt or exception stack,
so the interrupt or exception dispatcher copies that data prior to
pivoting to the main stack and executing the handler.

### Protection Domain Control Structures (PDCSes)

Each protection domain is managed by the kernel and privileged
functions using a PDCS.  The PDCS structure is entirely
software-defined.  The initial protection domain plugin does not
support re-entrant protection domains to simplify the implementation
of the plugin by enabling domain-specific information (e.g. system
call return address) to be trivially stored in each PDCS.

### Paging-Based Protection Domains

Primary implementation sources:

 - cpu/x86/mm/paging-prot-domains.c
 - cpu/x86/mm/syscalls-int.c
 - cpu/x86/mm/syscalls-int-asm.S

#### Introduction

Only a single page table is used for all protection domains.  A flat
memory model is used.  Almost all linear-to-physical address mappings
are identity mappings, with the exceptions being the MMIO and metadata
regions.  The X86 port of Contiki currently only supports at most one
MMIO and one metadata range per driver, and the paging-based
protection domain implementation always starts at particular linear
addresses when mapping an MMIO or metadata range.  This may reduce
overhead, due to the way protection domain switches are implemented.

#### System Call and Return Dispatching

The system call dispatcher executes at ring level 0, since it uses the
privileged INVLPG or MOV CR3 instructions to invalidate TLB entries.
The dispatcher modifies page table entries to grant only the
permissions required by the protection domain being activated.  It
then optionally uses the INVLPG instruction to invalidate any TLB
entries for any page table entries that were modified.  If INVLPG is
not used to invalidate specific TLB entries, then CR3 is reloaded to
invalidate the entire TLB (global entries would be excluded, but they
are not used in this implementation).

It is more efficient to always start at a particular linear address
when mapping an MMIO or metadata region, since the page table entries
for that region can be updated to unmap any previous region of that
type, map the new region, and then invalidated to cause the new
settings to take effect.  The alternative using an identity
linear-to-physical address mapping for regions would be to unmap the
previous region by editing one set of page table entries and to then
map the new region by editing a different set of page table entries
and to finally perform invalidations for both sets of page table
entries.  Another drawback of such an identity address mapping is that
additional page tables may need to be allocated to represent the
various MMIO regions, since page tables are indexed by linear address
and MMIO regions are often at high physical addresses.  Note that this
is specific to MMIO regions, since metadata regions are not at
particularly high physical addresses.  Additionally, if different base
linear addresses are used, it is necessary to communicate those to the
system call handler code so that the regions can be accessed.  This
would require care to prevent an adversary from manipulating the
addresses and it may increase complexity.

The overall process of handling a system call can be illustrated at a
high level as follows.  Some minor steps are omitted in the interest
of clarity and brevity.

```
 == BEGIN Client protection domain ==========================================
 -- BEGIN Caller ------------------------------------------------------------
  1.  Call system call stub.
 --
  20. Continue execution...
 -- END Caller --------------------------------------------------------------
 -- BEGIN System call stub --------------------------------------------------
  2.  Already in desired (server) protection domain?
    - No: Issue software interrupt #100 to request system call.
    - Yes: Jump to system call body.
 -- END System call stub ----------------------------------------------------
 == END Client protection domain ============================================
 == BEGIN Ring level 0 ======================================================
 -- BEGIN System call dispatcher---------------------------------------------
  3.  Check that the requested system call is allowed.  Get entrypoint.
  4.  Check that the server protection domain is available (not yet present
      in the protection domain call stack) and then mark it as busy.
  5.  Save the caller return address from the main stack into the client
      PDCS.
  6.  Overwrite the caller return address on the main stack to point to
      system call return stub.
  7.  Push server protection domain onto protection domain call stack.
  8.  Update the interrupt return stack EIP to start of system call body.
  9.  Update and invalidate page table entries to grant only the permissions
      required by the server protection domain.
  10. Update interrupt flag to disable interrupts, since interrupts are only
      enabled in app protection domain, which exports no system calls.
  11. Perform interrupt return (IRET).
 -- END System call dispatcher ----------------------------------------------
 -- BEGIN System call return dispatcher -------------------------------------
  15. Mark protection domain on top of protection domain call stack as
      available.
  16. Retrieve the caller return address from the kernel data structure for
      the client protection domain and use it to overwrite the EIP in the
      interrupt return stack.
  17. Update and invalidate page table entries to grant only the permissions
      required by the client protection domain.
  18. Update interrupt flag to only enable interrupts if returning to app
      protection domain cooperative scheduling context.
  19. Perform interrupt return (IRET).
 -- END System call dispatcher ----------------------------------------------
 == END Ring level 0 ========================================================
 == BEGIN Server protection domain ==========================================
 -- BEGIN System call body --------------------------------------------------
  12. Execute the work for the requested system call.
  13. Return (to system call return stub, unless invoked from server
      protection domain, in which case return is to caller).
 -- END System call body ----------------------------------------------------
 -- BEGIN System call return stub -------------------------------------------
  14. Issue software interrupt #101 to request system call return.
 -- END System call return stub ---------------------------------------------
 == END Server protection domain ============================================
```

The first step in performing a system call is to invoke a system call
stub that actually issues the software interrupt to request a system
call dispatch.  This approach reduces disruption to existing code,
since macros are used to generate separate stubs and corresponding
system call bodies with a single system call signature definition.

#### Memory Layout

The approximate memory layout of the system is depicted below,
starting with the highest physical addresses and proceeding to lower
physical addresses.  Optional permissions are denoted with
parentheses.  See cpu/x86/quarkX1000_paging.ld for details of how this
memory layout is implemented.

```
                                              | Kernel | App    | Other  |
 ...                                          +--------+--------+--------+
 +------------------------------------------+ |        |        |        |
 | Domain X MMIO                            | |        |        | (RW)   |
 +------------------------------------------+ |        |        |        |
 ...                                          |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Domain X DMA-accessible metadata         | |        |        | (RW)   |
 | (section .dma_bss)                       | |        |        |        |
 +------------------------------------------+ |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Domain X metadata (section .meta_bss)    | |        |        | (RW)   |
 +------------------------------------------+ |        |        |        |
 ...                                          |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Kernel-private data                      | | RW     |        |        |
 | (sections .prot_dom_bss, .gdt_bss, etc.) | |        |        |        |
 +------------------------------------------+ |        |        |        |
 +------------------------------------------+ |        |        |        |
 | System call data (section .syscall_bss)  | | RW     | R      | R      |
 +------------------------------------------+ |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Kernel-owned data (section .kern_bss)    | | RW     | R      | R      |
 +------------------------------------------+ |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Page-aligned, Kernel-owned data          | | RW     | R      | R      |
 | (section .page_aligned_kern_bss)         | |        |        |        |
 +------------------------------------------+ |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Common data                              | | RW     | RW     | RW     |
 | (sections .data, .rodata*, .bss, etc.)   | |        |        |        |
 +------------------------------------------+ |        |        |        |
 (not-present guard band page)                |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Exception stack                          | | RW     | RW     | RW     |
 | (section .exc_stack)                     | |        |        |        |
 +------------------------------------------+ |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Interrupt stack                          | | RW     | RW     | RW     |
 | (section .int_stack)                     | |        |        |        |
 +------------------------------------------+ |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Main stack (section .main_stack)         | | RW     | RW     | RW     |
 +------------------------------------------+ |        |        |        |
 (not-present guard band page)                |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Main code (.text)                        | | RX     | RX     | RX     |
 +------------------------------------------+ |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Bootstrap code (section .boot_text)      | |        |        |        |
 +------------------------------------------+ |        |        |        |
 +------------------------------------------+ |        |        |        |
 | Multiboot header                         | |        |        |        |
 +------------------------------------------+ |        |        |        |
 ...
```

The only protection domain that is permitted to access kernel-owned
data is the kernel protection domain.  Some devices can also be
instructed to perform DMA to kernel-owned data, although that is an
incorrect configuration.

Paging only differentiates between memory accesses from ring 3 (user
level) and those from rings 0-2 (supervisor level).  To avoid granting
code running in the preemptive scheduling context supervisory write
access to kernel data structures (including the page tables), those
structures are marked read-only (except when the kernel protection
domain is active) and the Write Protect (WP) bit in Control Register 0
(CR0) is cleared only when it is necessary to update a write-protected
structure.  Only ring 0 is allowed to modify CR0.

Optional metadata for each protection domain is intended to only be
accessible from the associated protection domain and devices.

Read accesses to executable code have not been observed to be needed
in at least a limited set of tests, but they are permitted, since
paging does not support an execute-only permission setting.  On the
other hand, the Execute-Disable feature is used to prevent execution
of non-code memory regions.  All non-startup code is mapped in all
protection domains.  Limiting the code that is executable within each
protection domain to just the code that is actually needed within that
protection domain could improve the robustness of the system, but it
is challenging to determine all code that may be needed in a given
protection domain (e.g. all needed library routines).

Stack accesses to non-stack memory are not needed, but they are
permitted.  However, one page of unmapped linear address space is
placed above and below the stacks to detect erroneous stack accesses
to those linear address regions, which are the types of accesses most
likely to occur during a stack overflow or underflow condition.  The
main stack is placed just below the interrupt stack, which is just
below the exception stack.  Stack overflows are more common than stack
underflows, which motivates arranging the stacks such that an overflow
from a less-critical stack will not affect a more-critical stack.
Furthermore, the main stack is the most likely to overflow, since the
code that uses it is typically the most voluminous and difficult to
characterize.  That provides additional motivation for positioning it
such that an overflow results in an immediate page fault.  An
alternative design placing each stack on a separate group of
contiguous pages may improve the robustness of the system by
permitting the insertion of unmapped guard pages around them to
generate page faults in the event an overflow or underflow occurs on
any stack.  However, that would consume additional memory.

Data in the .rodata sections is marked read/write, even though it may
be possible to improve the robustness of the system by marking that
data as read-only.  Doing so would introduce additional complexity
into the system.

### Pointer Validation

Primary implementation sources:
 - cpu/x86/mm/syscalls.h

At the beginning of each system call routine, it is necessary to check
that any untrusted pointer that could have been influenced by a caller
(i.e. a stack parameter or global variable) refers to a location above
the return address and to halt otherwise.  This is to prevent a
protection domain from calling a different protection domain and
passing a pointer that references a location in the callee's stack
other than its parameters to influence the execution of the callee in
an unintended manner.  For example, if an incoming pointer referenced
the return address, it could potentially redirect execution with the
privileges of the callee protection domain.

It is also necessary to check that the pointer is either within the
stack region or the shared data region (or a guard band region, since
that will generate a fault) to prevent redirection of data accesses to
MMIO or metadata regions.

The pointer is both validated and copied to a new storage location,
which must be within the callee's local stack region (excluding the
parameter region).  This is to mitigate scenarios such as two pointers
being validated and an adversary later inducing a write through one of
the pointers to the other pointer to corrupt the latter pointer before
it is used.

Any pointer whose value is fixed at link or load time does not need to
be validated prior to use, since no adversary within the defined
threat model is able to influence the link or load process.

### DMA Restrictions

Primary implementation sources:
 - cpu/x86/drivers/quarkX1000/imr.c
 - cpu/x86/drivers/quarkX1000/imr-conf.c

The CPU is not the only agent with the ability to issue requests to
the interconnect within the SoC.  For example, SoC peripherals such as
the Ethernet driver use DMA to efficiently access memory buffers.
This could introduce a risk that DMA could be used to bypass the
memory protections enforced on the CPU by segmentation or paging.  For
example, a device driver could instruct a device to access a memory
region to which the kernel has not granted the driver's protection
domain permission to access.

The Isolated Memory Region (IMR) feature is configured to restrict the
memory that can be accessed by system agents other than the CPU [3].
It only allows those system agents to access portions of the Contiki
memory space that are specifically intended to be used with DMA.  The
source code for each protection domain specifies that its optional
metadata region needs to be accessible from other system agents
besides the CPU by using ATTR_BSS_DMA instead of ATTR_BSS_META when
allocating storage for the metadata.

Extending the Framework
-----------------------

### Adding a New Protection Domain

The following steps are required.  See the existing device drivers for
examples of various types of protection domains and how they are
initialized.

 - Allocate storage for the PDCS and the corresponding
   client-accessible data structure using the PROT_DOMAINS_ALLOC
   macro.
 - Apply the ATTR_BSS_META attribute to the metadata structure, if
   applicable.  Apply the ATTR_BSS_DMA attribute instead if the
   metadata structure needs to be DMA-accessible.  Pad the metadata
   structure to completely fill an integer multiple of the minimum
   page size, 4096, when paging-based protection domains are in use.
   See the definition of quarkX1000_eth_meta_t for an example.
 - Perform the following steps during boot stage 2:
   - Initialize the protection domain ID in the client-accessible data
     structure using the PROT_DOMAINS_INIT_ID macro.
   - Register the domain.  See prot-domains.c:prot_domains_init for an
     example of registering a non-driver protection domain.  See
     cpu/x86/drivers/quarkX1000/eth.c:quarkX1000_eth_init for an
     example of registering a PCI driver protection domain with an
     MMIO region and a metadata region.

### Adding a New System Call

The following steps are required:

 - Define the system call procedure using the SYSCALLS_DEFINE or
   SYSCALLS_DEFINE_SINGLETON macro.  See
   cpu/x86/drivers/legacy_pc/uart-16x50.c:uart_16x50_tx for an example
   of a non-singleton system call.  See
   cpu/x86/drivers/quarkX1000/eth.c:quarkX1000_eth_send for an example
   of a singleton system call.  A singleton system call is one for
   which at most one server protection domain will be associated with
   it.
 - During boot phase 2, associate the system call with one or more
   server protection domains using the SYSCALLS_AUTHZ macro.

Usage
-----

To enable protection domain support, add
"X86_CONF_PROT_DOMAINS=paging" to the command line.

The paging option accepts a sub-option to determine whether the TLB is
fully- or selectively-invalidated during protection domain switches.
By default, full invalidation is selected.  Set the
X86_CONF_USE_INVLPG variable to 1 to override the default.

References
----------

[1] J. H. Saltzer, "Protection and the Control of Information Sharing
    in Multics," Commun. ACM, vol. 17, no. 7, pp. 388-402, Jul. 1974.

[2] https://github.com/contiki-os/contiki/wiki/Processes

[3] "Intel(R) Quark(TM) SoC X1000 Secure Boot Programmer's Reference
    Manual,"
    http://www.intel.com/support/processors/quark/sb/CS-035228.htm
