/*******************************************************************************
 *
 * File: serial.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 25/12/2017
 *
 * Version: 1.0
 *
 * Serial driver for the kernel.
 ******************************************************************************/

#ifndef __SERIAL_H_
#define __SERIAL_H_

#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../lib/stdint.h" /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define SERIAL_COM1_BASE                0x3F8
#define SERIAL_COM2_BASE                0x2F8
#define SERIAL_COM3_BASE                0x3E8
#define SERIAL_COM4_BASE                0x2E8

#define COM1 SERIAL_COM1_BASE
#define COM2 SERIAL_COM2_BASE
#define COM3 SERIAL_COM3_BASE
#define COM4 SERIAL_COM4_BASE

#define SERIAL_DEBUG_PORT COM1

#define SERIAL_DATA_PORT(port)          (port)
#define SERIAL_DATA_PORT_2(port)        (port + 1)
#define SERIAL_FIFO_COMMAND_PORT(port)  (port + 2)
#define SERIAL_LINE_COMMAND_PORT(port)  (port + 3)
#define SERIAL_MODEM_COMMAND_PORT(port) (port + 4)
#define SERIAL_LINE_STATUS_PORT(port)   (port + 5)

#define SERIAL_DATA_LENGTH_5 0x00
#define SERIAL_DATA_LENGTH_6 0x01
#define SERIAL_DATA_LENGTH_7 0x02
#define SERIAL_DATA_LENGTH_8 0x03

#define SERIAL_STOP_BIT_1   0x00
#define SERIAL_STOP_BIT_2   0x04

#define SERIAL_PARITY_NONE  0x00
#define SERIAL_PARITY_ODD   0x01
#define SERIAL_PARITY_EVEN  0x03
#define SERIAL_PARITY_MARK  0x05
#define SERIAL_PARITY_SPACE 0x07

#define SERIAL_BREAK_CTRL_ENABLED  0x40
#define SERIAL_BREAK_CTRL_DISABLED 0x00

#define SERIAL_DLAB_ENABLED  0x80
#define SERIAL_DLAB_DISABLED 0x00

#define SERIAL_ENABLE_FIFO       0x01
#define SERIAL_CLEAR_RECV_FIFO   0x02
#define SERIAL_CLEAR_SEND_FIFO   0x04
#define SERIAL_DMA_ACCESSED_FIFO 0x08

#define SERIAL_FIFO_DEPTH_14     0x00
#define SERIAL_FIFO_DEPTH_64     0x10

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef enum SERIAL_BAUDRATE
{
    BAURDATE_50     = 2304,
    BAUDRATE_75     = 1536,
    BAUDRATE_150    = 768,
    BAUDRATE_300    = 384,
    BAUDRATE_600    = 192,
    BAUDRATE_1200   = 96,
    BAUDRATE_1800   = 64,
    BAUDRATE_2400   = 48,
    BAUDRATE_4800   = 24,
    BAUDRATE_7200   = 16,
    BAUDRATE_9600   = 12,
    BAUDRATE_14400  = 8,
    BAUDRATE_19200  = 6,
    BAUDRATE_38400  = 3,
    BAUDRATE_57600  = 2,
    BAUDRATE_115200 = 1,
} SERIAL_BAUDRATE_E;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init serial driver structures and hardware.
 *
 * @returns OS_NO_ERR if successfull. Otherwise an error is returned.
 */
OS_RETURN_E init_serial(void);

/* Write the data given as patameter on the desired port.
 *
 * @param port The desired port to write the data to.
 * @param data The byte to write to the serial port.
 */
void serial_write(const uint32_t port, const uint8_t data);

/* Write the string given as patameter on the debug port.
 *
 * @param str The string to write to the debug port.
 */
void serial_put_string(const char* string);

/* Write the char given as patameter on the debug port.
 *
 * @param character The cahracter to write to the debug port.
 */
void serial_put_char(const char character);

#endif /* __SERIAL_H_ */
