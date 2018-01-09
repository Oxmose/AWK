/*******************************************************************************
 *
 * File: cpu.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * CPU management functions
 ******************************************************************************/

#ifndef __CPU_H_
#define __CPU_H_

#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
 #define MAX_CPU_COUNT 16

#define CPU_FLAG_CPUID_CAPABLE 0x00200000

/* %ecx */
#define BIT_SSE3    (1 << 0)
#define BIT_PCLMUL  (1 << 1)
#define BIT_LZCNT   (1 << 5)
#define BIT_SSSE3   (1 << 9)
#define BIT_FMA     (1 << 12)
#define BIT_CMPXCHG16B  (1 << 13)
#define BIT_SSE4_1  (1 << 19)
#define BIT_SSE4_2  (1 << 20)
#define BIT_MOVBE   (1 << 22)
#define BIT_POPCNT  (1 << 23)
#define BIT_AES     (1 << 25)
#define BIT_XSAVE   (1 << 26)
#define BIT_OSXSAVE (1 << 27)
#define BIT_AVX     (1 << 28)
#define BIT_F16C    (1 << 29)
#define BIT_RDRND   (1 << 30)

/* %edx */
#define BIT_CMPXCHG8B   (1 << 8)
#define BIT_CMOV    (1 << 15)
#define BIT_MMX     (1 << 23)
#define BIT_FXSAVE  (1 << 24)
#define BIT_SSE     (1 << 25)
#define BIT_SSE2    (1 << 26)

/* Extended Features (%eax == 0x80000001) */
/* %ecx */
#define BIT_LAHF_LM (1 << 0)
#define BIT_ABM     (1 << 5)
#define BIT_SSE4a   (1 << 6)
#define BIT_PRFCHW  (1 << 8)
#define BIT_XOP         (1 << 11)
#define BIT_LWP     (1 << 15)
#define BIT_FMA4        (1 << 16)
#define BIT_TBM         (1 << 21)
#define BIT_MWAITX      (1 << 29)

/* %edx */
#define BIT_MMXEXT  (1 << 22)
#define BIT_LM      (1 << 29)
#define BIT_3DNOWP  (1 << 30)
#define BIT_3DNOW   (1u << 31)

/* %ebx  */
#define BIT_CLZERO  (1 << 0)

/* Extended Features (%eax == 7) */
/* %ebx */
#define BIT_FSGSBASE    (1 << 0)
#define BIT_SGX (1 << 2)
#define BIT_BMI (1 << 3)
#define BIT_HLE (1 << 4)
#define BIT_AVX2    (1 << 5)
#define BIT_BMI2    (1 << 8)
#define BIT_RTM (1 << 11)
#define BIT_MPX (1 << 14)
#define BIT_AVX512F (1 << 16)
#define BIT_AVX512DQ    (1 << 17)
#define BIT_RDSEED  (1 << 18)
#define BIT_ADX (1 << 19)
#define BIT_AVX512IFMA  (1 << 21)
#define BIT_CLFLUSHOPT  (1 << 23)
#define BIT_CLWB    (1 << 24)
#define BIT_AVX512PF    (1 << 26)
#define BIT_AVX512ER    (1 << 27)
#define BIT_AVX512CD    (1 << 28)
#define BIT_SHA     (1 << 29)
#define BIT_AVX512BW    (1 << 30)
#define BIT_AVX512VL    (1u << 31)

/* %ecx */
#define BIT_PREFETCHWT1   (1 << 0)
#define BIT_AVX512VBMI  (1 << 1)
#define BIT_PKU (1 << 3)
#define BIT_OSPKE   (1 << 4)
#define BIT_AVX512VBMI2 (1 << 6)
#define BIT_SHSTK   (1 << 7)
#define BIT_GFNI    (1 << 8)
#define BIT_VAES    (1 << 9)
#define BIT_AVX512VNNI  (1 << 11)
#define BIT_AVX512VPOPCNTDQ (1 << 14)
#define BIT_RDPID   (1 << 22)

