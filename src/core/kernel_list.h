/*******************************************************************************
 *
 * File: kernel_list.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.5
 *
 * Kernel priority lists used to manage data.
 ******************************************************************************/

#ifndef __KERNEL_LIST_H_
#define __KERNEL_LIST_H_

#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../lib/stdint.h" /* Generic int types */
#include "../sync/lock.h"  /* spinlock */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* List node structure */
typedef struct kernel_list_node
{
    struct kernel_list_node* next; /* Next node of the list */
    struct kernel_list_node* prev; /* Previous node of the list */

    uint32_t priority;      /* Priority of the element */

    void* data;             /* Data contained by the node */
} kernel_list_node_t;

/* List structure */
typedef struct kernel_list
{
    struct kernel_list_node* head; /* Head of the list */
    struct kernel_list_node* tail; /* Tail of the list */

    lock_t lock;            /* List lock */
} kernel_list_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

kernel_list_node_t* kernel_list_create_node(void* data, OS_RETURN_E *error);

OS_RETURN_E kernel_list_delete_node(kernel_list_node_t* node);

kernel_list_t* kernel_list_create_list(OS_RETURN_E *error);

OS_RETURN_E kernel_list_delete_list(kernel_list_t* list);

/* Enlist a node in the list given as parameter. The data will be placed
 * in the list accordinlgy to the priority defined.
 *
 * @param node A now node to add in the list.
 * @param list The list to manage.
 * @param priority The priority of the element to place in the list.
 * @returns The function returns OS_NO_ERR on success, see system returns type
 * for further error description.
 */
OS_RETURN_E kernel_list_enlist_data(kernel_list_node_t* node,
                                    kernel_list_t* list,
                                    const uint32_t priority);

/* Remove a node from the list given as parameter. The retreived node that is
 * returned is the one with the highest priority parameter.
 *
 * @param list The list to manage.
 * @param error A pointer to the variable that contains the function success
 * state. May be NULL.
 * @returns The data poitner with the highest priority parameter is returned.
 */
kernel_list_node_t* kernel_list_delist_data(kernel_list_t* list,
                                            OS_RETURN_E* error);

/* Rmove a node containing the pointed data from the list given as parameter.
 * The node is also deleted from the memory, you should not use
 * kernel_list_detele_node.
 *
 * @param list The list to manage.
 * @param data The pointer contained by the node to remove.
 * @returns The function returns OS_NO_ERR on success, see system returns type
 * for further error description.
 */
OS_RETURN_E kernel_list_remove_data(kernel_list_t* list,
                                    void* data);

#endif /* __KERNEL_LIST_H_ */
