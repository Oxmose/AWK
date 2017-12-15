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
 * See: cpu.c
 *
 * CPU management functions
 ******************************************************************************/

#ifndef __CPU_H_
#define __CPU_H_

/* Generic int types */
#include "../lib/stdint.h"

/******************************
 * CONSTANTS
 *****************************/
#define MAX_CPU_COUNT 2048

/******************************
 * FUNCTIONS
 *****************************/
#define __MOVSB__(dest, src, n)                \
    __asm__ __volatile__(                      \
        "rep\n"                                \
        "\tmovsb"                              \
        : "=S" (src), "=D" (dest), "=c" (n)    \
        : "0" (src), "1" (dest), "2" (n)       \
        : "memory", "cc")

#define __MOVSL__(dest, src, n)                \
    __asm__ __volatile__(                      \
        "rep\n"                                \
        "\tmovsl"                              \
        : "=S" (src), "=D" (dest), "=c" (n)    \
        : "0" (src), "1" (dest), "2" (n)       \
        : "memory", "cc")

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


/* Save CPU flags */
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

/* Restore CPU flags */
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
__inline__ static uint32_t cpu_compare_and_swap(volatile uint32_t *p_val, 
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
__inline__ static int cpu_test_and_set(volatile uint32_t *lock)
{
    return cpu_compare_and_swap(lock, 0, 1); 
}

#endif /* __CPU_H_ */