/* %edx */
#define BIT_AVX5124VNNIW (1 << 2)
#define BIT_AVX5124FMAPS (1 << 3)
#define BIT_IBT (1 << 20)

/* XFEATURE_ENABLED_MASK register bits (%eax == 13, %ecx == 0) */
#define BIT_BNDREGS     (1 << 3)
#define BIT_BNDCSR      (1 << 4)

/* Extended State Enumeration Sub-leaf (%eax == 13, %ecx == 1) */
#define BIT_XSAVEOPT    (1 << 0)
#define BIT_XSAVEC  (1 << 1)
#define BIT_XSAVES  (1 << 3)

/* Signatures for different CPU implementations as returned in uses
   of cpuid with level 0.  */
#define SIG_AMD_EBX   0x68747541
#define SIG_AMD_ECX   0x444d4163
#define SIG_AMD_EDX   0x69746e65

#define SIG_CENTAUR_EBX   0x746e6543
#define SIG_CENTAUR_ECX   0x736c7561
#define SIG_CENTAUR_EDX   0x48727561

#define SIG_CYRIX_EBX 0x69727943
#define SIG_CYRIX_ECX 0x64616574
#define SIG_CYRIX_EDX 0x736e4978

#define SIG_INTEL_EBX 0x756e6547
#define SIG_INTEL_ECX 0x6c65746e
#define SIG_INTEL_EDX 0x49656e69

#define SIG_TM1_EBX   0x6e617254
#define SIG_TM1_ECX   0x55504361
#define SIG_TM1_EDX   0x74656d73

#define SIG_TM2_EBX   0x756e6547
#define SIG_TM2_ECX   0x3638784d
#define SIG_TM2_EDX   0x54656e69

#define SIG_NSC_EBX   0x646f6547
#define SIG_NSC_ECX   0x43534e20
#define SIG_NSC_EDX   0x79622065

#define SIG_NEXGEN_EBX    0x4778654e
#define SIG_NEXGEN_ECX    0x6e657669
#define SIG_NEXGEN_EDX    0x72446e65

#define SIG_RISE_EBX  0x65736952
#define SIG_RISE_ECX  0x65736952
#define SIG_RISE_EDX  0x65736952

#define SIG_SIS_EBX   0x20536953
#define SIG_SIS_ECX   0x20536953
#define SIG_SIS_EDX   0x20536953

#define SIG_UMC_EBX   0x20434d55
#define SIG_UMC_ECX   0x20434d55
#define SIG_UMC_EDX   0x20434d55

#define SIG_VIA_EBX   0x20414956
#define SIG_VIA_ECX   0x20414956
#define SIG_VIA_EDX   0x20414956

#define SIG_VORTEX_EBX    0x74726f56
#define SIG_VORTEX_ECX    0x436f5320
#define SIG_VORTEX_EDX    0x36387865

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef struct cpu_info
{
    int32_t cpu_flags;
} cpu_info_t;

typedef enum cpuid_requests {
  CPUID_GETVENDORSTRING,
  CPUID_GETFEATURES,
  CPUID_GETTLB,
  CPUID_GETSERIAL,

  CPUID_INTELEXTENDED=0x80000000,
  CPUID_INTELFEATURES,
  CPUID_INTELBRANDSTRING,
  CPUID_INTELBRANDSTRINGMORE,
  CPUID_INTELBRANDSTRINGEND,
} cpuid_requests;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Fill the structure in parameters with the CPU information
 *
 * @returns The succes state or the error code.
 * @param info The pointer to the structure to receive the data.
 */
OS_RETURN_E get_cpu_info(cpu_info_t* info);

/* Tell is the CPUID intruction is available on the CPU.
 *
 * @returns 1 if the cpuid instruction is available, 0 otherwise.
 */
int8_t cpuid_capable(void);

