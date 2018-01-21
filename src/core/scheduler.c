/*******************************************************************************
 *
 * File: scheduler.c
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

#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E, OS_EVENT_ID */
#include "../lib/string.h"      /* strncpy */
#include "../memory/heap.h"     /* kmalloc, kfree */
#include "../cpu/cpu.h"         /* sti, hlt */
#include "../sync/lock.h"       /* spinlock */
#include "../drivers/graphic.h" /* colorsheme */
#include "interrupts.h"         /* register_interrupt_handler,
                                 set_IRQ_EOI, update_tick */
#include "kernel_output.h"      /* kernel_success, kernel_error */
#include "kernel_list.h"        /* kernel_list_t, kernel_list_node_t */

#include "panic.h"              /* kernel_panic */

#include "../debug.h"           /* DEBUG */

/* Header file */
#include "scheduler.h"

 /*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* INT management */
static uint32_t sched_irq;
static uint32_t sched_hw_int_line;

/* Threads management */
static volatile uint32_t last_given_pid;
static volatile uint32_t thread_count;
static volatile uint32_t first_schedule;

/* Kernel thread */
static kernel_thread_t*    idle_thread;
static kernel_list_node_t* idle_thread_node;
static kernel_thread_t*    init_thread;
static kernel_list_node_t* init_thread_node;

/* Active thread */
static kernel_thread_t*    active_thread;
static kernel_list_node_t* active_thread_node;
static kernel_thread_t*    old_thread;
static kernel_list_node_t* old_thread_node;

/* System state */
static volatile SYSTEM_STATE_E system_state;

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
 *******************************************************/
