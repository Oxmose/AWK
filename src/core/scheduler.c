/*******************************************************************************
 *
 * File: scheduler.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 1.5
 *
 * Kernel scheduler
 * Thread creation and management functions are located in this file.
 *
 ******************************************************************************/

#include "../lib/stdint.h"  /* Generic int types */
#include "../lib/stddef.h"  /* OS_RETURN_E, OS_EVENT_ID */
#include "../lib/string.h"  /* strncpy */
#include "../lib/malloc.h"  /* malloc, free */
#include "../cpu/cpu.h"     /* sti, hlt */
#include "../sync/lock.h"   /* enable_interrupt */
#include "interrupts.h"     /* register_interrupt_handler,
                               set_IRQ_EOI, update_tick */
#include "kernel_output.h"  /* kernel_success, kernel_error */
#include "kernel_thread.h"  /* kernel_thread_t */
#include "kernel_queue.h"   /* thread_queue_t,kernel_enqueue_thread,
                               kernel_dequeue_thread */
#include "panic.h"          /* kernel_panic */

#include "../debug.h"      /* kernel_serial_debug */

/* Header file */
#include "scheduler.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* INT management */
static uint32_t sched_irq;
static uint32_t sched_hw_int_line;

/* Scheduler lock */
static lock_t sched_lock;

/* Kernel thread */
static kernel_thread_t  idle_thread;
static kernel_thread_t* init_thread;

/* Active thread */
static kernel_thread_t* active_thread;
static kernel_thread_t* old_thread;

/* System state */
static SYSTEM_STATE_E system_state;

/* Threads management */
static uint32_t last_given_pid;
static uint32_t thread_count;
static uint32_t first_schedule;

/*******************************************************
 * THREAD TABLES
 * Sorted by priority:
 *     - active_thread: thread priority
 *     - sleeping_threads: thread wakeup time
 *     - io_waiting_threads: thread priority
 *
 * Global thread table used to browse the threads, even those
 * kept in a nutex / semaphore or other structure and that do
 * not appear in the three previous tables.
 *
 * Index 0 is the head, 1 is the tail
 *******************************************************/
static thread_queue_t* active_threads_table[2];
static thread_queue_t* zombie_threads_table[2];
static thread_queue_t* sleeping_threads_table[2];
static thread_queue_t* io_waiting_threads_table[2];
static thread_queue_t* global_threads_table[2];

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Extern user programm entry point */
extern int main(int, char**);

/* INIT thread routine. In addition to the IDLE thread, the INIT thread is the
 * last thread to run. The thread will gather all orphan thread and wait for
 * their death before halting the system. The INIT thread routine is also
 * responsible for calling the main function.
 *
 * @param args The argument to send to the INIT thread, usualy null.
 * @return NULL always
 */
static void* init_func(void* args)
{
    OS_RETURN_E      err;
    kernel_thread_t* thread;
    char*            argv[2] = {"main", NULL};

    #ifdef DEBUG_SCHED
    kernel_serial_debug("INIT Started\n");
    #endif

    (void)args;
    /* Call main */
    main(1, argv);

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Main returned, INIT waiting for children\n");
    #endif

    spinlock_lock(&sched_lock);

    /* Wait all children */
    while(thread_count > 2)
    {

        thread = kernel_dequeue_thread(active_thread->children, &err);
        while(thread != NULL && err == OS_NO_ERR)
        {
            spinlock_unlock(&sched_lock);

            err = wait_thread(thread, NULL);
            if(err != OS_NO_ERR)
            {
                kernel_error("Error while waiting thread in INIT [%d]\n", err);
                kernel_panic();
            }

            spinlock_lock(&sched_lock);

            thread = kernel_dequeue_thread(active_thread->children, &err);
        }
    }
    spinlock_unlock(&sched_lock);

    #ifdef DEBUG_SCHED
    kernel_serial_debug("INIT Ended\n");
    #endif

    /* If here, the system is halted */
    system_state = HALTED;

    return NULL;
}

/* IDLE thread routine. This thread should always be ready, it is the only
 * threa running when no other trhread are ready. It allows better power
 * consumption management. The IDLE thread launches the INIT thread before
 * idling forever.
 *
 * @param args The argument to send to the IDLE thread, usualy null.
 * @return NULL always, should never return.
 */
