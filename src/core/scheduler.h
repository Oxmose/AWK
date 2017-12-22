/*******************************************************************************
 *
 * File: scheduler.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 1.0
 *
 * Kernel scheduler
 * Thread creation and management functions are located in this file.
 *
 ******************************************************************************/

#ifndef __SCHEDULER_H_
#define __SCHEDULER_H_

#include "../lib/stddef.h"  /* OS_RETURN_E */
#include "interrupts.h"     /* cpu_state_t, stack_state_t */
#include "kernel_thread.h"  /* thread_t */
#include "kernel_queue.h"   /* kernel_queue_t */

/****************************
 * CONSTANTS
 ***************************/
#define SCHEDULE_PERIOD         1
#define SCHEDULER_SW_INT_LINE   0x30

#define KERNEL_LOWEST_PRIORITY  64
#define KERNEL_HIGHEST_PRIORITY 0
#define IDLE_THREAD_PRIORITY    KERNEL_LOWEST_PRIORITY

/****************************
 * STRUCTURES 
 ***************************/
/* System possible sates */
typedef enum SYSTEM_STATE
{
    RUNNING,
    HALTED
} SYSTEM_STATE_E;

/* Thread information struct */
typedef struct thread_info
{
    int32_t          pid;
    int32_t          ppid;
    char             name[THREAD_MAX_NAME_LENGTH];

    uint32_t         priority;

    THREAD_STATE_E   state;

    thread_queue_t *children;

    uint32_t start_time;
    uint32_t end_time;
    uint32_t exec_time;
} thread_info_t;

/****************************
 * FUNCTIONS 
 ***************************/

/* Idle thread routine */
void* idle_sys(void* args);

/* Return system state 
 * @returns The system state.
 */
SYSTEM_STATE_E get_system_state(void);

/* Init the scheduler service.
 * This function SHOULD BE THE LAST CALLED AT THE INIT OF THE KERNEL
 * !!!! IT SHOULD NEVER RETURN !!!!
 * @return If the function returns, it means the init failed, the error code is
 * set accordingly.
 */
OS_RETURN_E init_scheduler(void);

/* Schedule current thread, raise an interrupt.
 */
void schedule(void);

/* Schedule current thread in interrupt
 */
void schedule_int(cpu_state_t *cpu_state, uint32_t int_id, 
                  stack_state_t *stack_state);

/* Put the calling thread to sleep.
 * @param time_ms The number of milliseconds to wait.
 * @returns The success or error code.
 */
OS_RETURN_E sleep(const unsigned int time_ms);

/* Returns the number of existing threads
 * @returns The number of alive thread (all but dead).
 */
uint32_t get_thread_count(void);

/* Returns the PID of the current executing thread.
 * @returns The PID of the current executing thread.
 */
int32_t get_pid(void);

/* Returns the PPID of the current executing thread.
 * @returns The PPID of the current executing thread.
 */
int32_t get_ppid(void);

/* Returns the name of the current executing thread.
 * @returns The name of the current executing thread.
 */
char *get_current_thread_name(void);

/* Returns the priority of the current executing thread.
 * @returns The priority of the current executing thread.
 */
uint32_t get_priority(void);

/* Returns the executing thread structuer pointer.
 * @returns The executing thread structuer pointer.
 */
kernel_thread_t *get_active_thread(void);

/* Create a new thread in the thread table.
 *
 * @param thread The pointer to the thread structure.
 * @param function The thread routine to be executed.
 * @param priority The desired priority of the thread.
 * @param name The name of the thread.
 * @param args The arguments to be used by the thread.
 * @returns OS_NO_ERR on success, error code otherwise.
 */
OS_RETURN_E create_thread(thread_t *thread, 
	                      void *(*function)(void*), 
	                      const uint32_t priority, 
	                      const char *name, 
	                      void *args);

/* Remove a thread from the threads table. Wait for the thread to finish.
 *
 * @param thread The pointer to the thread structure.
 * @returns OS_NO_ERR on success, error code otherwise.
 */
OS_RETURN_E wait_thread(thread_t thread, void **ret_val);

/* Add a thread to the active threads table, the thread might be contained
 * in an other structure such as a mutex
 *
 * @param thread The thread to unlock.
 * @param block_type The type of block (mutex, sem, ...)
 * @param do_schedule Set to 1 if you want an immediat schedule.
 * @returns OS_NO_ERR on success, error code otherwise.
 */
OS_RETURN_E unlock_thread(const thread_t thread,
                          const BLOCK_TYPE_E block_type,
                          const uint8_t do_schedule);
/* Remove the active thread from the active threads table, the thread might be 
 * contained in an other structure such as a mutex
 *
 * @param block_type The type of block (mutex, sem, ...)
 * @returns OS_NO_ERR on success, error code otherwise.
 */
OS_RETURN_E lock_thread(const BLOCK_TYPE_E block_type);

/* Lock the current thread waiting for an IO.
 * @param block_type The type of IO that locks the thread.
 * @returns OS_NO_ERR on success, error code otherwise.
 */
OS_RETURN_E lock_io(const BLOCK_TYPE_E block_type);

/* Unlock a thread that was waiting for an IO.
 * Schedule if necessary.
 * @param block_type The type of IO that locks the thread.
 * @returns OS_NO_ERR on success, error code otherwise.
 */
OS_RETURN_E unlock_io(const BLOCK_TYPE_E block_type);

OS_RETURN_E get_threads_info(thread_info_t *threads, int32_t *size);

#endif /* __SCHEDULER_H_ */