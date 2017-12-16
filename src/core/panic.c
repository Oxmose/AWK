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


#include "interrupts.h"    /* cpu_state_t, stack_state_t */
#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stdio.h"  /* printf */
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


	printf("#=============================      OS PANIC      =============================#");
	printf("|                                                                              |");
	printf("| Reason: ");
	switch(int_id)
	{
		case 0:
			printf("Division by zero                        ");
			break;
		default:
			printf("Unknown                                 ");
	}
	printf("INT ID: 0x%02x                 |", int_id);
	printf("|                                                                              |");
	printf("| Instruction: 0x%08x                                                      |", stack_state->eip);
	printf("|                                                                              |");
	printf("|================================= CPU STATE ==================================|");
	printf("|                                                                              |");
	printf("| EAX: 0x%08x  |  EBX: 0x%08x  |  ECX: 0x%08x  |  EDX: 0x%08x  |", cpu_state->eax, cpu_state->ebx, cpu_state->ecx, cpu_state->edx);
	printf("| ESI: 0x%08x  |  EDI: 0x%08x  |  EBP: 0x%08x  |  ESP: 0x%08x  |", cpu_state->esi, cpu_state->edi, cpu_state->ebp, cpu_state->esp);
	printf("|                                                                              |");
	printf("|============================= SEGMENT REGISTERS ==============================|");
	printf("|                                                                              |");
	printf("| CS: 0x%04x  |  DS: 0x%04x  |  SS: 0x%04x                                     |", stack_state->cs, cpu_state->ds, cpu_state->ss);
	printf("| ES: 0x%04x  |  FS: 0x%04x  |  GS: 0x%04x                                     |", cpu_state->es, cpu_state->fs, cpu_state->gs);
	printf("|                                                                              |");
	printf("|================================= EFLAGS REG =================================|");
	printf("|                                                                              |");
    printf("| CF: %d  |  PF: %d  |  AF: %d  |  ZF: %d  |  SF: %d  |  TF: %d  |  IF: %d  |  DF: %d  |", cf_f, pf_f, af_f, zf_f, sf_f, tf_f, if_f, df_f);
    printf("| OF: %d  |  NT: %d  |  RF: %d  |  VM: %d  |  AC: %d  |  ID: %d                      |", of_f, nt_f, rf_f, vm_f, ac_f, id_f);
	printf("| IOPL: %d  |  VIF: %d  |  VIP: %d                                                |", (iopl0_f | iopl1_f << 1), vif_f, vip_f);
    printf("|                                                                              |");
    printf("|                                                                              |");
	printf("|                         LET'S HOPE IT WON'T EXPLODE                          |");
	printf("#==============================================================================");

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