static void* idle_sys(void* args)
{
    OS_RETURN_E err;
    int8_t      notify = 0;

    #ifdef DEBUG_SCHED
    kernel_serial_debug("IDLE Started\n");
    #endif

    kernel_info("Interrupts unleached\n");
    kernel_printf("=================================== Welcome! ===============\
====================\n");

    sti();

    /* We create the init thread */
    err = create_thread(&init_thread, init_func,
                        KERNEL_HIGHEST_PRIORITY, "init\0", args);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while creating INIT thread [%d]\n", err);
        kernel_panic();
    }

    /* Halt forever, hlt for energy consumption */
    while(1 < 2)
    {
        sti();
        if(system_state == HALTED && notify == 0)
        {
            notify = 1;
            kernel_info("System HALTED");
        }
        hlt();
    }

    return NULL;
}

/* Exit point of a thread. The function will release the resources of the thread
 * and manage its children (INIT will inherit them). Put the thread in a ZOMBIE
 * state. If an other thread is already joining the active thread, then the
 * joining thread will switch from blocked to ready state.
 */
static void thread_exit(void)
{
    OS_RETURN_E      err;
    kernel_thread_t* thread;

    spinlock_lock(&sched_lock);

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Exit thread %d\n", active_thread->pid);
    #endif

    if(active_thread == init_thread)
    {
        spinlock_unlock(&sched_lock);

        /* Schedule thread, should never return since the state is zombie */
        schedule();

        return;
    }

    if(active_thread->joining_thread != NULL &&
       active_thread->joining_thread->state == JOINING)
    {
        #ifdef DEBUG_SCHED
        kernel_serial_debug("Woke up joining thread %d\n",
            active_thread->joining_thread->pid);
        #endif
        active_thread->joining_thread->state = READY;

        err = kernel_enqueue_thread(active_thread->joining_thread,
                                         active_threads_table,
                                         active_thread->priority);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue joining thread[%d]\n", err);
            kernel_panic();
        }
    }

    /* Set new thread state */
    active_thread->state = ZOMBIE;

    err = kernel_enqueue_thread(active_thread, zombie_threads_table, 0);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not enqueue zombie thread[%d]\n", err);
        kernel_panic();
    }

    /* All the children of the thread are inherited by init */
    thread = kernel_dequeue_thread(active_thread->children, &err);
    while(thread != NULL && err == OS_NO_ERR)
    {
        thread->ppid = init_thread->pid;

        if(thread->joining_thread == active_thread)
        {
            thread->joining_thread = NULL;
        }

        err = kernel_enqueue_thread(thread, init_thread->children, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread to init[%d]\n", err);
            kernel_panic();
        }

        thread = kernel_dequeue_thread(active_thread->children, &err);
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from children[%d]\n", err);
        kernel_panic();
    }

    spinlock_unlock(&sched_lock);

    /* Schedule thread */
    schedule();
}

/* Thread launch routine. Wrapper for the actual thread routine. The wrapper
 * will call the thread routine, pass its arguments and gather the return value
 * of the thread function to allow the joining thread to retreive it.
 * Some statistics about the thread might be added in this function.
 */
static void thread_wrapper(void)
{
    /* TODO STAT PROBE OR SOMETHING */
    active_thread->start_time = get_current_uptime();

    active_thread->ret_val = active_thread->function(active_thread->args);

    active_thread->end_time = get_current_uptime();
    active_thread->exec_time =
        active_thread->end_time - active_thread->start_time;

    /* Exit thread properly */
    thread_exit();
}

/* Set the old_thread and active_thread pointers. The function will select the
 * next most prioritary thread to be executed.
 * This function also wake up sleeping thread which wake-up time has been
 * reached
 */
