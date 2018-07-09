#ifndef MARIOS_H
#define MARIOS_H

#include "marios_config.h"
#include "port.h"

typedef uint32_t marios_stack_t;
typedef uint16_t mariOS_task_id_t;

/**
 * The mariOS_task_status_t is the enumerative type that mariOS exploits
 * for implemeting the scheduler. Actually, mariOS handles four different
 * task status.
 *
 * MARIOS_TASK_STATUS_READY indicates that a task is ready to be scheduled,
 * hence the scheduling algorithm can take it into consideration.
 *
 * MARIOS_TASK_STATUS_ACTIVE indicates that a taks is currently executing.
 * As one can figure out, one and only one task a time takes the active
 * status.
 *
 * MARIOS_TASK_STATUS_WAIT means that a task performed the mariOS_delay
 * function for some ticks which have not yet been elapsing. In this
 * status, a task cannot be scheduled as active.
 *
 * MARIOS_TASK_STATUS_SUSPEND means that a task voluntary performed a yield
 * due to a resource contest, such as sending/receiving to/from a queue.
 * In this status, a task cannot be scheduled as active.
 */
typedef enum
{
	MARIOS_TASK_STATUS_READY = 0,
	MARIOS_TASK_STATUS_ACTIVE = 1,
	MARIOS_TASK_STATUS_WAIT = -1,
	MARIOS_TASK_STATUS_SUSPEND = 2
} mariOS_task_status_t;

/**
 * \brief This struct defines the mariOS task control block.
 * It contains the task stack pointer, the function pointer to the task
 * implementation, the task status, the last time it has been scheduled
 * as active, the number of ticks that the mariOS system tick has to
 * reach to switch its status from MARIOS_TASK_STATUS_WAIT to
 * MARIOS_TASK_STATUS_READY.
 */
typedef struct
{
	/* The stack pointer (sp) has to be the first element as it is located
	   at the same address as the structure itself (which makes it possible
	   to locate it safely from assembly implementation of PendSV_Handler).
	   The compiler might add padding between other structure elements. */
	volatile uint32_t sp;
	void (*handler)(void);
	volatile mariOS_task_status_t status;
	volatile uint32_t last_active_time;
	volatile uint32_t wait_ticks;
} mariOS_task_control_block_t;

/**
 * \brief This function initialize the marios_tasks_list struct and populate it\
 * with the first task of the system, namely the idle one.
 * For this reason, idle takes the threadID 0.
 */
void mariOS_init(void);

/**
 * \brief This function initialize a new entry in the tasks list.
 * Actually, mariOS defines a task as a C function with no input/output parameters
 * (properly, non returning/cyclic).The stack size has to be statically determined,
 * taking into account the number of nesting function calling and related
 * static-allocated data (which belongs to the task's stack).
 *
 * Status of each created task is set to MARIOS_TASK_STATUS_READY, meant that
 * the scheduler may pick it to be the current executed task, setting it to the
 * status MARIOS_TASK_STATUS_ACTIVE.
 * \param handler is the function pointer
 * \param stack_size decides the size of the task's stack
 *
 */
mariOS_task_id_t mariOS_task_init(void (*handler)(void), uint32_t stack_size);

/**
 * \brief This function makes mariOS started, meant that the system's timer has to\\
 * be configured accordingly to the mariOS defined sytick and the first task\\
 * (the idle one) has to be loaded.
 * Depending on the system target, some other
 * configurations must be provided, especially those related to the interrupt
 * controller, interrupt priority and additional driver configuration.
 * \param systick_ticks is the number of last_active_time's ticks between two interruption
 * events
 */
int mariOS_start(uint32_t systick_ticks);

/**
 * \brief This function implements the mariOS's scheduler.
 * The main aim is to get the current active task status to MARIOS_TASK_STATUS_READY
 * (mariOS_current_task), pick a task from the mariOS's tasks list and assign it to
 * the marios_next_task, which is then set as MARIOS_TASK_STATUS_READY.
 *
 * Currently, the scheduler implementation is just a round robin, hence the
 * scheduler picks the task next to the current one in the mariOS tasks list.
 * Obviously, a more sophisticated implementation is feasible, but maybe the
 * marios_task_control_block_t should be enriched with further information.
 *
 * Most important aspect, which does not depend on specific implementation of
 * scheduler, is the existance of the mariOS_idle task. Indded, each time the
 * scheduler is not able to determine a task to be executed, it should pick
 * the mariOS_idle task instead. In other words, the mariOS_idle task is the
 * least priority task of the system, matching the general meaing of the
 * idle task.
 */
void mariOS_scheduler(void);

/**
 * This function implements the task yield, hence it gives to a task the
 * possibility of a self-preemption for letting the scheduler pick another
 * task. Hence, first, the function must call the scheduler, then it forces
 * the system to call the yield, which forces the PendSV_Handler ISR whenever
 * the scheduler picks a task different to the current active one.
 */
void mariOS_task_yield(void);

/**
 * \brief This function allows to switch the task status from MARIOS_TASK_STATUS_ACTIVE
 * to the MARIOS_TASK_STATUS_WAIT.
 * In particular, the function must record the time (expressed in mariOS ticks) on
 * which the task status has to be switch to MARIOS_TASK_STATUS_READY. Until the task
 * status is MARIOS_TASK_STATUS_WAIT, the scheduler cannot execute (mariOS does not
 * admit task status transition from MARIOS_TASK_STATUS_WAIT to
 * MARIOS_TASK_STATUS_ACTIVE).
 *
 * Once the task status transition from MARIOS_TASK_STATUS_ACTIVE to
 * MARIOS_TASK_STATUS_WAIT completed, the function calls mariOS_task_yield to
 * call the scheduler and perform a context switch.
 *
 * \param ticks represents the mariOS ticks that the scheduler must wait before
 * taking into account the task when it is called.
 */
void mariOS_delay(uint32_t ticks);

/**
 * \brief This function represents the core of mariOS.
 * As a common implementation along RTOS projects, the systick handler must take care
 * about main duties of the RTOS that must be performed periodically. A basic one
 * comprehend the mariosTicks incrementing, the verification of which tasks that have
 * the status MARIOS_TASK_STATUS_WAIT must be switched as MARIOS_TASK_STATUS_READY
 * and, finally, the scheduler calling.
 */
void mariOS_systick_handler(void);

/**
 * This accessory function returns the mariOS_task_id of the current active task
 *
 * \return mariOS_task_id of the current active task
 */
mariOS_task_id_t get_current_task_id(void);

/**
 * This accessory function configure the status of the current active task
 *
 * \param status is the new status of current active task
 */
void set_current_task_status(mariOS_task_status_t status);

/**
 * This accessory function configure the status of the a task
 *
 * \param task_id is the id of the task to modify
 * \param status is the new status of current active task
 */
void set_task_status(mariOS_task_id_t task_id, mariOS_task_status_t status);

/**
 * This accessory function returns the status of the current active task
 *
 * \warning Currently, mariOS admits one and only one status when the task is
 * running, namely MARIOS_TASK_STATUS_ACTIVE. However, future extensions may
 * plan more than one active status.
 *
 * \return mariOS_task_status of the current active task
 *
 */
mariOS_task_status_t get_current_task_status(void);

/**
 * This accessory function returns the status of a given task
 *
 * \param task_id is the ID of the task
 *
 * \return mariOS_task_status of the task_id task
 *
 */
mariOS_task_status_t get_task_status(mariOS_task_id_t task_id);


#endif
