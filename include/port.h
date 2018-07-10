/*
 * port.h
 *
 *  Created on: 29 giu 2018
 *      Author: mariobarbareschi
 */

#ifndef PORT_H_
#define PORT_H_

#include <inttypes.h>

/**
 * Once the system is configured and ready for executing the user application,
 * we need to get back a pristine system state, especially regarding the
 * privileged stack. Once everything is cleaned, the system must trigger
 * the first context switch via software interrupt.
 *
 * As for ARM Cortex M3 and M4, we perform the stack reset by retreiving the
 * initial stack pointer value from the interrupt table vector, while the first
 * context switch is due to the SVC call. Since no more than one supervisor
 * call is provided, we assume to call the number 0.
 */
void loadFirstTask();

/**
 * This SVC handler do provide only one interrupt. It performs the first task
 * loading onto the uC, sets the Base Priority Mask Register to the lowest value.
 * These commands need to be executed by the supervisor call since they need the
 * privilege mode.
 */
void SVC_Handler();

/**
 * mariOS achieve the context switch by triggering the PendSV interrupt.
 * For ARM architectures, PendSV is an interrupt-driven request for system-level
 * service mainly used to completes the context switch. The function body is
 * well documented.
 *
 * It exploits two handers, namely marios_curr_task and marios_next_task.
 * Before calling the first instruction of the PendSV_Handler, the uC has
 * already pushed 8 4-byte values onto the stack, including first 4 general
 * registers, of the task marios_curr_task.
 * Once the remaining register have been pushed onto the marios_curr_task stack,
 * the execution context of marios_next_task is restored by replay same steps,
 * but in opposite direction. First, the stack pointer of marios_next_task is
 * loaded, then registers are restored, except for the first 4 which will be
 * automatically restored at the exit.
 */
void PendSV_Handler() __attribute__ (( naked ));

/**
 * Whenever the scheduler wants to remove the current task in favor of
 * another task, it can explicitly call the yield function, which will trigger the
 * PendSV_Handler and, hence, the context switch.
 */
void yield();

/**
 * The function enter_critical_section must guarantee that no other task can
 * access shared resources of the system before calling the exit_critical_sction.
 * This property can be simply achieved by disabling the system interrupts.
 */
void enter_critical_section();

/**
 * The function exit_critical_sction guarantees that the current task can be
 * preempt by the scheduler in favor of other tasks. This property can be simply
 * achieved by enabling the system interrupts.
 */
void exit_critical_sction();

/**
 * The configureSystick function hides the main mechanism on which an RTOS is
 * based, that is periodic interrupts from a system timer.
 * A proper clock period must be select to generate a good interrupt period.
 * Too short period may lead to low efficiency due to the fact that the RTOS
 * routines (and scheduler too) are uselessly executed. Vice versa, Too large
 * period may give to the RTOS less control on system execution.
 *
 * \param systick_ticks is the number of ticks between two interrupts
 */
int configureSystick(uint32_t systick_ticks);

#endif /* PORT_H_ */
