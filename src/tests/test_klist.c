/*******************************************************************************
 *
 * File: test_klist.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: Kernel list tests
 ******************************************************************************/

#include "../core/kernel_list.h"
#include "../core/kernel_output.h"
#include "../core/panic.h"
#include "../lib/stddef.h"

void test_klist(void)
{
    OS_RETURN_E error = OS_ERR_NULL_POINTER;
    kernel_list_node_t* nodes[40] = { NULL };
    kernel_list_t*      list = NULL;
    uint32_t   sorted[40];
    uint32_t   unsorted[10] = {0, 3, 5, 7, 4, 1, 8, 9, 6, 2};

    int8_t j = -1;
    for(uint32_t i = 0; i < 40; ++i)
    {
        if(i % 4 == 0)
        {
            ++j;
        }
        sorted[i] = j;
    }

    /* Create node */
    nodes[0] = kernel_list_create_node((void*) 0, &error);
    if(nodes[0] == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KLIST 0\n");
        kernel_panic();
    }

    /* Delete node */
    error = kernel_list_delete_node(&nodes[0]);
    if(nodes[0] != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KLIST 1\n");
        kernel_panic();
    }

    error = OS_ERR_NULL_POINTER;

    /* Create node */
    nodes[0] = kernel_list_create_node((void*) 0, &error);
    if(nodes[0] == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KLIST 2\n");
        kernel_panic();
    }

    error = OS_ERR_NULL_POINTER;

    /* Create list */
    list = kernel_list_create_list(&error);
    if(list == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KLIST 3\n");
        kernel_panic();
    }

    /* Delete list */
    error = kernel_list_delete_list(&list);
    if(list != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KLIST 4\n");
        kernel_panic();
    }

    error = OS_ERR_NULL_POINTER;

    /* Create list */
    list = kernel_list_create_list(&error);
    if(list == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KLIST 5\n");
        kernel_panic();
    }

    /* Enlist node */
    error = kernel_list_enlist_data(nodes[0], list, 0);
    if(error != OS_NO_ERR)
    {
        kernel_error("TEST_KLIST 6\n");
        kernel_panic();
    }

    /* Delete node */
    error = kernel_list_delete_node(&nodes[0]);
    if(nodes[0] == NULL || error != OS_ERR_UNAUTHORIZED_ACTION)
    {
        kernel_error("TEST_KLIST 7 %d %d\n", nodes[0], error);
        kernel_panic();
    }

    /* Enlist NULL node */
    error = kernel_list_enlist_data(NULL, list, 0);
    if(error != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_KLIST 8\n");
        kernel_panic();
    }

    /* Delete list */
    error = kernel_list_delete_list(&list);
    if(list == NULL || error != OS_ERR_UNAUTHORIZED_ACTION)
    {
        kernel_error("TEST_KLIST 9\n");
        kernel_panic();
    }

    error = OS_ERR_NULL_POINTER;

    /* Delist node */
    nodes[0] = kernel_list_delist_data(list, &error);
    if(nodes[0] == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KLIST 10\n");
        kernel_panic();
    }

    error = OS_ERR_NULL_POINTER;

    /* Create more nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = kernel_list_create_node((void*) unsorted[i % 10], &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KLIST 11\n");
            kernel_panic();
        }

        error = OS_ERR_NULL_POINTER;
    }

    /* Enlist nodes with prio */
    for(uint8_t i = 0; i < 40; ++i)
    {
        error = kernel_list_enlist_data(nodes[i], list, (uint32_t)nodes[i]->data);
        if(error != OS_NO_ERR)
        {
            kernel_error("TEST_KLIST 12\n");
            kernel_panic();
        }
    }

    error = OS_ERR_NULL_POINTER;

    /* Delist nodes and check order */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = kernel_list_delist_data(list, &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KLIST 14\n");
            kernel_panic();
        }
        if((uint32_t)nodes[i]->data != sorted[i])
        {
            kernel_error("TEST_KLIST 15\n");
            kernel_panic();
        }

        error = OS_ERR_NULL_POINTER;
    }


    if(list->size != 0)
    {
        kernel_error("TEST_KLIST 16\n");
        kernel_panic();
    }

    /* Delete nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        error = kernel_list_delete_node(&nodes[i]);
        if(nodes[i] != NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KLIST 17\n");
            kernel_panic();
        }
    }

    /* Create more nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = kernel_list_create_node((void*) unsorted[i % 10], &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KLIST 18\n");
            kernel_panic();
        }

        error = OS_ERR_NULL_POINTER;
    }

    /* Enlist without prio */
    for(uint8_t i = 0; i < 40; ++i)
    {
        error = kernel_list_enlist_data(nodes[i], list, 0);
        if(error != OS_NO_ERR)
        {
            kernel_error("TEST_KLIST 19\n");
            kernel_panic();
        }
    }


    error = OS_ERR_NULL_POINTER;

    /* Find a present node */
    kernel_list_node_t* find = NULL;
    find =  kernel_list_find_node(list, (void*) 9, &error);
    if(find == NULL || error != OS_NO_ERR || find->data != (void*) 9)
    {
        kernel_error("TEST_KLIST 20\n");
        kernel_panic();
    }

    error = OS_ERR_NULL_POINTER;

    /* Find a not present node */
    find = NULL;
    find =  kernel_list_find_node(list, (void*) 42, &error);
    if(find != NULL || error != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("TEST_KLIST 21\n");
        kernel_panic();
    }

    error = OS_ERR_NULL_POINTER;

    /* Delist nodes and check "non order" */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = kernel_list_delist_data(list, &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KLIST 22\n");
            kernel_panic();
        }
        if((uint32_t)nodes[i]->data != unsorted[i%10])
        {
            kernel_error("TEST_KLIST 23\n");
            kernel_panic();
        }

        error = OS_ERR_NULL_POINTER;
    }

    if(list->size != 0)
    {
        kernel_error("TEST_KLIST 24\n");
        kernel_panic();
    }

    /* Delist node on empty list */
    find = kernel_list_delist_data(list, &error);
    if(find != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KLIST 25\n");
        kernel_panic();
    }

    /* Delete list */
    error = kernel_list_delete_list(&list);
    if(list != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KLIST 26\n");
        kernel_panic();
    }

    /* Enlist node on NULL list */
    error = kernel_list_enlist_data(nodes[0], list, 0);
    if(error != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_KLIST 27\n");
        kernel_panic();
    }

    /* Delist node on NULL list */
    find = kernel_list_delist_data(list, &error);
    if(find != NULL || error != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_KLIST 28\n");
        kernel_panic();
    }

    /* Delete nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        error = kernel_list_delete_node(&nodes[i]);
        if(nodes[i] != NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KLIST 29\n");
            kernel_panic();
        }
    }

    kernel_debug("Kernel lists tests passed\n");
}