static void select_thread(void)
{
    OS_RETURN_E err;
    uint32_t current_time = get_current_uptime();

    /* Switch running thread */
    old_thread = active_thread;

    /* If the thread was not locked */
    if(old_thread->state == ELECTED)
    {

        err = kernel_enqueue_thread(old_thread, active_threads_table,
                             old_thread->priority);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue old thread[%d]\n", err);
            kernel_panic();
        }

        old_thread->state = READY;
    }
    else if(active_thread->state == SLEEPING)
    {
        err = kernel_enqueue_thread(old_thread, sleeping_threads_table,
                                    old_thread->wakeup_time);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue old thread[%d]\n", err);
            kernel_panic();
        }
    }

    /* Wake up the sleeping threads */
    kernel_thread_t *sleeping;
    do
    {
        sleeping = kernel_dequeue_thread(sleeping_threads_table, &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue sleeping thread[%d]\n", err);
            kernel_panic();
        }

        /* If we should wakeup the thread */
        if(sleeping != NULL && sleeping->wakeup_time < current_time)
        {
            err = kernel_enqueue_thread(sleeping, active_threads_table,
                                        sleeping->priority);

            if(err != OS_NO_ERR)
            {
                kernel_error("Could not enqueue sleeping thread[%d]\n", err);
                kernel_panic();
            }
            sleeping->state = READY;
        }
        else if(sleeping != NULL)
        {
            err = kernel_enqueue_thread(sleeping, sleeping_threads_table,
                                        sleeping->wakeup_time);
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not enqueue sleeping thread[%d]\n", err);
                kernel_panic();
            }
            break;
        }
    } while(sleeping != NULL);

    /* Get the new thread */
    active_thread = kernel_dequeue_thread(active_threads_table, &err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue next thread[%d]\n", err);
        kernel_panic();
    }
    if(active_thread == NULL)
    {
        kernel_error("Next thread to schedule should not be NULL\n");
        kernel_panic();
    }
    active_thread->state = ELECTED;
}

/* !!! THIS FUNCTION SHOULD NEVER BE CALLED OUTSIDE OF AN INTERRUPT !!!
 * Scheduling function, will the interrupt saved the registers in the current
 * thread stack. The function will call the select_thread function and then
 * set the CPU registers with the values on the new active_thread stack.
 * Then the function manage interruption and return from interrupt.
 */
static void schedule_int(cpu_state_t *cpu_state, uint32_t int_id,
                         stack_state_t *stack_state)
{
    OS_RETURN_E err;
    (void) stack_state;

#if SCHEDULE_DYN_PRIORITY

    if(active_thread != &idle_thread && active_thread != init_thread)
    {
        if(int_id == PIT_INTERRUPT_LINE)
        {

            /* Here the thread consumed all its time slice four times in a row
             * then we decay its priority by one.
             */
            ++(active_thread->full_consume);
            if(active_thread->full_consume >= 50)
            {
                active_thread->full_consume = 0;
                if(active_thread->priority < KERNEL_LOWEST_PRIORITY)
                {
                    ++(active_thread->priority);
                }
            }

        }
        else
        {
            /* Here the thread did not consumed all its time slice */
            active_thread->full_consume = 0;
        }
         /* Upgrade process priority by 1 if the thread has not been executed since
          * the last 100 ticks. Note that the actual change will be seen when more
          * prioritary threads will be puck bakc in the queue later.
          */
        thread_queue_t *cursor = active_threads_table[0];
        while(cursor != NULL)
        {
            if(cursor->thread == &idle_thread || cursor->thread == init_thread)
            {
                cursor = cursor->next;
                continue;
            }

            ++cursor->thread->last_sched;
            if(cursor->thread->last_sched >= 25)
            {
                if(cursor->thread->priority > KERNEL_HIGHEST_PRIORITY)
                {
                    --(cursor->thread->priority);
                    --(cursor->priority);
                }
                cursor->thread->last_sched = 0;
            }

            cursor = cursor->next;
        }

        active_thread->last_sched = 0;
    }

#endif /* SCHEDULE_DYN_PRIORITY */

    /* If not first schedule */
    if(first_schedule == 1)
    {
        /* Save the actual ESP */
        active_thread->esp = cpu_state->esp - 4;
        /* Search for next thread */
        select_thread();
    }

    first_schedule = 1;

    if(int_id == sched_hw_int_line)
    {
        /* Update TIMER tick count */
        update_tick();

        /* Send EOI signal */
        err = set_IRQ_EOI(sched_irq);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not EIO scheduler timer[%d]\n", err);
            kernel_panic();
        }
    }

    /* Restore thread esp */
    __asm__ __volatile__("mov %%eax, %%esp": :"a"(active_thread->esp));
    __asm__ __volatile__("pop %esp");
    __asm__ __volatile__("pop %ebp");
    __asm__ __volatile__("pop %edi");
    __asm__ __volatile__("pop %esi");
    __asm__ __volatile__("pop %edx");
    __asm__ __volatile__("pop %ecx");
    __asm__ __volatile__("pop %ebx");
    __asm__ __volatile__("pop %eax");

    __asm__ __volatile__("pop %ss");
    __asm__ __volatile__("pop %gs");
    __asm__ __volatile__("pop %fs");
    __asm__ __volatile__("pop %es");
    __asm__ __volatile__("pop %ds");
    __asm__ __volatile__("add $8, %esp");
    __asm__ __volatile__("iret");
}