static kernel_list_t* active_threads_table;
static kernel_list_t* zombie_threads_table;
static kernel_list_t* sleeping_threads_table;
static kernel_list_t* io_waiting_threads_table;
static kernel_list_t* global_threads_table;

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
    OS_RETURN_E         err;
    kernel_list_node_t* thread_node;
    kernel_thread_t*    thread;
    char*               argv[2] = {"main", NULL};

    #ifdef DEBUG_SCHED
    kernel_serial_debug("INIT Started\n");
    #endif

    (void)args;

    /* Call main */
    main(1, argv);

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Main returned, INIT waiting for children\n");
    #endif

    disable_local_interrupt();

    /* Wait all children */
    while(thread_count > 2)
    {
        thread_node = kernel_list_delist_data(active_thread->children, &err);
        while(thread_node != NULL && err == OS_NO_ERR)
        {
            enable_local_interrupt();

            thread = (kernel_thread_t*)thread_node->data;

            err = wait_thread(thread, NULL);
            if(err != OS_NO_ERR)
            {
                kernel_error("Error while waiting thread in INIT [%d]\n", err);
                kernel_panic();
            }

            err = kernel_list_delete_node(&thread_node);
            if(err != OS_NO_ERR)
            {
                kernel_error("Error while deleting thread node in INIT [%d]\n", err);
                kernel_panic();
            }

            disable_local_interrupt();

            thread_node = kernel_list_delist_data(active_thread->children, &err);
        }
    }
    enable_local_interrupt();

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
    OS_RETURN_E   err;
    colorscheme_t buffer;
    colorscheme_t new_scheme;
    int8_t        notify = 0;


    #ifdef DEBUG_SCHED
    kernel_serial_debug("IDLE Started\n");
    #endif

    kernel_printf("\nWelcome to ");

    new_scheme.foreground = FG_CYAN;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

    /* No need to test return value */
    save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    set_color_scheme(new_scheme);

    /* Print tag */
    kernel_printf("AWK\n\n");

    /* Restore original screen color scheme */
    set_color_scheme(buffer);

    (void)args;
    (void)err;
    (void)init_func;
    /* We create the init thread */
    err = create_thread(&init_thread, init_func,
                        KERNEL_HIGHEST_PRIORITY, "init", args);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while creating INIT thread [%d]\n", err);
        kernel_panic();
    }

    #ifdef DEBUG_SCHED
    kernel_serial_debug("INIT Created\n");
    #endif


    /* Halt forever, hlt for energy consumption */
    while(1 < 2)
    {
        enable_local_interrupt();
        if(system_state == HALTED && notify == 0)
        {
            notify = 1;
            kernel_info("System HALTED");
            disable_local_interrupt();
        }
        hlt();
    }

    /* If we return better go away and cry in a corner */
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
    kernel_thread_t*    thread;
    kernel_thread_t*    joining_thread = NULL;
    kernel_list_node_t* node;

    disable_local_interrupt();

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Exit thread %d\n", active_thread->pid);
    #endif

    if(active_thread == init_thread)
    {
        active_thread->state = ZOMBIE;
        enable_local_interrupt();

        /* Schedule thread, should never return since the state is zombie */
        schedule();

        return;
    }

    if(active_thread->joining_thread != NULL)
    {
        joining_thread = (kernel_thread_t*)active_thread->joining_thread->data;
    }

    if(joining_thread != NULL && joining_thread->state == JOINING)
    {
        #ifdef DEBUG_SCHED
        kernel_serial_debug("Woke up joining thread %d\n",
            joining_thread->pid);
        #endif
        joining_thread->state = READY;

        err = kernel_list_enlist_data(active_thread->joining_thread,
                                      active_threads_table,
                                      joining_thread->priority);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue joining thread[%d]\n", err);
            kernel_panic();
        }
    }

    /* Set new thread state */
    active_thread->state = ZOMBIE;

    err = kernel_list_enlist_data(active_thread_node, zombie_threads_table, 0);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not enqueue zombie thread[%d]\n", err);
        kernel_panic();
    }

    /* All the children of the thread are inherited by init */
    node = kernel_list_delist_data(active_thread->children, &err);
    while(node != NULL && err == OS_NO_ERR)
    {
        thread = (kernel_thread_t*)node->data;
        thread->ppid = init_thread->pid;

        if(thread->joining_thread->data == active_thread)
        {
            thread->joining_thread->data = NULL;
        }

        err = kernel_list_enlist_data(node, init_thread->children, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread to init[%d]\n", err);
            kernel_panic();
        }

        node = kernel_list_delist_data(active_thread->children, &err);
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from children[%d]\n", err);
        kernel_panic();
    }

    /* Delete lsit */
    err = kernel_list_delete_list(&active_thread->children);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not delete lsit of children[%d]\n", err);
        kernel_panic();
    }

    enable_local_interrupt();

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

    if(active_thread->function == NULL)
    {
        kernel_error("Thread routine cannot be NULL\n");
        kernel_panic();
    }
    active_thread->ret_val = active_thread->function(active_thread->args);

    active_thread->end_time = get_current_uptime();
    active_thread->exec_time = active_thread->end_time -
                               active_thread->start_time;

    /* Exit thread properly */
    thread_exit();
}

/* Clean a thread that is currently being joined by the curent active thread.
 * Remove the thread from all lists and celan the lists nodes. Scheduler lock
 * should be locked before calling this function.
 *
 * @param thread The thread to clean.
 */
