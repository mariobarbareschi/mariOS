.syntax unified
.cpu cortex-m4
.fpu softvfp

.thumb

.global PendSV_Handler
.type PendSV_Handler, %function
PendSV_Handler:
	/* Disable interrupts: */
	cpsid	i
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
	mrs	r0, psp //Get the current stack pointer
	subs	r0, #32 //go 32 bytes ahead to push over 8 remaining regs
	stmia	r0!,{r4-r11}
	subs	r0, #32	//Restore the pristine state

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
	ldr	r2, =marios_curr_task
	ldr	r1, [r2]
	str	r0, [r1]

	//Now we are ready to load the new context overwriting the one into the CPU

	/* Load next task's SP: */
	ldr	r2, =marios_next_task
	ldr	r1, [r2]
	ldr	r0, [r1] //Got the stack pointer

	//The process is pretty much like the same, but we pull instead
	ldmia	r0!,{r4-r11}
	msr	psp, r0
	//Note that remaining registers are going to be automatically restored by returning from the ISR
	/* EXC_RETURN - Thread mode with PSP: */
	ldr r0, =0xFFFFFFFD

	/* Enable interrupts: */
	cpsie	i

	bx	r0

.size PendSV_Handler, .-PendSV_Handler

.global SVC_Handler
.type SVC_Handler, %function
SVC_Handler:
	ldr	r2,=marios_curr_task
	ldr r1, [r2]
	ldr r0, [r1] //Get the stack pointer of the task
	ldmia r0!, {r4-r11} // Pop registers that will be not automatically loaded on exception entry
	msr psp, r0
	mov r0, #0 //Set 0 to basepri
	msr	basepri, r0
	//Note that remaining registers are going to be automatically restored by returning from the ISR
	/* EXC_RETURN - Thread mode with PSP: */
	orr r14, #0xd
	bx r14
.align 2
.size SVC_Handler, .-SVC_Handler
