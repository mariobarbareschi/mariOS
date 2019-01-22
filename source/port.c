/**
 ******************************************************************************
 *
 * @file 	port.c
 * @author 	Mario Barbareschi <mario.barbareschi@unina.it>
 * @version V1.3
 * @date    29-June-2018
 * @brief 	Implementation file for function of port.h, in particular for
 * 			supporting ARM Cortex M3/M4 uC.
 *
 ******************************************************************************
 * @attention
 *
 *  Copyright (C) 2018  Mario Barbareschi
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************
 */


#include <mariOS_config.h>
#include "port.h"


void loadFirstTask(){
	__asm volatile(	//" ldr r0,=_estack 		\n" //Take the original master stack pointer from the system startup
			" ldr R0,=0xE000ED08 	\n" //Set R0 to VTOR address
			" ldr r0, [r0] 			\n" //Load VTOR
			" ldr r0, [r0] 			\n" //Load initial MSP value
			" msr msp, r0			\n" // Set the msp back to the start of the stack
			" cpsie i				\n" //enable interrupt for getting the SWI 0
			" dsb					\n" //No instruction in program order after this instruction executes until this instruction completes
			" isb					\n" //It ensures that the effects of context altering operations, such as changing the ASID, or completed TLB maintenance operations, or branch predictor maintenance operations
			" svc 0					\n" // we invoke a SWI to start first task. */
			" nop					\n"
			" .align 2				\n"

	);

}

void SVC_Handler(){
	__asm volatile(
			" ldr	r2,=mariOS_curr_task 		\n"
			" ldr r1, [r2]						\n"
			" ldr r0, [r1] 						\n"//Get the stack pointer of the task
			" ldmia r0!, {r4-r11}				\n" // Pop registers that will be not automatically loaded on exception entry
			" msr psp, r0						\n"
			" mov r0, #0						\n" //Set 0 to basepri
			" msr	basepri, r0					\n"
			//Note that remaining registers are going to be automatically restored by returning from the ISR
			/* EXC_RETURN - Thread mode with PSP: */
			" orr r14, #0xd						\n"
			" bx r14							\n"
			" .align 2							\n"

	);
}


__attribute__(( naked )) void PendSV_Handler(){
	__asm volatile(	/* Disable interrupts: */
			" cpsid	i 					\n"
			//Since entering this handler is due to NVIC, first 4 regs have been already pushed onto the stack
			/*
			+------+
			|      | <- SP after entering interrupt (orig. SP - 32 bytes)
			|  R0  |
			|  R1  |
			|  R2  |
			|  R3  |
			|  R12 |
			|  LR  | (R14)
			|  PC  | (Return address)
			| xPSR |
			|      | <- SP before interrupt (orig. SP)
			+------+

			 */

			" mrs		r0, psp 			\n" //Get the current stack pointer
			" dsb							\n"
			" isb							\n" //Flushes pipe
			" stmdb	r0!,{r4-r11} 			\n" //Save the remaining registers

			/*
			+------+
			|      | <- SP right now (orig. - 64 bytes)
			|  R4  |
			|  R5  |
			|  R6  |
			|  R7  |
			|  R8  |
			|  R9  |
			| R10  |
			| R11  |
			|  R1  |
			|  R2  |
			|  R3  |
			| R12  |
			|  LR  | (R14)
			|  PC  | (Return address)
			| xPSR |
			|      | <- SP before interrupt (orig. SP)
			+------+
			 */
			/* Save current task's SP: */
			" ldr	r2, =mariOS_curr_task 	\n"
			" ldr	r1, [r2] 				\n"
			" str	r0, [r1] 				\n"

			//Now we are ready to load the new context overwriting the one into the CPU

			/* Load next task's SP: */
			" ldr	r2, =mariOS_next_task 	\n"
			" ldr	r1, [r2] 				\n"
			" ldr	r0, [r1] 				\n"//Got the stack pointer

			//The process is pretty much like the same, but we pull instead
			" ldmia	r0!,{r4-r11} 			\n"
			" msr	psp, r0 				\n"
			//Note that remaining registers are going to be automatically restored by returning from the ISR
			/* EXC_RETURN - Thread mode with PSP: */
			" orr r14, #0xd					\n"
			/* Enable interrupts: */
			" cpsie	i	 					\n"
			" dsb							\n"
			" isb							\n"
			" bx	r14 					\n"
			" .align 2						\n"
	);
}


void yield(){
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	__DSB(); //__asm volatile( "dsb" );
	__ISB(); //__asm volatile( "isb" );
}

void enter_critical_section()
{
	__disable_irq();
}
void exit_critical_sction()
{
	__enable_irq();
}

int configureSystick(uint32_t systick_ticks)
{
	uint32_t ret_val = SysTick_Config(systick_ticks);
	if (ret_val != 0)
		return -1;
}