static void clean_joined_thread(kernel_thread_t* thread)
{
    kernel_list_node_t* node;
    OS_RETURN_E         err;

    /* Remove node from children table */
    node = kernel_list_find_node(active_thread->children, thread, &err);
    if(err != OS_NO_ERR && err != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("Could not find joined thread in chlidren table[%d]\n",
                     err);
        kernel_panic();
    }
    if(node != NULL && err == OS_NO_ERR)
    {
        err = kernel_list_remove_node_from(active_thread->children, node);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could delete thread node in children table[%d]\n",
                         err);
            kernel_panic();
        }
        err = kernel_list_delete_node(&node);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could delete thread node[%d]\n", err);
            kernel_panic();
        }
    }

    /* Remove node from zombie table */
    node = kernel_list_find_node(zombie_threads_table, thread, &err);
    if(err != OS_NO_ERR && err != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("Could not find joined thread in zombie table[%d]\n",
                     err);
        kernel_panic();
    }
    if(node != NULL && err == OS_NO_ERR)
    {
        err = kernel_list_remove_node_from(zombie_threads_table, node);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could delete thread node in zombie table[%d]\n",
                         err);
            kernel_panic();
        }
        err = kernel_list_delete_node(&node);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could delete thread node[%d]\n", err);
            kernel_panic();
        }
    }

    /* Remove node from general table */
    node = kernel_list_find_node(global_threads_table, thread, &err);
    if(err != OS_NO_ERR && err != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("Could not find joined thread in general table[%d]\n",
                     err);
        kernel_panic();
    }
    if(node != NULL && err == OS_NO_ERR)
    {
        err = kernel_list_remove_node_from(global_threads_table, node);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could delete thread node in general table[%d]\n",
                         err);
            kernel_panic();
        }
        err = kernel_list_delete_node(&node);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could delete thread node[%d]\n", err);
            kernel_panic();
        }
    }

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Thread %d joined thread %d\n",
                         active_thread->pid,
                         thread->pid);
    #endif

    kfree(thread);

    --thread_count;

}

/* Set the old_thread and active_thread pointers. The function will select the
 * next most prioritary thread to be executed.
 * This function also wake up sleeping thread which wake-up time has been
 * reached
 */
static void select_thread(void)
{
    OS_RETURN_E         err;
    kernel_thread_t*    sleeping;
    kernel_list_node_t* sleeping_node;
    uint32_t             current_time = get_current_uptime();

    /* Switch running thread */
    old_thread = active_thread;
    old_thread_node = active_thread_node;


    /* If the thread was not locked */
    if(old_thread->state == RUNNING)
    {

        err = kernel_list_enlist_data(old_thread_node,
                                      active_threads_table,
                                      old_thread->priority);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue old thread[%d]\n", err);
            kernel_panic();
        }

        old_thread->state = READY;
    }
    else if(old_thread->state == SLEEPING)
    {
        err = kernel_list_enlist_data(old_thread_node,
                                      sleeping_threads_table,
                                      old_thread->wakeup_time);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue old thread[%d]\n", err);
            kernel_panic();
        }
    }

    /* Wake up the sleeping threads */
    do
    {
        sleeping_node = kernel_list_delist_data(sleeping_threads_table, &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue sleeping thread[%d]\n", err);
            kernel_panic();
        }

        /* If nothing to wakeup */
        if(sleeping_node == NULL)
        {
            break;
        }

        sleeping = (kernel_thread_t*)sleeping_node->data;

        /* If we should wakeup the thread */
        if(sleeping != NULL && sleeping->wakeup_time < current_time)
        {
            err = kernel_list_enlist_data(sleeping_node,
                                          active_threads_table,
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
            err = kernel_list_enlist_data(sleeping_node,
                                          sleeping_threads_table,
                                          sleeping->wakeup_time);
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not enqueue sleeping thread[%d]\n", err);
                kernel_panic();
            }
            break;
        }
    } while(sleeping_node != NULL);

    /* Get the new thread */
    active_thread_node = kernel_list_delist_data(active_threads_table, &err);
    if(active_thread_node == NULL || err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue next thread[%d]\n", err);
        kernel_panic();
    }

    active_thread = (kernel_thread_t*)active_thread_node->data;

    if(active_thread == NULL)
    {
        kernel_error("Next thread to schedule should not be NULL\n");
        kernel_panic();
    }
    active_thread->state = RUNNING;
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

    if(get_local_interrupt_enabled() != 1)
    {
        kernel_error("Interrupts should not be disabled when calling the\
 scheduler\n");
        kernel_panic();
    }

