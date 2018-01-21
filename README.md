
# AWK
AWK is a simple hobby kernel created for educational purposes.
AWK stands for "Almost Working Kernel".
The kernel is based on the x86 acrhitecture and currently supports:

* GDT
* IDT
* CPUID
* VGA Text mode (80x25) 
* VESA
* Real Mode <-> Protected Mode switch
* Serial
* PIC
* IO-APIC
* Local APIC
* APIC timer (used by scheduler)
* PIT
* RTC
* Keyboard
* Mouse
* ATA PIO
* Multi threading (dynamic priority based scheduler)
* Synchronization (spinlock, mutex, semaphore)
* Communication (mailbox, queue)
* Dynamic allocation
* Printf
