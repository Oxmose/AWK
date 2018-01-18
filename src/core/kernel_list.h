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

    uint16_t priority;      /* Priority of the element */
    uint16_t enlisted;      /* Is the node in a list */

    void* data;             /* Data contained by the node */
} kernel_list_node_t;

/* List structure */
typedef struct kernel_list
{
    struct kernel_list_node* head; /* Head of the list */
    struct kernel_list_node* tail; /* Tail of the list */

    uint32_t size;         /* List size */
} kernel_list_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Create a node ready to be inserted in a list. The data can be modified later
 * by accessing the data field of the node structure.
 *
 * WARNING A node should be only used in ONE list at most !!!
 *
 * @param data The pointer to the data to carry in the node.
 * @param error A pointer to the variable that contains the function success
 * state. May be NULL.
 * @returns The node pointer is returned.
 */
kernel_list_node_t* kernel_list_create_node(void* data, OS_RETURN_E *error);

/* Delete a node from the memory. The node should not be used in any list. If it
 * is the case, the function will return an error.
 *
 * @param node The node pointer of pointer to destroy.
 * @returns The function returns OS_NO_ERR on success, see system returns type
 * for further error description.
 */
OS_RETURN_E kernel_list_delete_node(kernel_list_node_t** node);

/* Create an empty list ready to be used.
 *
 * @param error A pointer to the variable that contains the function success
 * state. May be NULL.
 * @returns The list pointer is returned.
 */
kernel_list_t* kernel_list_create_list(OS_RETURN_E *error);

/* Delete a list from the memory.If the list is not empty, an error is returned.
 *
 * @param list The list pointer of pointer to destroy.
 * @returns The function returns OS_NO_ERR on success, see system returns type
 * for further error description.
 */
OS_RETURN_E kernel_list_delete_list(kernel_list_t** list);

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
                                    const uint16_t priority);

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

/* Find a node containing the data given as paramter. An error is set if not any
 * node is found.
 *
 * @param list The list to manage.
 * @param data The data contained by the node to find.
 * @param error A pointer to the variable that contains the function success
 * state. May be NULL.
 * @returns The function returns a pointer to the node if found, NULL otherwise.
 */
kernel_list_node_t* kernel_list_find_node(kernel_list_t* list, void* data,
                                          OS_RETURN_E *error);

/* Remove a node from a list given as parameter. If the node is not found,
 * nothing is done and an error is returned.
 *
 * @param list The list containing the node.
 * @param node The node to remove.
 * @returns The function returns OS_NO_ERR on success, see system returns type
 * for further error description.
 */
OS_RETURN_E kernel_list_remove_node_from(kernel_list_t* list,
                                         kernel_list_node_t* node);


#endif /* __KERNEL_LIST_H_ */