#if SCHEDULE_DYN_PRIORITY
    kernel_list_node_t *cursor;
    kernel_thread_t* thread;

    if(active_thread != idle_thread && active_thread != init_thread)
    {
        if(int_id == PIT_INTERRUPT_LINE)
        {
            /* Here the thread consumed all its time slice so it get its init
             * priority.
             */
             active_thread->priority = active_thread->init_prio;
        }
         /* Upgrade process priority by 1 if the thread has not been executed since
          * the last 100 ticks. Note that the actual change will be seen when more
          * prioritary threads will be puck bakc in the queue later.
          */
        cursor = active_threads_table->head;
        while(cursor != NULL)
        {
            if(cursor->data == idle_thread || cursor->data == init_thread)
            {
                cursor = cursor->next;
                continue;
            }

            thread = (kernel_thread_t*)cursor->data;

            ++thread->last_sched;
            if(thread->last_sched >= 25)
            {
                if(thread->priority > KERNEL_HIGHEST_PRIORITY)
                {
                    --(thread->priority);
                    --(cursor->priority);
                }
                thread->last_sched = 0;
            }

            cursor = cursor->next;
        }

        active_thread->last_sched = 0;
    }

#endif /* SCHEDULE_DYN_PRIORITY */

    (void) stack_state;

    /* If not first schedule */
    if(first_schedule == 1)
    {
        /* Save the actual ESP */
        active_thread->esp = cpu_state->esp - 4;

        /* Search for next thread */
        select_thread();
    }
    else
    {
        first_schedule = 1;
    }

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Sched %d -> %d\n",
                         old_thread->pid,
                         active_thread->pid);
    #endif

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
    kernel_list_node_t* second_idle_thread_node;

    /* Init scheduler settings */
    last_given_pid  = 0;
    thread_count    = 0;
    first_schedule  = 0;

    idle_thread   = NULL;
    idle_thread_node  = NULL;
    init_thread  = NULL;
    init_thread_node  = NULL;

    active_thread      = NULL;
    active_thread_node = NULL;
    old_thread         = NULL;
    old_thread_node    = NULL;
    idle_thread        = NULL;
    idle_thread_node   = NULL;
    init_thread        = NULL;
    init_thread_node   = NULL;

    /* Init thread tables */
    global_threads_table     = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create global_threads_table[%d]\n", err);
        kernel_panic();
    }
    active_threads_table     = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create active_threads_table[%d]\n", err);
        kernel_panic();
    }
    zombie_threads_table     = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create zombie_threads_table[%d]\n", err);
        kernel_panic();
    }
    sleeping_threads_table   = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create sleeping_threads_table[%d]\n", err);
        kernel_panic();
    }
    io_waiting_threads_table = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create io_waiting_threads_table[%d]\n", err);
        kernel_panic();
    }

    /* Create idle thread */
    idle_thread = kmalloc(sizeof(kernel_thread_t));
    idle_thread_node = kernel_list_create_node(idle_thread, &err);

    if(err != OS_NO_ERR || idle_thread == NULL || idle_thread_node == NULL)
    {
        kernel_error("Could not create IDLE thread\n");
        kernel_panic();
    }

    memset(idle_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    idle_thread->pid            = last_given_pid;
    idle_thread->ppid           = last_given_pid;
    idle_thread->priority       = IDLE_THREAD_PRIORITY;
    idle_thread->init_prio      = IDLE_THREAD_PRIORITY;
    idle_thread->args           = 0;
    idle_thread->function       = idle_sys;
    idle_thread->joining_thread = NULL;
    idle_thread->state          = RUNNING;

    idle_thread->children = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create children table[%d]\n", err);
        kernel_panic();
    }

    /* Init thread context */
    idle_thread->eip = (uint32_t) thread_wrapper;
    idle_thread->esp =
        (uint32_t)&idle_thread->kernel_stack[THREAD_STACK_SIZE - 18];
    idle_thread->ebp =
        (uint32_t)&idle_thread->kernel_stack[THREAD_STACK_SIZE - 1];

    /* Init thread stack */
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 1] = 0x00000002;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 2] = THREAD_INIT_CS;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 3] = idle_thread->eip;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 4] = 0; /* UNUSED (error core) */
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 5] = 0; /* UNUSED (int id) */
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 6] = THREAD_INIT_DS;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 7] = THREAD_INIT_ES;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 8] = THREAD_INIT_FS;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 9] = THREAD_INIT_GS;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 10] = THREAD_INIT_SS;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 11] = THREAD_INIT_EAX;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 12] = THREAD_INIT_EBX;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 13] = THREAD_INIT_ECX;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 14] = THREAD_INIT_EDX;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 15] = THREAD_INIT_ESI;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 16] = THREAD_INIT_EDI;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 17] = idle_thread->ebp;
    idle_thread->kernel_stack[THREAD_STACK_SIZE - 18] =
        (uint32_t)&idle_thread->kernel_stack[THREAD_STACK_SIZE - 17];

    strncpy(idle_thread->name, "idle\0", 5);

    active_thread = idle_thread;
    active_thread_node = idle_thread_node;
    old_thread = active_thread;
    old_thread_node = active_thread_node;

    system_state = RUNNING;
    ++thread_count;

    #ifdef DEBUG_SCHED
    kernel_serial_debug("IDLE thread created\n");
    #endif

    second_idle_thread_node = kernel_list_create_node(idle_thread, &err);

    if(err != OS_NO_ERR || second_idle_thread_node == NULL)
    {
        kernel_error("Could not create second IDLE thread node\n");
        kernel_panic();
    }

    err = kernel_list_enlist_data(second_idle_thread_node, global_threads_table,
                                  idle_thread->priority);
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

    enable_local_interrupt();

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
    if(active_thread == idle_thread)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    active_thread->wakeup_time = get_current_uptime() + time_ms;
    active_thread->state = SLEEPING;

    #ifdef DEBUG_SCHED
    kernel_serial_debug("%d Thread %d asleep until %d\n", get_current_uptime(), active_thread->pid,
                        active_thread->wakeup_time );
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

