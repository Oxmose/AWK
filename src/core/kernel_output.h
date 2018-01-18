/*******************************************************************************
 *
 * File: kernel_output.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 15/12/2017
 *
 * Version: 2.0
 *
 * Simple output functions to print messages to screen. These are really basic
 * output too allow early kernel boot output and debug.
 ******************************************************************************/

#ifndef __KERNEL_OUTPUT_H_
#define __KERNEL_OUTPUT_H_

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef struct output
{
	void (*putc)(const char);
	void (*puts)(const char*);
} output_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Print the desired string to the screen. Add a [SYS] tag at the
 * beggining of the string before printing it.
 *
 * @param __format The format string to output.
 */
void kernel_printf(const char *__format, ...);

/* Print the desired string to the screen. Add a red [ERROR] tag at the
 * beggining of the string before printing it.
 *
 * @param __format The format string to output.
 */
void kernel_error(const char *__format, ...);

/* Print the desired string to the screen. Add a green [OK] tag at the
 * beggining of the string before printing it.
 *
 * @param __format The format string to output.
 */
void kernel_success(const char *__format, ...);

/* Print the desired string to the screen. Add a cyan [INFO] tag at the
 * beggining of the string before printing it.
 *
 * @param __format The format string to output.
 */
void kernel_info(const char *__format, ...);

/* Print the desired string to the screen. Add a yellow [DEBUG] tag at the
 * beggining of the string before printing it.
 *
 * @param __format The format string to output.
 */
void kernel_debug(const char *__format, ...);

/* Print the desired string to the serial port. Add a [DEBUG] tag at the
 * beggining of the string before printing it.
 *
 * @param __format The format string to output.
 */
void kernel_serial_debug(const char *__format, ...);

/* Print the desired string to the screen with the argument list given as
 * parameter, may ber used by printf.
 *
 * @param str The format string to output.
 * @param args The arguments list.
 */
void kernel_doprint(const char* str, __builtin_va_list args);

#endif /* __KERNEL_OUTPUT_H_ */
