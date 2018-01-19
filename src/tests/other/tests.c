#define TEST_MUTEX
#define TEST_SEM
#define TEST_MULTITHREAD
#define TEST_PAYLOAD

#define TESTS 1

#include "test_sem.h"
#include "test_mutex.h"
#include "test_multithread.h"
#include "test_mouse.h"
#include "test_dyn_sched.h"
#include "test_mouse.h"

#include "../../lib/stdio.h"
#include "../../sync/lock.h"
#include "../../core/scheduler.h"
#include "../../core/kernel_output.h"

#ifdef TESTS
static const int32_t tests_count = 4;
#endif

/***************
 TEST MUST BE EXECUTED ON THE LOWEST PRIORITY POSSIBLE
 ****************/

 void* th(void*args)
 {
 	printf("HI %d ", (int)args);
 	return NULL;
 }


void *launch_tests(void*args)
{

    test_mouse2(NULL);

    //while(1);

    (void)args;
#ifdef TESTS

#ifdef TEST_PAYLOAD
    printf("1/%d\n", tests_count);

    thread_t test_ths[200];


    for(int i = 0; i < 200; ++i)
    {
        create_thread(&test_ths[i], th, 0, "tests\0", (void*)i);
    }

    for(int i = 0; i < 200; ++i)
    {
        wait_thread(test_ths[i], NULL);
    }

    kernel_success(" Test payload passed\n");
#endif

#ifdef TEST_SEM
    printf("2/%d\n", tests_count);
    if(test_sem())
    {
        kernel_error(" Test semaphores failed\n");
    }
    else
    {

        kernel_success(" Test semaphores passed\n");
    }
#endif
    printf("\n");
#ifdef TEST_MUTEX
    printf("3/%d\n", tests_count);
    if(test_mutex())
    {
        kernel_error(" Test mutex failed\n");
    }
    else
    {
        kernel_success(" Test mutex passed\n");
    }
#endif
    printf("\n");
#ifdef TEST_MULTITHREAD
    printf("4/%d\n", tests_count);
    if(test_multithread())
    {
        kernel_error(" Test multithread failed\n");
    }
    else
    {
        kernel_success(" Test multithread passed\n");
    }
#endif
    printf("\n");

#endif
    return NULL;
}