uint32_t get_priority(void)
{
    return active_thread->priority;
}

OS_RETURN_E create_thread(thread_t* thread,
                          void* (*function)(void*),
                          const uint32_t priority,
                          const char *name,
                          void* args)
{
    OS_RETURN_E         err;
    kernel_thread_t*    new_thread;
    kernel_list_node_t* new_thread_node;
    kernel_list_node_t* seconde_new_thread_node;
    kernel_list_node_t* children_new_thread_node;

    if(thread != NULL)
    {
        *thread = NULL;
    }

    /* Check if priority is free */
    if(priority > KERNEL_LOWEST_PRIORITY)
    {
        return OS_ERR_FORBIDEN_PRIORITY;
    }

    disable_local_interrupt();

    new_thread = kmalloc(sizeof(kernel_thread_t));
    new_thread_node = kernel_list_create_node(new_thread, &err);

    if(err != OS_NO_ERR || new_thread == NULL)
    {
        if(new_thread != NULL)
        {
            kfree(new_thread);
        }

        if(err == OS_NO_ERR)
        {
            err = OS_ERR_MALLOC;
        }
        enable_local_interrupt();
        return err;
    }

    memset(new_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    new_thread->pid            = ++last_given_pid;
    new_thread->ppid           = active_thread->pid;
    new_thread->priority       = priority;
    new_thread->init_prio      = priority;
    new_thread->args           = args;
    new_thread->function       = function;
    new_thread->joining_thread = NULL;
    new_thread->state          = READY;
    new_thread->last_sched     = 0;

    new_thread->children = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        kernel_list_delete_node(&new_thread_node);
        kfree(new_thread);
        enable_local_interrupt();
        return err;
    }

     /* Init thread context */
    new_thread->eip = (uint32_t) thread_wrapper;
    new_thread->esp =
        (uint32_t) &new_thread->kernel_stack[THREAD_STACK_SIZE - 18];
    new_thread->ebp =
        (uint32_t) &new_thread->kernel_stack[THREAD_STACK_SIZE - 1];

    /* Init thread stack */
    new_thread->kernel_stack[THREAD_STACK_SIZE - 1] = THREAD_INIT_EFLAGS;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 2] = THREAD_INIT_CS;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 3] = new_thread->eip;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 4] = 0; /* UNUSED (error core) */
    new_thread->kernel_stack[THREAD_STACK_SIZE - 5] = 0; /* UNUSED (int id) */
    new_thread->kernel_stack[THREAD_STACK_SIZE - 6] = THREAD_INIT_DS;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 7] = THREAD_INIT_ES;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 8] = THREAD_INIT_FS;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 9] = THREAD_INIT_GS;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 10] = THREAD_INIT_SS;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 11] = THREAD_INIT_EAX;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 12] = THREAD_INIT_EBX;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 13] = THREAD_INIT_ECX;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 14] = THREAD_INIT_EDX;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 15] = THREAD_INIT_ESI;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 16] = THREAD_INIT_EDI;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 17] = new_thread->ebp;
    new_thread->kernel_stack[THREAD_STACK_SIZE - 18] =
        (uint32_t)&new_thread->kernel_stack[THREAD_STACK_SIZE - 17];

    strncpy(new_thread->name, name, THREAD_MAX_NAME_LENGTH);

    seconde_new_thread_node = kernel_list_create_node(new_thread, &err);
    if(err != OS_NO_ERR)
    {
        kernel_list_delete_list(&new_thread->children);
        kernel_list_delete_node(&new_thread_node);
        kfree(new_thread);
        enable_local_interrupt();
        return err;
    }

    children_new_thread_node = kernel_list_create_node(new_thread, &err);
    if(err != OS_NO_ERR)
    {
        kernel_list_delete_list(&new_thread->children);
        kernel_list_delete_node(&new_thread_node);
        kernel_list_delete_node(&seconde_new_thread_node);
        kfree(new_thread);
        enable_local_interrupt();
        return err;
    }

    err = kernel_list_enlist_data(new_thread_node, active_threads_table,
                                  priority);
    if(err != OS_NO_ERR)
    {
        kernel_list_delete_list(&new_thread->children);
        kernel_list_delete_node(&children_new_thread_node);
        kernel_list_delete_node(&new_thread_node);
        kernel_list_delete_node(&seconde_new_thread_node);
        kfree(new_thread);
        enable_local_interrupt();
        return err;
    }

     err = kernel_list_enlist_data(seconde_new_thread_node,
                                   global_threads_table,
                                   new_thread->priority);
     if(err != OS_NO_ERR)
     {
         kernel_list_delete_list(&new_thread->children);
         kernel_list_delete_node(&children_new_thread_node);
         kernel_list_delete_node(&new_thread_node);
         kernel_list_delete_node(&seconde_new_thread_node);
         kfree(new_thread);
         enable_local_interrupt();
         return err;
     }

    err = kernel_list_enlist_data(children_new_thread_node,
                                  active_thread->children, 0);
    if(err != OS_NO_ERR)
    {
        kernel_list_delete_list(&new_thread->children);
        kernel_list_delete_node(&children_new_thread_node);
        kernel_list_delete_node(&new_thread_node);
        kernel_list_delete_node(&seconde_new_thread_node);
        kfree(new_thread);
        enable_local_interrupt();
        return err;
    }

    ++thread_count;

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Created thread %d\n", new_thread->pid);
    #endif

    if(thread != NULL)
    {
        *thread = new_thread;
    }

    enable_local_interrupt();

    return OS_NO_ERR;
}

