/**
 ******************************************************************************
 *
 * @file 	mariOS.c
 * @author 	Mario Barbareschi <mario.barbareschi@unina.it>
 * @version V1.1
 * @date    10-July-2018
 * @brief 	Implementation file of main functionalities of mariOS.
 *		  	This file provides the system's tasks list structure, the mariOS
 *		  	ticks variable, two pointers that are used for the context switch by
 *		  	porting function and configured each time the scheduler is invoked.
 *		  	Additionally, this file contains the implementation of the idle task
 *		  	and the function executed by completed task.
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

#include "mariOS.h"

/**
 * Here we define a list containing all tasks that the scheduler must handle.
 * Additionally, the table reports the number of created tasks and the current
 * active task (there is one and only one active task at time)
 */
static struct
{
	mariOS_task_control_block_t tasks[MARIOS_CONFIG_MAX_TASKS];
	volatile mariOS_task_id_t current_active_task; /** The current executing task */
	uint16_t size; /** Number of tasks that have been created */
} mariOS_tasks_list;

/**
 * This variable handles the mariOS ticks from the RTOS boot.
 */
volatile uint32_t mariOS_ticks;

/**
 * These two variables are used to get trace of current executing task
 * and one that has just been scheduled as next one. Once the yield
 * accomplishes, these variables are equal to each other.
 */
mariOS_task_control_block_t* volatile mariOS_curr_task;
mariOS_task_control_block_t* volatile mariOS_next_task;

/**
 * mariOS_idle is the system idle task. It should be modified accordingly to
 * the system's needs.
 */
static void mariOS_idle()
{
	volatile uint64_t idleCount = 0;
	while (1){
		idleCount++;
		__disable_irq(); //Here the yield must be protected against other incoming interrupts
		mariOS_task_yield();
		__enable_irq();
	}
}

/**
 * The task_completion acts like a trap for those tasks that completes their
 * job. It should be modified accordingly to the system's needs.
 */
static void task_completion(void)
{
	/* This function is called when some task handler returns. */
	volatile uint32_t i = 0;
	while (1)
		i++;
}

void mariOS_init(void)
{
	memset(&mariOS_tasks_list, 0, sizeof(mariOS_tasks_list));
	mariOS_ticks = 0;
	/**
	 * Current idle process implementation does not need a lot of space,
	 * even though its minimum size depends on the target architecture.
	 */
	mariOS_task_init(mariOS_idle, MARIOS_IDLE_TASK_STACK);
}

mariOS_task_id_t mariOS_task_init(void (*handler)(void), uint32_t stack_size)
{
	if (mariOS_tasks_list.size >= MARIOS_CONFIG_MAX_TASKS-1)
		return -1;

	/* Initialize the task structure and set SP to the top of the stack
	   minus 16 words (64 bytes) to leave space for storing 16 registers: */
	mariOS_task_control_block_t *p_task = &mariOS_tasks_list.tasks[mariOS_tasks_list.size];
	p_task->handler = handler;
	if(MARIOS_MINIMUM_TASK_STACK_SIZE >= stack_size)
		stack_size = MARIOS_MINIMUM_TASK_STACK_SIZE;
	mariOS_stack_t *p_stack = malloc(stack_size*sizeof(mariOS_stack_t));
	p_task->sp = (uint32_t)(p_stack+stack_size-16);
	p_task->status = MARIOS_TASK_STATUS_READY;
	p_task->wait_ticks = 0;

	/* Here some special registers are defined and they  will be restored on exception return, namely:
	   - XPSR: Default value (0x01000000)
	   - PC: Point to the handler function
	   - LR: Point to a function to be called when the handler returns */
	p_stack[stack_size-1] = 0x01000000;
	p_stack[stack_size-2] = (uint32_t)handler;
	p_stack[stack_size-3] = (uint32_t) &task_completion;

	mariOS_tasks_list.size++;

	/** The taskID of tasks starts from 0, while mariOS_idle does not have any ID, even though its index is 0 */
	return (mariOS_tasks_list.size-1);
}

