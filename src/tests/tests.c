#include "tests.h"

#include "test_sem.h"
#include "test_mutex.h"
#include "test_multithread.h"
#include "test_mouse.h"
#include "test_dyn_sched.h"
#include "test_ata.h"

#include "../lib/stdio.h"

#include "../sync/lock.h"
#include "../core/scheduler.h"

#ifdef TESTS
static const int32_t tests_count = 5;
#endif

/***************
 TEST MUST BE EXECUTED ON THE LOWEST PRIORITY POSSIBLE
 ****************/

void *launch_tests(void*args)
{
    //test_mouse(NULL);

    //while(1);

    (void)args;
#ifdef TESTS
#ifdef TEST_SEM
    printf("1/%d\n", tests_count);
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
    printf("2/%d\n", tests_count);
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
    printf("3/%d\n", tests_count);
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
#ifdef TEST_DYN_SCHED
    printf("4/%d\n", tests_count);
    if(test_dyn_sched())
    {
        kernel_error(" Test dyn sched failed\n");
    }
    else
    {
        kernel_success(" Test dyn sched passed\n");
    }
#endif
    printf("\n");
#ifdef TEST_ATA
    printf("5/%d\n", tests_count);
    if(test_ata())
    {
        kernel_error(" Test ATA failed\n");
    }
    else
    {
        kernel_success(" Test ATA passed\n");
    }
#endif
#endif
    return NULL;
}