OS_RETURN_E wait_thread(thread_t thread, void** ret_val)
{
    if(thread == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Thread %d waiting for thread %d\n",
                         active_thread->pid,
                         thread->pid);
    #endif

    disable_local_interrupt();

    if(thread->state == DEAD)
    {
        enable_local_interrupt();
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

        clean_joined_thread(thread);

        enable_local_interrupt();

        return OS_NO_ERR;
    }

    /* Wait for the thread to finish */
    thread->joining_thread = active_thread_node;
    active_thread->state   = JOINING;

    enable_local_interrupt();

    /* Schedule thread */
    schedule();

    disable_local_interrupt();

    if(ret_val != NULL)
    {
        *ret_val = thread->ret_val;
    }

    clean_joined_thread(thread);

    enable_local_interrupt();

    return OS_NO_ERR;
}

kernel_list_node_t* lock_thread(const BLOCK_TYPE_E block_type)
{
    kernel_list_node_t* current_thread_node;
    /* Cant lock kernel thread */
    if(active_thread == idle_thread)
    {
        return NULL;
    }

    disable_local_interrupt();

    current_thread_node = active_thread_node;

    /* Lock the thread */
    active_thread->state      = BLOCKED;
    active_thread->block_type = block_type;

    enable_local_interrupt();

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Thread %d locked, reason: %d\n",
                        active_thread->pid,
                        block_type);
    #endif

    return current_thread_node;
}