/* Return highest supported input value for cpuid instruction.  ext can
 * be either 0x0 or 0x80000000 to return highest supported value for
 * basic or extended cpuid information.  Function returns 0 if cpuid
 * is not supported or whatever cpuid returns in eax register.  If sig
 * pointer is non-null, then first four bytes of the SIG
 * (as found in ebx register) are returned in location pointed by sig.
 *
 * @param ext The opperation code for the CPUID instruction.
 * @returns The highest supported input value for cpuid instruction.
 */
__inline__ static uint32_t get_cpuid_max (uint32_t ext)
{
    uint32_t regs[4];
    if(cpuid_capable() == 0)
    {
        return 0;
    }

    /* Host supports cpuid.  Return highest supported cpuid input value.  */
    __asm__ __volatile__("cpuid":"=a"(*regs),"=b"(*(regs+1)),
                         "=c"(*(regs+2)),"=d"(*(regs+3)):"a"(ext));

    return regs[0];
}

/* Return cpuid data for requested cpuid leaf, as found in returned
 *  eax, ebx, ecx and edx registers.  The function checks if cpuid is
 * supported and returns 1 for valid cpuid information or 0 for
 * unsupported cpuid leaf.  All pointers are required to be non-null.
 *
 * @param code The opperation code for the CPUID instruction.
 * @param regs The register used to store the CPUID instruction return.
 * @returns 1 in case of succes, 0 otherwise.
 */

__inline__ static int32_t cpuid (uint32_t code,
                                 uint32_t regs[4])
{
    if(cpuid_capable() == 0)
    {
        return 0;
    }
    uint32_t ext = code & 0x80000000;
    uint32_t maxlevel = get_cpuid_max (ext);

    if (maxlevel == 0 || maxlevel < code)
    {
        return 0;
    }
    __asm__ __volatile__("cpuid":"=a"(*regs),"=b"(*(regs+1)),
                         "=c"(*(regs+2)),"=d"(*(regs+3)):"a"(code));
    return 1;
}

/* Clear interupt bit which results in disabling interupts */
__inline__ static void cli(void)
{
    __asm__ __volatile__("cli":::"memory");
}

/* Sets interupt bit which results in enabling interupts */
__inline__ static void sti(void)
{
    __asm__ __volatile__("sti":::"memory");
}

/* Halts the CPU for lower energy consuption */
__inline__ static void hlt(void)
{
    __asm__ __volatile__ ("hlt":::"memory");
}


/* Save CPU flags
 *
 * @returns The current flags that were saved.
 */
__inline__ static uint32_t save_flags(void)
{
    uint32_t flags;

    __asm__ __volatile__(
        "pushfl\n"
        "\tpopl    %0\n"
        : "=g" (flags)
        :
        : "memory"
    );

    return flags;
}

/* Restore CPU flags
 *
 * @param flags The flags to be restored.
 */
__inline__ static void restore_flags(uint32_t flags)
{
    __asm__ __volatile__(
        "pushl    %0\n"
        "\tpopfl\n"
        :
        : "g" (flags)
        : "memory"
    );
}

/* Write byte on port.
 *
 * @param value The value to send to the port.
 * @param port The port to which the value has to be written.
 */
__inline__ static void outb(uint8_t value, uint16_t port)
{
    __asm__ __volatile__("outb %0, %1" : : "a" (value), "Nd" (port));
}

/* Write word on port.
 *
 * @param value The value to send to the port.
 * @param port The port to which the value has to be written.
 */
__inline__ static void outw(uint16_t value, uint16_t port)
{
    __asm__ __volatile__("outw %0, %1" : : "a" (value), "Nd" (port));
}

/* Write long on port.
 *
 * @param value The value to send to the port.
 * @param port The port to which the value has to be written.
 */
__inline__ static void outl(uint32_t value, uint16_t port)
{
    __asm__ __volatile__("outl %0, %1" : : "a" (value), "Nd" (port));
}

