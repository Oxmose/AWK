/*******************************************************************************
 *
 * File: doprnt.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/10/2017
 *
 * Version: 1.0
 *
 * doprnt function used by printf and derivate to printf data,
 *
 ******************************************************************************/

#ifndef __DOPRNT_H_
#define __DOPRNT_H_

void _doprnt(const char *fmt, __builtin_va_list args, int radix,
             void (*putc)(), char *putc_arg);

#endif /* __DOPRNT_H_ */
