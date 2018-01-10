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
 *
 * !!! WHEN TESTS ARE ENABLED PLEASE DONT USE KEYBOARD OR MOUSE BEFORE THE !!!
 * !!! KERNEL HAS FINISHED ITS INITIALIZATION, THE IRQs ARE NOT HANDLED    !!!
 * !!! PROPERLY DURING THE TEST PHASE                                      !!!
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
extern void test_io_apic(void);
extern void test_pit(void);
extern void test_lapic(void);
extern void test_lapic_timer(void);
extern void test_rtc(void);
extern void test_keyboard(void);
extern void test_mouse(void);

 #endif /* __TESTS_H_ */
