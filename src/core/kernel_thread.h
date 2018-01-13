/*******************************************************************************
 *
 * File: kernel_thread.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.5
 *
 * Kernel thread structure definitions
 ******************************************************************************/

#ifndef __KERNEL_THREAD_H_
#define __KERNEL_THREAD_H_

#include "../lib/stdint.h"       /* Generic int types */
#include "../cpu/cpu_settings.h" /* KERNEL_CS KERNEL_DS */
#include "kernel_list.h"

/* Forward declaration */
struct thread_queue;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* Thread settings */
#define THREAD_MAX_NAME_LENGTH  32
#define THREAD_STACK_SIZE       2048 /* 64 KB */

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

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* Thread states */
typedef enum THREAD_STATE
{
    RUNNING,
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

/* Kernel thread structure */
typedef struct kernel_thread
{
    /* General thread's settings */
    int32_t          pid;
    int32_t          ppid;
    char             name[THREAD_MAX_NAME_LENGTH];
    uint32_t         priority;

    /* Thread routine, arguments and return value */
    void*            args;
    void*            (*function)(void*);
    void*            ret_val;

    THREAD_STATE_E   state;

    /* Thread specific registers */
    uint32_t         esp;
    uint32_t         ebp;
    uint32_t         eip;

    /* Thread kernel stack */
    uint32_t         kernel_stack[THREAD_STACK_SIZE];

    /* Wake up time for the sleeping thread */
    uint32_t         wakeup_time;

    /* Thread block management */
    BLOCK_TYPE_E     block_type;
    uint32_t         io_req_time;

    /* Thread pointer that is joining the thread */
    struct kernel_thread* joining_thread;

    /* Thread's children */
    struct kernel_list_t children;

    /* Statistics (scheduler) */
    uint32_t         last_sched;
    uint32_t         full_consume;

    /* Statistics */
    uint32_t start_time;
    uint32_t end_time;
    uint32_t exec_time;
} kernel_thread_t;

typedef kernel_thread_t* thread_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#endif /* __THREAD_H_ */
