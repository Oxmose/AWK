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
#include "../lib/stdint.h"  /* Generic int types */
#include "../lib/stddef.h"  /* OS_RETURN_E, OS_EVENT_ID */
#include "../lib/string.h"  /* strncpy */
#include "../lib/malloc.h"  /* malloc, free */
#include "../drivers/pit.h" /* PIT_INTERRUPT_LINE, get_current_uptime */
#include "../drivers/pic.h" /* set_IRQ_EOI */
#include "../cpu/cpu.h"     /* sti, hlt */
#include "interrupts.h"     /* register_interrupt_handler */
#include "kernel_output.h"  /* kernel_success, kernel_error */
#include "kernel_thread.h"  /* kernel_thread_t */
#include "kernel_queue.h"   /* thread_queue_t, enqueue_thread, dequeue_thread */
#include "panic.h"          /* kernel_panic */

 /* Header file */
 #include "scheduler.h"

/* Kernel thread */
static kernel_thread_t *idle_thread;

/* Active thread */
static kernel_thread_t *active_thread;
static kernel_thread_t *old_thread;

/* System state */
static SYSTEM_STATE_E system_state;

/* Threads management */
static uint32_t last_given_pid;
static uint32_t thread_count;
static uint32_t first_schedule = 0;

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
 * Index 0 is the head, 1 is the tail
 *******************************************************/
static thread_queue_t *active_threads_table[2];
static thread_queue_t *zombie_threads_table[2];
static thread_queue_t *sleeping_threads_table[2];

static thread_queue_t *global_threads_table[2];

void *th_func(void*args)
{
    for(unsigned int i = 0; i < 1000000000; ++i)
    {
        if(i % 100000000 == 0)
        {
            kernel_printf("%d - ", (int)args);
        }
    }

    return (void*)((int)args + 5);
}

static void *init_func(void *args)
{
    (void)args;
    kernel_printf("INIT\n");

    thread_t new_thread;
    thread_t new_thread_2;

    create_thread(&new_thread, th_func, 31, "th\0", (void*)0);
    create_thread(&new_thread_2, th_func, 33, "th2\0", (void*)1);

    int retl;
    wait_thread(new_thread, ((void**)&retl));
    kernel_printf("RET: %d - \n", retl);
    wait_thread(new_thread_2, ((void**)&retl));
    kernel_printf("RET: %d - ", retl);

    return NULL;
}

static void thread_exit(void)
{
    /* Set new thread state */
    active_thread->state = ZOMBIE;

    if(active_thread->joining_thread != NULL &&
       active_thread->joining_thread->state == JOINING)
    {
        active_thread->joining_thread->state = READY;

        OS_RETURN_E err = enqueue_thread(active_thread->joining_thread,
                                         active_threads_table,
                                         active_thread->priority);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue joining thread[%d]\n", err);
            kernel_panic();
        }
    }

    OS_RETURN_E err = enqueue_thread(active_thread,
                                     zombie_threads_table,
                                     0);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not enqueue zombie thread[%d]\n", err);
        kernel_panic();
    }
    
    /* Schedule thread */
    schedule();
}

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