SYSTEM_STATE_E get_system_state(void)
{
    return system_state;
}

OS_RETURN_E init_scheduler(void)
{
    OS_RETURN_E err;

    /* Init scheduler settings */
    last_given_pid  = 0;
    thread_count    = 0;
    first_schedule  = 0;

    spinlock_init(&sched_lock);

    active_thread       = NULL;
    old_thread          = NULL;

    /* Init thread tables */
    global_threads_table[0]   = NULL;
    global_threads_table[1]   = NULL;
    active_threads_table[0]   = NULL;
    active_threads_table[1]   = NULL;
    zombie_threads_table[0]   = NULL;
    zombie_threads_table[1]   = NULL;
    sleeping_threads_table[0] = NULL;
    sleeping_threads_table[1] = NULL;
    io_waiting_threads_table[0] = NULL;
    io_waiting_threads_table[1] = NULL;

    /* Create idle thread */
    memset(&idle_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    idle_thread.pid            = last_given_pid;
    idle_thread.ppid           = last_given_pid;
    idle_thread.priority       = IDLE_THREAD_PRIORITY;
    idle_thread.args           = 0;
    idle_thread.function       = idle_sys;
    idle_thread.joining_thread = NULL;
    idle_thread.state          = ELECTED;
    idle_thread.children[0]    = NULL;
    idle_thread.children[1]    = NULL;

    /* Init thread context */
    idle_thread.eip = (uint32_t) thread_wrapper;
    idle_thread.esp =
        (uint32_t) &idle_thread.stack[THREAD_STACK_SIZE - 18];
    idle_thread.ebp =
        (uint32_t) &idle_thread.stack[THREAD_STACK_SIZE - 1];

    /* Init thread stack */
    idle_thread.stack[THREAD_STACK_SIZE - 1] = THREAD_INIT_EFLAGS;
    idle_thread.stack[THREAD_STACK_SIZE - 2] = THREAD_INIT_CS;
    idle_thread.stack[THREAD_STACK_SIZE - 3] = idle_thread.eip;
    idle_thread.stack[THREAD_STACK_SIZE - 4] = 0; /* UNUSED (error core) */
    idle_thread.stack[THREAD_STACK_SIZE - 5] = 0; /* UNUSED (int id) */
    idle_thread.stack[THREAD_STACK_SIZE - 6] = THREAD_INIT_DS;
    idle_thread.stack[THREAD_STACK_SIZE - 7] = THREAD_INIT_ES;
    idle_thread.stack[THREAD_STACK_SIZE - 8] = THREAD_INIT_FS;
    idle_thread.stack[THREAD_STACK_SIZE - 9] = THREAD_INIT_GS;
    idle_thread.stack[THREAD_STACK_SIZE - 10] = THREAD_INIT_SS;
    idle_thread.stack[THREAD_STACK_SIZE - 11] = THREAD_INIT_EAX;
    idle_thread.stack[THREAD_STACK_SIZE - 12] = THREAD_INIT_EBX;
    idle_thread.stack[THREAD_STACK_SIZE - 13] = THREAD_INIT_ECX;
    idle_thread.stack[THREAD_STACK_SIZE - 14] = THREAD_INIT_EDX;
    idle_thread.stack[THREAD_STACK_SIZE - 15] = THREAD_INIT_ESI;
    idle_thread.stack[THREAD_STACK_SIZE - 16] = THREAD_INIT_EDI;
    idle_thread.stack[THREAD_STACK_SIZE - 17] = idle_thread.ebp;
    idle_thread.stack[THREAD_STACK_SIZE - 18] =
        (uint32_t)&idle_thread.stack[THREAD_STACK_SIZE - 17];

    strncpy(idle_thread.name, "idle\0", 5);

    active_thread = &idle_thread;
    old_thread = active_thread;

    system_state = RUNNING;
    ++thread_count;

    #ifdef DEBUG_SCHED
    kernel_serial_debug("IDLE thread created\n");
    #endif

    err = kernel_enqueue_thread(&idle_thread, global_threads_table,
                                idle_thread.priority);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not enqueue thread in global table[%d]\n", err);
        kernel_panic();
    }

    sched_irq         = (uint32_t)get_IRQ_SCHED_TIMER();
    sched_hw_int_line = (uint32_t)get_line_SCHED_HW();

    /* Register SW interrupt scheduling */
    err = register_interrupt_handler(SCHEDULER_SW_INT_LINE, schedule_int);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Register HW interrupt scheduling, unregister interrupt since all driver
     * should register at least a dunny handler at init.
     */
    err = remove_interrupt_handler(sched_hw_int_line);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = register_interrupt_handler(sched_hw_int_line, schedule_int);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    kernel_success("SCHEDULER Initialized\n");

    /* First schedule */
    schedule();

    /* We should never return fron this function */
    return OS_ERR_UNAUTHORIZED_ACTION;
}

