/*******************************************************************************
 *
 * File: kernel_thread.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * Kernel thread structure definitions 
 ******************************************************************************/

#ifndef __KERNEL_THREAD_H_
#define __KERNEL_THREAD_H_

#include "../lib/stdint.h"       /* Generic int types */
#include "../cpu/cpu_settings.h" /* KERNEL_CS KERNEL_DS */

/* Thread settings */
#define THREAD_MAX_NAME_LENGTH  64
#define THREAD_STACK_SIZE       4096 /* 16 Kb */
#define THREAD_MEMORY_SIZE      sizeof(scheduler_thread_t);
#define THREAD_MAX_STATE_LENGTH 10

/* Thread init values */
#define THREAD_INIT_EFLAGS 0x202 // INT | PARITY
#define THREAD_INIT_EAX    0
#define THREAD_INIT_EBX    0
#define THREAD_INIT_ECX    0
#define THREAD_INIT_EDX    0
#define THREAD_INIT_ESI    0
#define THREAD_INIT_EDI    0
#define THREAD_INIT_CS     KERNEL_CS
#define THREAD_INIT_SS     KERNEL_DS
#define THREAD_INIT_DS     KERNEL_DS
#define THREAD_INIT_ES     KERNEL_DS
#define THREAD_INIT_FS     KERNEL_DS
#define THREAD_INIT_GS     KERNEL_DS

/* Thread states */
typedef enum THREAD_STATE
{
    ELECTED,
    READY,
    SLEEPING,
    DEAD,
    ZOMBIE,
    JOINING,
    BLOCKED
} THREAD_STATE_E;

/* Block types */
typedef enum BLOCK_TYPE
{
    SEM,
    MUTEX,
    QUEUE,
    IO_KEYBOARD
} BLOCK_TYPE_E;

typedef struct kernel_thread
{
    int32_t          pid;
    int32_t          ppid;
    char             name[THREAD_MAX_NAME_LENGTH];

    uint32_t         priority;

    void             *args;
    void             *(*function)(void*);
    void             *ret_val;

    THREAD_STATE_E   state;

    uint32_t         esp;
    uint32_t         ebp;
    uint32_t         eip;

    uint32_t         stack[THREAD_STACK_SIZE];

    uint32_t         wakeup_time;

    BLOCK_TYPE_E     block_type;

    struct kernel_thread *joining_thread;
} kernel_thread_t;

typedef kernel_thread_t* thread_t;

#endif /* __THREAD_H_ */
