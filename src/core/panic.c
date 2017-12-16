/*******************************************************************************
 *
 * File: panic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 1.0
 *
 * Kernel panic functions. Displays the CPU registers, the faulty instruction
 * and the interrupt ID and cause.
 ******************************************************************************/

#include "kernel_output.h" /* kernel_print, kernel_print_unsigned_hex */
#include "interrupts.h"    /* cpu_state_t, stack_state_t */
#include "../lib/stdint.h" /* Generic int types */
#include "../cpu/cpu.h"    /* hlt */

/* Header file */
#include "panic.h"

void panic(cpu_state_t *cpu_state, uint32_t int_id, stack_state_t *stack_state)
{
	/* Get EFLAGS one by one */
	int8_t cf_f = (stack_state->eflags & 0x1);
	int8_t pf_f = (stack_state->eflags & 0x4) >> 2;
	int8_t af_f = (stack_state->eflags & 0x10) >> 4;
	int8_t zf_f = (stack_state->eflags & 0x40) >> 6;
	int8_t sf_f = (stack_state->eflags & 0x80) >> 7;
	int8_t tf_f = (stack_state->eflags & 0x100) >> 8;
	int8_t if_f = (stack_state->eflags & 0x200) >> 9;
	int8_t df_f = (stack_state->eflags & 0x400) >> 10;
	int8_t of_f = (stack_state->eflags & 0x800) >> 11;
	int8_t nt_f = (stack_state->eflags & 0x4000) >> 14;
	int8_t rf_f = (stack_state->eflags & 0x10000) >> 16;
	int8_t vm_f = (stack_state->eflags & 0x20000) >> 17;
	int8_t ac_f = (stack_state->eflags & 0x40000) >> 18;
	int8_t id_f = (stack_state->eflags & 0x200000) >> 21;
	int8_t iopl0_f = (stack_state->eflags & 0x1000) >> 12;
	int8_t iopl1_f = (stack_state->eflags & 0x2000) >> 13;
	int8_t vif_f = (stack_state->eflags & 0x8000) >> 19;
	int8_t vip_f = (stack_state->eflags & 0x100000) >> 20;


	kernel_print("#=============================      OS PANIC      =============================#", 80);
	kernel_print("|                                                                              |", 80);
	/***************************************************************************************************/
	kernel_print("| Reason: ", 10);
	switch(int_id)
	{
		case 0:
			kernel_print("Division by zero                        ", 40);
			break;
		default:
			kernel_print("Unknown                                 ", 40);
	}
	kernel_print("INT ID: ", 8);
	kernel_print_unsigned_hex(int_id, 2);
	kernel_print("                 |", 18);
	/***************************************************************************************************/
	kernel_print("|                                                                              |", 80);
	kernel_print("| Instruction: ", 15);
	kernel_print_unsigned_hex(stack_state->eip, 8);
	kernel_print("                                                      |", 55);
	kernel_print("|                                                                              |", 80);
	/***************************************************************************************************/
	kernel_print("|================================= CPU STATE ==================================|", 80);
	kernel_print("|                                                                              |", 80);
	kernel_print("| EAX: ", 7);
	kernel_print_unsigned_hex(cpu_state->eax, 8);
	kernel_print("  |  EBX: ", 10);
	kernel_print_unsigned_hex(cpu_state->ebx, 8);
	kernel_print("  |  ECX: ", 10);
	kernel_print_unsigned_hex(cpu_state->ecx, 8);
	kernel_print("  |  EDX: ", 10);
	kernel_print_unsigned_hex(cpu_state->edx, 8);
	kernel_print("  |", 3);
	/***************************************************************************************************/
	kernel_print("| ESI: ", 7);
	kernel_print_unsigned_hex(cpu_state->esi, 8);
	kernel_print("  |  EDI: ", 10);
	kernel_print_unsigned_hex(cpu_state->edi, 8);
	kernel_print("  |  EBP: ", 10);
	kernel_print_unsigned_hex(cpu_state->ebp, 8);
	kernel_print("  |  ESP: ", 10);
	kernel_print_unsigned_hex(cpu_state->esp, 8);
	kernel_print("  |", 3);
	kernel_print("|                                                                              |", 80);
	/***************************************************************************************************/
	kernel_print("|============================= SEGMENT REGISTERS ==============================|", 80);
	kernel_print("|                                                                              |", 80);
	kernel_print("| CS:  ", 7);
	kernel_print_unsigned_hex(stack_state->cs, 8);
	kernel_print("  |  DS:  ", 10);
	kernel_print_unsigned_hex(cpu_state->ds, 8);
	kernel_print("  |  SS:  ", 10);
	kernel_print_unsigned_hex(cpu_state->ss, 8);
	kernel_print("                      |", 23);
	kernel_print("| ES:  ", 7);
	kernel_print_unsigned_hex(cpu_state->es, 8);
	kernel_print("  |  FS:  ", 10);
	kernel_print_unsigned_hex(cpu_state->fs, 8);
	kernel_print("  |  GS:  ", 10);
	kernel_print_unsigned_hex(cpu_state->gs, 8);
	kernel_print("                      |", 23);
	kernel_print("|                                                                              |", 80);
	/***************************************************************************************************/
	kernel_print("|================================= EFLAGS REG =================================|", 80);
	kernel_print("|                                                                              |", 80);
    kernel_print("| CF: ", 6);
    kernel_print_unsigned_hex(cf_f, 1);
    kernel_print("  |  PF: ", 9);
    kernel_print_unsigned_hex(pf_f, 1);
    kernel_print("  |  AF: ", 9);
    kernel_print_unsigned_hex(af_f, 1);
    kernel_print("  |  ZF: ", 9);
    kernel_print_unsigned_hex(zf_f, 1);
    kernel_print("  |  SF: ", 9);
    kernel_print_unsigned_hex(sf_f, 1);
    kernel_print("  |  TF: ", 9);
    kernel_print_unsigned_hex(tf_f, 1);
    kernel_print("          |", 11);
    /***************************************************************************************************/
    kernel_print("| IF: ", 6);
    kernel_print_unsigned_hex(if_f, 1);
    kernel_print("  |  DF: ", 9);
    kernel_print_unsigned_hex(df_f, 1);
    kernel_print("  |  OF: ", 9);
    kernel_print_unsigned_hex(of_f, 1);
    kernel_print("  |  NT: ", 9);
    kernel_print_unsigned_hex(nt_f, 1);
    kernel_print("  |  RF: ", 9);
    kernel_print_unsigned_hex(rf_f, 1);
    kernel_print("  |  VM: ", 9);
    kernel_print_unsigned_hex(vm_f, 1);
    kernel_print("          |", 11);
	/***************************************************************************************************/
    kernel_print("| AC: ", 6);
    kernel_print_unsigned_hex(ac_f, 1);
    kernel_print("  |  ID: ", 9);
    kernel_print_unsigned_hex(id_f, 1);
    kernel_print("                                                          |", 59);
    /***************************************************************************************************/
	kernel_print("| IOPL0: ", 9);
    kernel_print_unsigned_hex(iopl0_f, 1);
    kernel_print("  | IOPL1: ", 11);
    kernel_print_unsigned_hex(iopl1_f, 1);
    kernel_print("  | VIF: ", 9);
    kernel_print_unsigned_hex(vif_f, 1);
    kernel_print("  | VIP: ", 9);
    kernel_print_unsigned_hex(vip_f, 1);
    kernel_print("                             |", 31);    
	kernel_print("|                                                                              |", 80);
	kernel_print("|                         LET'S HOPE IT WON'T EXPLODE                          |", 80);
	kernel_print("#==============================================================================", 79);

	/* We will never return from interrupt */
	while(1)
	{
		hlt();
	}
}

