/*******************************************************************************
 *
 * File: scheduler.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 2.0
 *
 * Kernel scheduler
 * Thread creation and management functions are located in this file.
 ******************************************************************************/

#ifndef __SCHEDULER_H_
#define __SCHEDULER_H_

#include "../lib/stddef.h"  /* OS_RETURN_E */
#include "../lib/stdint.h"  /* Generic int types */
#include "kernel_thread.h"  /* thread_t */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
#define SCHEDULE_PERIOD         1

#define KERNEL_LOWEST_PRIORITY  64
#define KERNEL_HIGHEST_PRIORITY 0
#define IDLE_THREAD_PRIORITY    KERNEL_LOWEST_PRIORITY

#define SCHEDULE_DYN_PRIORITY   1

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* System possible sates */
typedef enum SYSTEM_STATE
{
    ALIVE,
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

    uint32_t start_time;
    uint32_t end_time;
    uint32_t exec_time;
} thread_info_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Tells the system state: HALTED or RUNNING.
 *
 * @returns The system state (RUNNING or HALTED)
 */
SYSTEM_STATE_E get_system_state(void);

/* Init the scheduler service.
 * This function SHOULD BE THE LAST CALLED AT THE INIT OF THE KERNEL
 * !!!! IT SHOULD NEVER RETURN !!!!
 *
 * @return If the function returns, it means the init failed, the error code is
 * set accordingly.
 */
OS_RETURN_E init_scheduler(void);

/* Call the scheduler, use interrupt since we should never call the
 * scheduler outside of an interrupt context
 */
void schedule(void);

/* Put the calling thread to sleep.
 *
 * @param time_ms The number of milliseconds to wait.
 * @returns The success or error code.
 */
OS_RETURN_E sleep(const uint32_t time_ms);

/* Returns the number of existing threads
 *
 * @returns The number of alive thread (all but dead).
 */
uint32_t get_thread_count(void);

/* Returns the PID of the current executing thread.
 *
 * @returns The PID of the current executing thread.
 */
int32_t get_pid(void);

/* Returns the PPID of the current executing thread.
 *
 * @returns The PPID of the current executing thread.
 */
int32_t get_ppid(void);

/* Returns the priority of the current executing thread.
 *
 * @returns The priority of the current executing thread.
 */
uint32_t get_priority(void);

/* Create a new thread in the thread table.
 *
 * @param thread The pointer to the thread structure.
 * @param function The thread routine to be executed.
 * @param priority The desired priority of the thread.
 * @param name The name of the thread.
 * @param args The arguments to be used by the thread.
 * @returns OS_NO_ERR on success, error code otherwise.
 */
OS_RETURN_E create_thread(thread_t* thread,
                          void* (*function)(void*),
                          const uint32_t priority,
                          const char* name,
                          void* args);

/* Remove a thread from the threads table. Wait for the thread to finish.
 *
 * @param thread The pointer to the thread structure.
 * @returns OS_NO_ERR on success, error code otherwise.
 */
OS_RETURN_E wait_thread(thread_t thread, void** ret_val);

/* Add a thread to the active threads table, the thread might be contained
 * in an other structure such as a mutex
 *
 * @param node The node containing the thread to unlock.
 * @param block_type The type of block (mutex, sem, ...)
 * @param do_schedule Set to 1 if you want an immediat schedule.
 * @returns OS_NO_ERR on success, error code otherwise.
 */
OS_RETURN_E unlock_thread(kernel_list_node_t* node,
                          const BLOCK_TYPE_E block_type,
                          const uint8_t do_schedule);

/* Remove the active thread from the active threads table, the thread might be
 * contained in an other structure such as a mutex.
 *
 * @param block_type The type of block (mutex, sem, ...)
 * @returns The node to the thread that has been locked. NULL is returned if the
 * current thread cannot be locked (idle).
 */
kernel_list_node_t* lock_thread(const BLOCK_TYPE_E block_type);

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

/* Get all the system threads information.
 * The function will fill the structure given as parameter until there is no
 * more thread to gather information from or the function already gathered
 * size threads. If size is greater than the current threads count in the system
 * then it will be modified to the current threads count.
 *
 * @param thread The array in wich we want to store the threads information.
 * @param size The size of the array given as parameter.  If size is greater
 * than the current threads count in the system then it will be modified to the
 * current threads count.
 * @returns OS_NO_ERR on success, error code otherwise.
 */
OS_RETURN_E get_threads_info(thread_info_t* threads, int32_t* size);

#endif /* __SCHEDULER_H_ */