void schedule(void)
{
    /* Raise scheduling interrupt */
    __asm__ __volatile__("int %0" :: "i" (SCHEDULER_SW_INT_LINE));
}

OS_RETURN_E sleep(const unsigned int time_ms)
{
    /* We cannot sleep in idle */
    if(active_thread == &idle_thread)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    active_thread->wakeup_time = get_current_uptime() + time_ms;
    active_thread->state = SLEEPING;

    #ifdef DEBUG_SCHED
   // kernel_serial_debug("Thread %d asleep until %d\n", active_thread->pid,
     //  active_thread->wakeup_time );
    #endif

    schedule();

    return OS_NO_ERR;
}

uint32_t get_thread_count(void)
{
    return thread_count;
}

int32_t get_pid(void)
{
    return active_thread->pid;
}

int32_t get_ppid(void)
{
    return active_thread->ppid;
}

char *get_current_thread_name(void)
{
    return active_thread->name;
}

uint32_t get_priority(void)
{
    return active_thread->priority;
}

kernel_thread_t* get_active_thread(void)
{
    return active_thread;
}

OS_RETURN_E create_thread(thread_t* thread,
                          void* (*function)(void*),
                          const uint32_t priority,
                          const char *name,
                          void* args)
{
    OS_RETURN_E      err;
    kernel_thread_t* new_thread;

    if(thread != NULL)
    {
        *thread = NULL;
    }

    /* Check if priority is free */
    if(priority > KERNEL_LOWEST_PRIORITY)
    {
        return OS_ERR_FORBIDEN_PRIORITY;
    }

    new_thread = malloc(sizeof(kernel_thread_t));

    if(new_thread == NULL)
    {
        return OS_ERR_MALLOC;
    }
    memset(new_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    new_thread->pid            = ++last_given_pid;
    new_thread->ppid           = active_thread->pid;
    new_thread->priority       = priority;
    new_thread->args           = args;
    new_thread->function       = function;
    new_thread->joining_thread = NULL;
    new_thread->state          = READY;
    new_thread->children[0]    = NULL;
    new_thread->children[1]    = NULL;
    new_thread->full_consume   = 0;
    new_thread->last_sched     = 0;


     /* Init thread context */
    new_thread->eip = (uint32_t) thread_wrapper;
    new_thread->esp =
        (uint32_t) &new_thread->stack[THREAD_STACK_SIZE - 18];
    new_thread->ebp =
        (uint32_t) &new_thread->stack[THREAD_STACK_SIZE - 1];

    /* Init thread stack */
    new_thread->stack[THREAD_STACK_SIZE - 1] = THREAD_INIT_EFLAGS;
    new_thread->stack[THREAD_STACK_SIZE - 2] = THREAD_INIT_CS;
    new_thread->stack[THREAD_STACK_SIZE - 3] = new_thread->eip;
    new_thread->stack[THREAD_STACK_SIZE - 4] = 0; /* UNUSED (error core) */
    new_thread->stack[THREAD_STACK_SIZE - 5] = 0; /* UNUSED (int id) */
    new_thread->stack[THREAD_STACK_SIZE - 6] = THREAD_INIT_DS;
    new_thread->stack[THREAD_STACK_SIZE - 7] = THREAD_INIT_ES;
    new_thread->stack[THREAD_STACK_SIZE - 8] = THREAD_INIT_FS;
    new_thread->stack[THREAD_STACK_SIZE - 9] = THREAD_INIT_GS;
    new_thread->stack[THREAD_STACK_SIZE - 10] = THREAD_INIT_SS;
    new_thread->stack[THREAD_STACK_SIZE - 11] = THREAD_INIT_EAX;
    new_thread->stack[THREAD_STACK_SIZE - 12] = THREAD_INIT_EBX;
    new_thread->stack[THREAD_STACK_SIZE - 13] = THREAD_INIT_ECX;
    new_thread->stack[THREAD_STACK_SIZE - 14] = THREAD_INIT_EDX;
    new_thread->stack[THREAD_STACK_SIZE - 15] = THREAD_INIT_ESI;
    new_thread->stack[THREAD_STACK_SIZE - 16] = THREAD_INIT_EDI;
    new_thread->stack[THREAD_STACK_SIZE - 17] = new_thread->ebp;
    new_thread->stack[THREAD_STACK_SIZE - 18] =
        (uint32_t)&new_thread->stack[THREAD_STACK_SIZE - 17];

    strncpy(new_thread->name, name, THREAD_MAX_NAME_LENGTH);

    spinlock_lock(&sched_lock);
    err = kernel_enqueue_thread(new_thread, active_threads_table,
                                priority);
    if(err != OS_NO_ERR)
    {
        free(new_thread);
        return err;
    }

    err = kernel_enqueue_thread(new_thread, global_threads_table,
                         new_thread->priority);
    if(err != OS_NO_ERR)
    {
        free(new_thread);
        return err;
    }

    err = kernel_enqueue_thread(new_thread, active_thread->children, 0);
    if(err != OS_NO_ERR)
    {
        free(new_thread);
        return err;
    }

    ++thread_count;

    spinlock_unlock(&sched_lock);

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Created thread %d\n", new_thread->pid);
    #endif

    if(thread != NULL)
    {
        *thread = new_thread;
    }

    return OS_NO_ERR;
}

OS_RETURN_E wait_thread(thread_t thread, void** ret_val)
{
    if(thread == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(thread->state == DEAD)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    spinlock_lock(&sched_lock);

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Thread %d waiting for thread %d\n",
                         active_thread->pid,
                        thread->pid);
    #endif

    /* If thread already done then remove it from the thread table */
    if(thread->state == ZOMBIE)
    {
        thread->state = DEAD;
        if(ret_val != NULL)
        {
            *ret_val = thread->ret_val;
        }

        kernel_remove_thread(active_thread->children, thread);
        kernel_remove_thread(zombie_threads_table, thread);
        kernel_remove_thread(global_threads_table, thread);

        #ifdef DEBUG_SCHED
        kernel_serial_debug("Thread %d joined thread %d\n",
                             active_thread->pid,
                             thread->pid);
        #endif

        --thread_count;
        free(thread);

        spinlock_unlock(&sched_lock);

        return OS_NO_ERR;
    }

    /* Wait for the thread to finish */
    thread->joining_thread = active_thread;
    active_thread->state   = JOINING;

    spinlock_unlock(&sched_lock);

    /* Schedule thread */
    schedule();

    spinlock_lock(&sched_lock);

    if(ret_val != NULL)
    {
        *ret_val = thread->ret_val;
    }

    kernel_remove_thread(active_thread->children, thread);
    kernel_remove_thread(zombie_threads_table, thread);
    kernel_remove_thread(global_threads_table, thread);

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Thread %d joined thread %d\n",
                         active_thread->pid,
                         thread->pid);
    #endif

    --thread_count;
    free(thread);

    spinlock_unlock(&sched_lock);

    return OS_NO_ERR;
}

