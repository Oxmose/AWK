/*******************************************************************************
 *
 * File: tests.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank
 ******************************************************************************/

#ifndef __TESTS_H_
#define __TESTS_H_

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define TESTS 1

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

extern void test_sw_interupts(void);
extern void test_pic(void);
extern void test_pit(void);
extern void test_rtc(void);

 #endif /* __TESTS_H_ */