/* Read byte on port.
 *
 * @returns The value read fron the port.
 * @param port The port to which the value has to be read.
 */
__inline__ static uint8_t inb(uint16_t port)
{
    uint8_t rega;
    __asm__ __volatile__("inb %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/* Read word on port.
 *
 * @returns The value read fron the port.
 * @param port The port to which the value has to be read.
 */
__inline__ static uint16_t inw(uint16_t port)
{
    uint16_t rega;
    __asm__ __volatile__("inw %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/* Read long on port.
 *
 * @returns The value read fron the port.
 * @param port The port to which the value has to be read.
 */
__inline__ static uint32_t inl(uint16_t port)
{
    uint32_t rega;
    __asm__ __volatile__("inl %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/* Compare and swap word atomicaly.
 *
 * @returns The value of the lock
 * @param p_val The pointer to the lock.
 * @param oldval The old value to swap.
 * @param newval The new value to be swapped.
 */
__inline__ static uint32_t cpu_compare_and_swap(volatile uint32_t* p_val,
        int oldval, int newval)
{
    uint8_t prev;
    __asm__ __volatile__ (
            "lock cmpxchg %1, %2\n"
            "setne %%al"
                : "=a" (prev)
                : "r" (newval), "m" (*p_val), "0" (oldval)
                : "memory");
    return prev;
}

/* Test and set atomic operation.
 *
 * @param lock The spinlock to apply the test on.
 */
__inline__ static int cpu_test_and_set(volatile uint32_t* lock)
{
        return cpu_compare_and_swap(lock, 0, 1);
}

/* Read the current value of the CPU's time-stamp counter and store into
 * EDX:EAX. The time-stamp counter contains the amount of clock ticks that have
 * elapsed since the last CPU reset. The value is stored in a 64-bit MSR and is
 * incremented after each clock cycle.
 *
 * @return The CPU's time stampe
 */
__inline__ static uint64_t rdtsc()
{
    uint64_t ret;
    __asm__ __volatile__ ( "rdtsc" : "=A"(ret) );
    return ret;
}

/*******************************************************************************
 * Memory mapped IOs, avoid compilers to reorganize memory access
 *
 * So instead of doing : *addr = value, do
 * mapped_io_write(addr, value)
 ******************************************************************************/

__inline__ static void mapped_io_write_8(void* volatile addr,
                                         const uint8_t value)
{
    *(volatile uint8_t*)(addr) = value;
}

__inline__ static void mapped_io_write_16(void* volatile addr,
                                          const uint16_t value)
{
    *(volatile uint16_t*)(addr) = value;
}

__inline__ static void mapped_io_write_32(void* volatile addr,
                                          const uint32_t value)
{
    *(volatile uint32_t*)(addr) = value;
}

__inline__ static void mapped_io_write_64(void* volatile addr,
                                          const uint64_t value)
{
    *(volatile uint64_t*)(addr) = value;
}

__inline__ static uint8_t mapped_io_read_8(const volatile void* addr)
{
    return *(volatile uint8_t*)(addr);
}

__inline__ static uint16_t mapped_io_read_16(const volatile void* addr)
{
    return *(volatile uint16_t*)(addr);
}

__inline__ static uint32_t mapped_io_read_32(const volatile void* addr)
{
    return *(volatile uint32_t*)(addr);
}

__inline__ static uint64_t mapped_io_read_64(const volatile void* addr)
{
    return *(volatile uint64_t*)(addr);
}

__inline__ static void mapped_io_read_sized(const volatile void* addr,
                                            void* value,
                                            uint32_t size)
{
    volatile uint8_t* base = (volatile uint8_t*)addr;
    uint8_t* dest = (uint8_t*)value;

    /* Tranfert memory until size limit is reached */
    while (size > 0)
    {
        *dest = *base;
        ++base;
        ++dest;
        --size;
    }
}

#endif /* __CPU_H_ */
