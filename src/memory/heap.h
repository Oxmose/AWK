/*******************************************************************************
 *
 * File: heap.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 10/01/2017
 *
 * Version: 1.0
 *
 * Kernel heap allocators
 ******************************************************************************/

#ifndef __HEAP_H_
#define __HEAP_H_

#include "../lib/stdint.h" /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
* STRUCTURES
******************************************************************************/

typedef struct list
{
    struct list* next;
    struct list* prev;
} list_t;


typedef struct mem_chunk
{
    list_t all;
    int8_t used;
    union
    {
	       uint8_t  data[0];
	       list_t   free;
    };
} mem_chunk_t;

enum heap_enum
{
    NUM_SIZES   = 32,
    ALIGN       = 4,
    MIN_SIZE    = sizeof(list_t),
    HEADER_SIZE = __builtin_offsetof(mem_chunk_t, data),
};

/*******************************************************************************
* FUNCTIONS
******************************************************************************/

/* Setup kernel heap management at start. It will also allign kernel heap start
 * and initialize the basic heap paramters such as its size.
 */
void setup_kheap(void);

/* Allocate size bytes of memory in the kernel heap.
 *
 * ­@param size The number of byte to allocate.
 * @return A pointer to the staqrt address of the allocated memory. If the
 * memory cannot be allocated, this pointer will be NULL.
 */
void* kmalloc(uint32_t size);

/* Release allocated memory. Of the pointer is NULL or has not been allocated
 * previously, nothing is done.
 *
 * @param ptr The pointer of the begining of the memory area to free.
 */
void kfree(void* ptr);

#endif /* __HEAP_H_ */
