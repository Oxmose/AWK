/*******************************************************************************
 *
 * File: serial.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 25/12/2017
 *
 * Version: 1.0
 *
 * Serial for the kernel.
 ******************************************************************************/

#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../lib/stdint.h" /* Generic int types */
#include "../cpu/cpu.h"    /* outb, inb */

/* Header file */
#include "serial.h"

static uint8_t serial_init = 0;

static OS_RETURN_E set_line(uint8_t attr, uint32_t com)
{
	uint8_t parity = attr & 0x38;
	if(parity != SERIAL_PARITY_NONE &&
	   parity != SERIAL_PARITY_ODD &&
	   parity != SERIAL_PARITY_EVEN &&
	   parity != SERIAL_PARITY_MARK &&
	   parity != SERIAL_PARITY_SPACE)

	{
		return OS_ERR_NO_SUCH_SERIAL_PARITY;
	}

    outb(attr, SERIAL_LINE_COMMAND_PORT(com));

    return OS_NO_ERR;
}

static OS_RETURN_E set_buffer(uint8_t attr, uint32_t com)
{
	outb(attr, SERIAL_FIFO_COMMAND_PORT(com));

    return OS_NO_ERR;
}

static OS_RETURN_E set_baudrate(SERIAL_BAUDRATE_E rate, uint32_t com)
{
	outb(SERIAL_DLAB_ENABLED, SERIAL_LINE_COMMAND_PORT(com));
    outb((rate >> 8) & 0x00FF, SERIAL_DATA_PORT(com));
    outb(rate & 0x00FF, SERIAL_DATA_PORT_2(com));

    return OS_NO_ERR;
}

OS_RETURN_E init_serial(void)
{
	OS_RETURN_E err;
	uint8_t attr;
	uint8_t i;
	uint32_t com;

	/* Init all comm ports */
	for(i = 0; i < 4; ++i)
	{
		if(i == 0)
		{
			com = SERIAL_COM1_BASE;
		}
		else if(i == 1)
		{
			com = SERIAL_COM2_BASE;
		}
		else if(i == 2)
		{
			com = SERIAL_COM3_BASE;
		}
		else if(i == 3)
		{
			com = SERIAL_COM4_BASE;
		}

		attr = SERIAL_DATA_LENGTH_8 | SERIAL_STOP_BIT_1;

		/* Disable interrupt */
		outb(0x00, SERIAL_DATA_PORT_2(com));    // Disable all interrupts

		/* Init baud rate */
		err = set_baudrate(BAUDRATE_38400, com);
		if(err != OS_NO_ERR)
		{
			return err;
		}

	    /* Configure the line */
	    err = set_line(attr, com);
	    if(err != OS_NO_ERR)
		{
			return err;
		}

		err = set_buffer(0xC0 | SERIAL_ENABLE_FIFO | SERIAL_CLEAR_RECV_FIFO | 
			             SERIAL_CLEAR_SEND_FIFO | SERIAL_FIFO_DEPTH_14, 
			             com);
		if(err != OS_NO_ERR)
		{
			return err;
		}

		outb(0x0B, SERIAL_MODEM_COMMAND_PORT(com));
		/* TODO SET IRQ */
	}

	serial_init = 1;

    return err;
}

void serial_write(uint32_t port, const char a)
{
	if(serial_init == 0)
	{
		return;
	}
	if(port != COM1 && port != COM2 && port != COM3 && port != COM4)
	{
		return;
	}

	/* Wait for empty transmit */
	while((SERIAL_LINE_STATUS_PORT(port) & 0x20) == 0)
	{}

	if(a == '\n')
	{
		serial_write(port, '\r');
		outb('\n', port); 
	}
	else
	{
		outb(a, port); 
	}

   	while((SERIAL_LINE_STATUS_PORT(port) & 0x20) == 0)
   	{}
}