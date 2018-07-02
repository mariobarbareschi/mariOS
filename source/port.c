/*
 * port.c
 *
 *  Created on: 29 giu 2018
 *      Author: mariobarbareschi
 */

#include "port.h"

void loadFirstTask(){
	__asm volatile(	//TODO: take the pristine system stack from VTOR
					" ldr r0,=_estack 		\n" //Take the original master stack pointer from the system startup
					" msr msp, r0			\n" // Set the msp back to the start of the stack
					" cpsie i				\n" //enable interrupt for getting the SWI 0
					" cpsie f				\n"
					" dsb					\n"
					" isb					\n" //flush the pipe
					" svc 0					\n" // we invoke a SWI to start first task. */
					" nop					\n"
	);

}

void SVC_Handler(){
	__asm volatile(
				" ldr	r2,=marios_curr_task 		\n"
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

			" 	mrs		r0, psp 		\n" //Get the current stack pointer
			"   isb						\n" //Flushes pipe
			" 	stmdb	r0!,{r4-r11} 	\n" //Save the remaining registers

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
			" 	ldr	r2, =marios_curr_task 			\n"
			" 	ldr	r1, [r2] 			\n"
			" 	str	r0, [r1] 			\n"

			//Now we are ready to load the new context overwriting the one into the CPU

			/* Load next task's SP: */
			" 	ldr	r2, =marios_next_task 			\n"
			" 	ldr	r1, [r2] 			\n"
			" 	ldr	r0, [r1] //Got the stack pointer 			\n"

			//The process is pretty much like the same, but we pull instead
			" 	ldmia	r0!,{r4-r11} 			\n"
			" 	msr	psp, r0 			\n"
			//Note that remaining registers are going to be automatically restored by returning from the ISR
			/* EXC_RETURN - Thread mode with PSP: */
			"	ldr r0, =0xFFFFFFFD						\n"
			/* Enable interrupts: */
			" 	cpsie	i 			\n"
			"	isb						\n"
			" 	bx	r0 			\n"
		);
	}