OS_RETURN_E lock_thread(const BLOCK_TYPE_E block_type)
{
    /* Cant lock kernel thread */
    if(active_thread == &idle_thread)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    /* Lock the thread */
    active_thread->state      = BLOCKED;
    active_thread->block_type = block_type;

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Thread %d locked, reason: %d\n",
                        active_thread->pid,
                        block_type);
    #endif

    /* Schedule to an other thread */
    schedule();

    return OS_NO_ERR;
}

OS_RETURN_E unlock_thread(const thread_t thread,
                          const BLOCK_TYPE_E block_type,
                          const uint8_t do_schedule)
{
    OS_RETURN_E err;

    /* Check thread value */
    if(thread == NULL || thread  == &idle_thread)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* Check thread state */
    if(thread->state != BLOCKED ||
       thread->block_type != block_type)
    {
        switch(block_type)
        {
            case SEM:
                return OS_ERR_NO_SEM_BLOCKED;
            case MUTEX:
                return OS_ERR_NO_MUTEX_BLOCKED;
            case QUEUE:
                return OS_ERR_NO_QUEUE_BLOCKED;
            default:
                return OS_ERR_NULL_POINTER;
        }

    }
    spinlock_lock(&sched_lock);
    err = kernel_enqueue_thread(thread, active_threads_table, thread->priority);
    spinlock_unlock(&sched_lock);

    if(err != OS_NO_ERR)
    {
        kernel_error("Could not enqueue thread in active table[%d]\n", err);
        kernel_panic();
    }

    /* Unlock thread state */
    thread->state = READY;

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Thread %d unlocked, reason: %d\n",
                         thread->pid,
                         block_type);
    #endif

    /* Schedule if asked for */
    if(do_schedule)
    {
        schedule();
    }

    return OS_NO_ERR;
}

