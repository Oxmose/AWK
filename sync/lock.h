/*******************************************************************************
 *
 * File: lock.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * Basic lock and synchronization primitives
 ******************************************************************************/

#ifndef __LOCK_H_
#define __LOCK_H_

/* Enable CPU interrupt */
void enable_interrupt(void);

/* Disable CPU interrupt */
void disable_interrupt(void);

#endif /* __LOCK_H_ */