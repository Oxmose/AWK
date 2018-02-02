
# AWK
AWK is a simple hobby kernel created for educational purposes.
AWK stands for "Almost Working Kernel".
Even if some bootstrap code has been written to support ser mode, I decided to
make AWK run in kernel mode only.

The kernel is based on the x86 acrhitecture and currently supports:

* GDT
* IDT
* Paging
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

## Some little things about the kernel

 * The kernel only executes in kernel mode
 * The kernel is locked to take only 32MB (for reasons) in memory
 * The kernel is not a higher half kernel, each process will have the kernel
 mapped from 0 to 32MB. Then the process code will be mapped.
 * The last points are of course security issue, but are done on purposes

## Screenshots
![Boot](https://raw.githubusercontent.com/Oxmose/AWK/master/screenshots/1.png)
![Test GUI](https://raw.githubusercontent.com/Oxmose/AWK/master/screenshots/2.png)
![Test GUI](https://raw.githubusercontent.com/Oxmose/AWK/master/screenshots/3.png)