OS_RETURN_E lock_io(const BLOCK_TYPE_E block_type)
{
    OS_RETURN_E err;

    /* Cant lock kernel thread */
    if(active_thread == &idle_thread)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    if(block_type == IO_KEYBOARD)
    {
        /* Lock current tread */
        active_thread->block_type = block_type;
                active_thread->state = BLOCKED;

        spinlock_lock(&sched_lock);
        err = kernel_enqueue_thread(active_thread, io_waiting_threads_table,
                                    active_thread->io_req_time);
        spinlock_unlock(&sched_lock);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread in kbd IO table[%d]\n", err);
            kernel_panic();
        }

        #ifdef DEBUG_SCHED
        kernel_serial_debug("Thread %d io-locked, reason: %d\n",
                             active_thread->pid,
                             block_type);
        #endif

        /* Schedule to let other thread execute */
        schedule();
    }

    return OS_NO_ERR;
}

OS_RETURN_E unlock_io(const BLOCK_TYPE_E block_type)
{
    kernel_thread_t *thread;
    OS_RETURN_E err;

    if(block_type != IO_KEYBOARD)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    spinlock_lock(&sched_lock);

    thread = kernel_dequeue_thread(io_waiting_threads_table, &err);
    if(thread != NULL && err == OS_NO_ERR)
    {
        err = kernel_enqueue_thread(thread, active_threads_table,
                                    thread->priority);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread in active table[%d]\n", err);
            kernel_panic();
        }
        thread->state = READY;
    }
    else
    {
        kernel_error("Could not dequee thread in kbd IO table[%d]\n", err);
        kernel_panic();
    }

    #ifdef DEBUG_SCHED
        kernel_serial_debug("Thread %d io-unlocked, reason: %d\n",
                             active_thread->pid,
                             block_type);
        #endif

    spinlock_unlock(&sched_lock);

    return OS_NO_ERR;
}

OS_RETURN_E get_threads_info(thread_info_t* threads, int32_t* size)
{
    int32_t          i;
    thread_queue_t*  cursor;
    kernel_thread_t* cursor_thread;

    if(threads == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    if(size == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    spinlock_lock(&sched_lock);

    if(*size > (int)thread_count)
    {
        *size = thread_count;
    }

    /* Walk the thread list and fill the structures */
    cursor = global_threads_table[0];
    cursor_thread = cursor->thread;
    for(i = 0; cursor != NULL && i < *size; ++i)
    {
        thread_info_t *current = &threads[i];
        current->pid = cursor_thread->pid;
        current->ppid = cursor_thread->ppid;
        strncpy(current->name, cursor_thread->name, THREAD_MAX_NAME_LENGTH);
        current->priority = cursor_thread->priority;
        current->state = cursor_thread->state;
        current->start_time = cursor_thread->start_time;
        current->children = cursor_thread->children[0];
        if(current->state != ZOMBIE)
        {
            current->end_time = 0;
            current->exec_time = get_current_uptime();
        }
        else
        {
            current->end_time = cursor_thread->end_time;
            current->exec_time = cursor_thread->exec_time;
        }

        cursor = cursor->next;
        cursor_thread = cursor->thread;
    }

    spinlock_unlock(&sched_lock);

    return OS_NO_ERR;
}