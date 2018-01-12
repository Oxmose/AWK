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

void setup_kheap(void);
void* kmalloc(uint32_t size);
void kfree(void* ptr);

#endif /* __HEAP_H_ */
