/*******************************************************************************
 *
 * File: stdint.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/10/2017
 *
 * Version: 1.0
 *
 * Define basics int types for the kernel
 *
 ******************************************************************************/

#ifndef __STDINT_H_
#define __STDINT_H_

/* Exact-width integer types */
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned           uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

/* Minimum-width integer types */
typedef signed char        int_least8_t;
typedef unsigned char      uint_least8_t;
typedef short              int_least16_t;
typedef unsigned short     uint_least16_t;
typedef int                int_least32_t;
typedef unsigned           uint_least32_t;
typedef signed long long   int_least64_t;
typedef unsigned long long uint_least64_t;

/*  Fastest minimum-width integer types
 *  Not actually guaranteed to be fastest for all purposes
 *  Here we use the exact-width types for 8 and 16-bit ints.
 */
typedef char               int_fast8_t;
typedef unsigned char      uint_fast8_t;
typedef short              int_fast16_t;
typedef unsigned short     uint_fast16_t;
typedef int                int_fast32_t;
typedef unsigned int       uint_fast32_t;
typedef signed long long   int_fast64_t;
typedef unsigned long long uint_fast64_t;

/* Limits of exact-width integer types */
#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN  (-9223372036854775807LL - 1)

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL

#define UINT8_MAX  0xff                       
#define UINT16_MAX 0xffff
#define UINT32_MAX 0xffffffff
#define UINT64_MAX 0xffffffffffffffffULL

#endif /* __STDINT_H_ */