static void select_thread(void)
{
    OS_RETURN_E err;

    /* Get current time */
    uint32_t current_time = get_current_uptime();

    /* Switch running thread */
    old_thread = active_thread;

    /* If the thread was not locked */
    if(old_thread->state == ELECTED)
    {
        err = enqueue_thread(old_thread, active_threads_table, 
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
        err = enqueue_thread(old_thread, sleeping_threads_table, 
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
        sleeping = dequeue_thread(sleeping_threads_table, &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue sleeping thread[%d]\n", err);
            kernel_panic();
        }

        /* If we should wakeup the thread */
        if(sleeping != NULL && sleeping->wakeup_time <= current_time)
        {
            err = enqueue_thread(sleeping, active_threads_table, 
                                 sleeping->priority);
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not enqueue sleeping thread[%d]\n", err);
                kernel_panic();
            }
        }
        else if(sleeping != NULL)
        {
            err = enqueue_thread(sleeping, sleeping_threads_table, 
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
    active_thread = dequeue_thread(active_threads_table, &err);
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

SYSTEM_STATE_E get_system_state(void)
{
    return system_state;
}

void* idle_sys(void* args)
{
    /* We create the init thread */
    thread_t init;
    create_thread(&init, init_func, 32, "init\0", args);

    while(1)
    {
        sti();
        hlt();
    }
    return NULL;
}

OS_RETURN_E init_scheduler(void)
{
    /* Init scheduler settings */
    last_given_pid  = 0;
    thread_count    = 0;
    first_schedule  = 0;

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

    /* Create idle thread */
    idle_thread = malloc(sizeof(kernel_thread_t));
    if(idle_thread == NULL)
    {
        return OS_ERR_MALLOC;
    }
    memset(idle_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    idle_thread->pid            = last_given_pid;
    idle_thread->ppid           = last_given_pid;
    idle_thread->priority       = IDLE_THREAD_PRIORITY;
    idle_thread->args           = 0;
    idle_thread->function       = idle_sys;
    idle_thread->joining_thread = NULL;
    idle_thread->state          = ELECTED;

    /* Init thread context */
    idle_thread->eip = (uint32_t) thread_wrapper;
    idle_thread->esp = 
        (uint32_t) &idle_thread->stack[THREAD_STACK_SIZE - 18];
    idle_thread->ebp = 
        (uint32_t) &idle_thread->stack[THREAD_STACK_SIZE - 1];
    
    /* Init thread stack */
    idle_thread->stack[THREAD_STACK_SIZE - 1] = THREAD_INIT_EFLAGS;
    idle_thread->stack[THREAD_STACK_SIZE - 2] = THREAD_INIT_CS;
    idle_thread->stack[THREAD_STACK_SIZE - 3] = idle_thread->eip;
    idle_thread->stack[THREAD_STACK_SIZE - 4] = 0; /* UNUSED (error core) */
    idle_thread->stack[THREAD_STACK_SIZE - 5] = 0; /* UNUSED (int id) */
    idle_thread->stack[THREAD_STACK_SIZE - 6] = THREAD_INIT_DS;
    idle_thread->stack[THREAD_STACK_SIZE - 7] = THREAD_INIT_ES;
    idle_thread->stack[THREAD_STACK_SIZE - 8] = THREAD_INIT_FS;
    idle_thread->stack[THREAD_STACK_SIZE - 9] = THREAD_INIT_GS;
    idle_thread->stack[THREAD_STACK_SIZE - 10] = THREAD_INIT_SS;
    idle_thread->stack[THREAD_STACK_SIZE - 11] = THREAD_INIT_EAX;
    idle_thread->stack[THREAD_STACK_SIZE - 12] = THREAD_INIT_EBX;
    idle_thread->stack[THREAD_STACK_SIZE - 13] = THREAD_INIT_ECX;
    idle_thread->stack[THREAD_STACK_SIZE - 14] = THREAD_INIT_EDX;
    idle_thread->stack[THREAD_STACK_SIZE - 15] = THREAD_INIT_ESI;
    idle_thread->stack[THREAD_STACK_SIZE - 16] = THREAD_INIT_EDI;
    idle_thread->stack[THREAD_STACK_SIZE - 17] = idle_thread->ebp;
    idle_thread->stack[THREAD_STACK_SIZE - 18] = 
        (uint32_t)&idle_thread->stack[THREAD_STACK_SIZE - 17];

    strncpy(idle_thread->name, "idle\0", 5);

    active_thread = idle_thread;
    old_thread = active_thread;

    system_state = RUNNING;  
    ++thread_count;

    OS_RETURN_E err;

    err = enqueue_thread(idle_thread, global_threads_table, 
                         idle_thread->priority);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not enqueue thread in global table[%d]\n", err);
        kernel_panic();
    }

    /* Register SW interrupt scheduling */
    err = register_interrupt_handler(SCHEDULER_SW_INT_LINE, schedule_int);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    kernel_success("SCHED Initialized\n");

    /* First schedule */
    schedule();

    return err;
}

void schedule(void)
{
    /* Raise scheduling interrupt */
    __asm__ __volatile__("int %0" :: "i" (SCHEDULER_SW_INT_LINE));
}

void schedule_int(cpu_state_t *cpu_state, uint32_t int_id, 
                  stack_state_t *stack_state)
{
    (void) stack_state;

    /* If not first schedule */
    if(first_schedule == 1)
    {
        /* Save the actual ESP */
        active_thread->esp = cpu_state->esp - 4;
        /* Search for next thread */
        select_thread();
        //kernel_printf("SCHED %s -> %s | %d\n", old_thread->name, active_thread->name, thread_count);
    }

    first_schedule = 1;

    if(int_id == PIT_INTERRUPT_LINE)
    {
        /* Send EOI signal */
        set_IRQ_EOI(PIT_IRQ);
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

OS_RETURN_E sleep(const unsigned int time_ms)
{
    /* We cannot sleep in idle */
    if(active_thread == idle_thread)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    active_thread->wakeup_time = get_current_uptime() + time_ms;
    active_thread->state = SLEEPING;

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

OS_RETURN_E create_thread(thread_t *thread, 
                          void *(*function)(void*), 
                          const uint32_t priority, 
                          const char *name, 
                          void *args)
{
    /* Check if priority is free */
    if(priority > KERNEL_LOWEST_PRIORITY)
    {
        return OS_ERR_FORBIDEN_PRIORITY;
    }

    kernel_thread_t *new_thread = malloc(sizeof(kernel_thread_t));
    if(thread != NULL)
    {
        *thread = new_thread;
    }

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

    OS_RETURN_E err = enqueue_thread(new_thread, active_threads_table, 
                                     priority);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = enqueue_thread(new_thread, global_threads_table, 
                         new_thread->priority);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    ++thread_count;

    /* If the priority of the new thread is higher than the curent one */
    if(first_schedule == 1 && priority < active_thread->priority)
    {
        schedule();
    }

    return OS_NO_ERR;
}

OS_RETURN_E wait_thread(thread_t thread, void **ret_val)
{
    if(thread == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }     

    if(thread->state == DEAD)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* If thread already done then remove it from the thread table */
    if(thread->state == ZOMBIE)
    {
        thread->state = DEAD;
        if(ret_val != NULL)
        {
            *ret_val = thread->ret_val;
        }

        remove_thread(zombie_threads_table, thread);
        remove_thread(global_threads_table, thread);
        free(thread);

        return OS_NO_ERR;
    }

    /* Wait for the thread to finish */
    thread->joining_thread = active_thread;
    active_thread->state   = JOINING;

    /* Schedule thread */
    schedule();

    if(ret_val != NULL)
    {
        *ret_val = thread->ret_val;
    }
     
    remove_thread(zombie_threads_table, thread);
    remove_thread(global_threads_table, thread);
    free(thread);

    --thread_count;

    return OS_NO_ERR;
}