OS_RETURN_E unlock_thread(kernel_list_node_t* node,
                          const BLOCK_TYPE_E block_type,
                          const uint8_t do_schedule)
{
    OS_RETURN_E err;
    kernel_thread_t* thread = (kernel_thread_t*)node->data;

    /* Check thread value */
    if(thread == NULL || thread == idle_thread)
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
    disable_local_interrupt();
    /* Unlock thread state */
    thread->state = READY;
    err = kernel_list_enlist_data(node, active_threads_table, thread->priority);
    enable_local_interrupt();

    if(err != OS_NO_ERR)
    {
        kernel_error("Could not enqueue thread in active table[%d]\n", err);
        kernel_panic();
    }

    #ifdef DEBUG_SCHED
    kernel_serial_debug("Thread %d unlocked, reason: %d\n",
                         thread->pid,
                         block_type);
    #endif

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
    if(active_thread == idle_thread)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    if(block_type == IO_KEYBOARD)
    {
        disable_local_interrupt();

        /* Lock current tread */
        active_thread->block_type = block_type;
        active_thread->state = BLOCKED;

        err = kernel_list_enlist_data(active_thread_node,
                                      io_waiting_threads_table,
                                      active_thread->io_req_time);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread in kbd IO table[%d]\n", err);
            kernel_panic();
        }

        enable_local_interrupt();


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
    kernel_thread_t*    thread;
    kernel_list_node_t* node;
    OS_RETURN_E         err;

    if(block_type != IO_KEYBOARD)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    disable_local_interrupt();

    node = kernel_list_delist_data(io_waiting_threads_table, &err);
    if(err != OS_NO_ERR || node == NULL)
    {
        kernel_error("Could not dequeing thread in kbd IO table[%d]\n", err);
        kernel_panic();
    }

    thread = node->data;
    if(thread != NULL && err == OS_NO_ERR &&
      thread->state == BLOCKED && thread->block_type == IO_KEYBOARD)
    {
        err = kernel_list_enlist_data(node,
                                      active_threads_table,
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

    enable_local_interrupt();

    return OS_NO_ERR;
}

OS_RETURN_E get_threads_info(thread_info_t* threads, int32_t* size)
{
    int32_t          i;
    kernel_list_node_t*  cursor;
    kernel_thread_t* cursor_thread;

    if(threads == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    if(size == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    disable_local_interrupt();

    if(*size > (int)thread_count)
    {
        *size = thread_count;
    }

    /* Walk the thread list and fill the structures */
    cursor = global_threads_table->head;
    cursor_thread = (kernel_thread_t*)cursor->data;
    for(i = 0; cursor != NULL && i < *size; ++i)
    {
        thread_info_t *current = &threads[i];
        current->pid = cursor_thread->pid;
        current->ppid = cursor_thread->ppid;
        strncpy(current->name, cursor_thread->name, THREAD_MAX_NAME_LENGTH);
        current->priority = cursor_thread->priority;
        current->state = cursor_thread->state;
        current->start_time = cursor_thread->start_time;
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
        cursor_thread = (kernel_thread_t*)cursor->data;
    }

    enable_local_interrupt();

    return OS_NO_ERR;
}