int mariOS_start(uint32_t systick_ticks)
{
	configureSystick(systick_ticks);

	/* Start the first task: should be the first non-idle */
	mariOS_curr_task = &mariOS_tasks_list.tasks[mariOS_tasks_list.current_active_task];

	loadFirstTask();
	/** This point should be never reached */
	return 0;
}

void marios_systick_handler(void)
{
	++mariOS_ticks;

	int i;
	for(i = 1; i < mariOS_tasks_list.size; i++) //loop over tasks except for idle one
	{	//Remove tasks from suspend state if "wait_ticks" elapses
		//Does not matter mariosTicks overflow, since we are checking the equality
		if(MARIOS_TASK_STATUS_WAIT == mariOS_tasks_list.tasks[i].status && mariOS_tasks_list.tasks[i].wait_ticks == mariOS_ticks){
			mariOS_tasks_list.tasks[i].status = MARIOS_TASK_STATUS_READY;
			mariOS_tasks_list.tasks[i].wait_ticks = 0;
		}
	}
	mariOS_task_yield();
}

void mariOS_task_yield(void)
{
	mariOS_scheduler();
	mariOS_next_task->last_active_time = mariOS_ticks;
	if(mariOS_next_task != mariOS_curr_task)
	{
		yield(); /** Actually, we are triggering the activation of the PendSV_Handler() */
	}
}

/**
 * mariOS scheduler makes use of this function for picking a task. If another algorithm
 * has to be executed, a different function must be provided and called
 */
void round_robin_scheduler(){
	int i = 0;
	do{
		++i;
		mariOS_tasks_list.current_active_task++;
		if (mariOS_tasks_list.current_active_task >= mariOS_tasks_list.size)
			mariOS_tasks_list.current_active_task = 1; //let's skip idle

	} while(i < mariOS_tasks_list.size && MARIOS_TASK_STATUS_READY != mariOS_tasks_list.tasks[mariOS_tasks_list.current_active_task].status);
	if( i == mariOS_tasks_list.size ) //if we cycled over the task table, the only suitable task is the idle
	{
		mariOS_tasks_list.current_active_task = 0;
	}
}

void mariOS_scheduler(void)
{
	/** Retrieve the current active task*/
	mariOS_curr_task = &mariOS_tasks_list.tasks[mariOS_tasks_list.current_active_task];

	/** If it is active, namely it has been set neither in wait nor suspend, its status must be changed in ready */
	if(MARIOS_TASK_STATUS_ACTIVE == mariOS_curr_task->status)
		mariOS_curr_task->status = MARIOS_TASK_STATUS_READY;

	/** Now, we need to pick the next task: */
	round_robin_scheduler();

	mariOS_next_task = &mariOS_tasks_list.tasks[mariOS_tasks_list.current_active_task];
	mariOS_next_task->status = MARIOS_TASK_STATUS_ACTIVE;
}

void mariOS_delay(uint32_t ticks){
	if(0 != ticks)
	{
		enter_critical_section(); //Here the yield and scheduling must be protected against other incoming interrupts
		{
			mariOS_tasks_list.tasks[mariOS_tasks_list.current_active_task].status = MARIOS_TASK_STATUS_WAIT;
			mariOS_tasks_list.tasks[mariOS_tasks_list.current_active_task].wait_ticks = mariOS_ticks+ticks;
			mariOS_task_yield();
		}
		exit_critical_sction();
	}
}

mariOS_task_id_t get_current_task_id(void)
{
	return mariOS_tasks_list.current_active_task;
}

void set_current_task_status(mariOS_task_status_t status)
{
	set_task_status(mariOS_tasks_list.current_active_task, status);
}

void set_task_status(mariOS_task_id_t task_id, mariOS_task_status_t status)
{
	mariOS_tasks_list.tasks[task_id].status = status;
}

mariOS_task_status_t get_task_status(mariOS_task_id_t task_id)
{
	return mariOS_tasks_list.tasks[task_id].status;
}

mariOS_task_status_t get_current_task_status(void)
{
	return get_task_status(mariOS_tasks_list.current_active_task);
}