/*********************************************************************************|
 *#=============================      OS PANIC      =============================#|
 *|                                                                              ||
 *| Reason: ###############################         INT ID: 0x##                 ||
 *|                                                                              ||
 *| Instruction: 0x########                                                      || 
 *|                                                                              || 
 *|================================= CPU STATE ==================================||
 *|                                                                              ||
 *| EAX: 0x########  |  EBX: 0x########  |  ECX: 0x########  |  EDX: 0x########  ||
 *| ESI: 0x########  |  EDI: 0x########  |  EBP: 0x########  |  ESP: 0x########  ||
 *|                                                                              ||
 *|============================= SEGMENT REGISTERS ==============================||
 *|                                                                              ||
 *| CS:  0x########  |  DS:  0x########  |  SS:  0x########                      ||
 *| ES:  0x########  |  FS:  0x########  |  GS:  0x########                      ||
 *|                                                                              ||
 *|================================= EFLAGS REG =================================||
 *|                                                                              ||
 *| CF: 0x#  |  PF: 0x#  |  AF: 0x#  |  ZF: 0x#  |  SF: 0x#  |  TF: 0x#          ||
 *| IF: 0x#  |  DF: 0x#  |  OF: 0x#  |  NT: 0x#  |  RF: 0x#  |  VM: 0x#          ||
 *| AC: 0x#  |  ID: 0x#                                                          ||
 *| IOPL0: 0x#  |  IOPL1: 0x#  |  VIF: 0x#  |  VIP: 0x#                          ||
 *|                                                                              ||
 *|                          LET'S HOPE IT WON'T EXPLODE                         ||
 *#==============================================================================#|                                                              
 *********************************************************************************|
 */