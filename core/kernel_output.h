/*******************************************************************************
 *
 * File: kernel_output.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 15/12/2017
 *
 * Version: 1.0
 *
 * Simple output functions to print messages to screen. These are really basic
 * output too allow early kernel boot output and debug.
 ******************************************************************************/

#ifndef __KERNEL_OUTPUT_H_
#define __KERNEL_OUTPUT_H_

/* Generic int types */
#include "../lib/stdint.h"

/* Print the desired string to the screen.
 *
 * @param string The string to output.
 * @param size The size of the string to output.
 */
void kernel_print(const char *string, uint32_t size);

/* Print the desired string to the screen. Add a red [ERROR] tag at the 
 * beggining of the string before printing it.
 *
 * @param string The string to output.
 * @param size The size of the string to output.
 */
void kernel_error(const char *string, uint32_t size);

/* Print the desired string to the screen. Add a green [OK] tag at the 
 * beggining of the string before printing it.
 *
 * @param string The string to output.
 * @param size The size of the string to output.
 */
void kernel_success(const char *string, uint32_t size);

/* Print the 32 bits variable in hexadecimal format with the 0x prefix.
 *
 * @param value The variable to output.
 */
void print_unsigned(const uint32_t value);

/* Print the 64 bits variable in hexadecimal format with the 0x prefix.
 *
 * @param value The variable to output.
 */
void print_unsigned_64(const uint64_t value);

#endif /* __KERNEL_OUTPUT_H_ */
