/**
 ******************************************************************************
 *
 * @file 	queue.h
 * @author 	Mario Barbareschi <mario.barbareschi@unina.it>
 * @version V1.0
 * @date    11-July-2018
 * @brief 	Header file of mariOS queue. This file contains some enum
 * 			definitions (namely ::mariOS_queue_status_t,
 * 			::mariOS_blocking_queue_op_t and ::mariOS_queue_op_status_t ),
 * 			the queue structure, which is exploited by queue accessory
 * 			functions.
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

#ifndef QUEUE_H_
#define QUEUE_H_

#include <mariOS_config.h>
#include "mariOS.h"

/**
 * @brief This enum is used to set the queue status to lock or unlock.
 * Whevener the queue is locked in reading, no dequeue operations are
 * permitted.
 * Conversely, whevener the queue is locked in writing, no enqueue
 * operations are permitted.
 */
typedef enum
{
	MARIOS_QUEUE_LOCKED,		/**< Queue operation is being blocked by a task					*/
	MARIOS_QUEUE_UNLOCKED,		/**< Queue is unlocked and the operation on it is allowed		*/
} mariOS_queue_status_t;

/**
 * @brief This enum is used to each time the enqueue and dequeue function
 * are invoked. By passing MARIOS_BLOCKING_QUEUE_OP, the queue operation
 * will suspend the task if it cannot be completed. Otherwise, if
 * MARIOS_NONBLOCKING_QUEUE_OP is passed, the queue operation returns
 * the condition (::marios_queue_op_status_t) why it cannot be completed.
 */
typedef enum
{
	MARIOS_BLOCKING_QUEUE_OP,		/**< Queue accessory function is invoked as blocking and may\
										 potentially preempt the task waiting for being resumed		*/
	MARIOS_NONBLOCKING_QUEUE_OP,	/**< Queue accessory function is invoked as non-blocking and\
										 no preemption will occur, conversely the function could
										 not succeed, returng a failure ::mariOS_queue_op_status_t	*/
} mariOS_blocking_queue_op_t;

/**
 * @brief This enum lists the possible outcomes of queue operations.
 * ::MARIOS_QUEUE_SUCCESS_OP indicates a success operation
 * ::MARIOS_QUEUE_BUSY_OP, ::MARIOS_QUEUE_FULL_OP and
 * ::MARIOS_QUEUE_EMPTY_OP indicates, respectively, that the operation
 * cannot be completed because the queue is busy upon another operation,
 * is full (nothing can be enqueued) or is empty (nothing can be dequeued);
 * they can be returned if and only if MARIOS_NONBLOCKING_QUEUE_OP is used
 * when @enqueue and @dequeue functions are invoked.
 */
typedef enum
{
	MARIOS_QUEUE_SUCCESS_OP, 	/**< Queue operation successfully completes						*/
	MARIOS_QUEUE_BUSY_OP,		/**< Queue operation fails due to busy status of the queue		*/
	MARIOS_QUEUE_FULL_OP,		/**< Enqueue on ::mariOS_queue fails since it is full			*/
	MARIOS_QUEUE_EMPTY_OP		/**< Dequeue on ::mariOS_queue fails since it is empty			*/
} mariOS_queue_op_status_t;

/**
 * @brief This struct is used to typedef the mariOS queue. It is a cyclic queue
 * with pointers to the head and tail.
 * The structure contains two arrays containing suspended task reference for
 * enqueue and dequeue operations.
 */
typedef struct queue_t
{
	volatile uint32_t head;											/** the head of the queue */
	volatile uint32_t tail;											/** the tail of the queue */

	uint32_t size;													/** the memory sized reserved to the queue */
	uint32_t freeMemory;											/** the free space on the queue */
	int8_t* queueMemory;											/** the pointer to the queue memory */

	uint8_t tasks_waiting_to_send[MARIOS_CONFIG_MAX_TASKS];			/** array of tasks that are blocked waiting to write into the queue. */
	uint8_t tasks_waiting_to_receive[MARIOS_CONFIG_MAX_TASKS]; 		/** array of tasks that are blocked waiting to read from the queue. */

	volatile mariOS_queue_status_t rLock; 							/** lock the queue for reading operation */
	volatile mariOS_queue_status_t wLock; 							/** lock the queue for writing operation */
} mariOS_queue;

/**
 * @brief The function initialize a mariOS_queue structure with a specified size.
 *
 * @param [in] size is the queue size in bytes
 * @return pointer to the ::mariOS_queue created
 */
mariOS_queue* createQueue(unsigned int size);

/**
 * @brief The enqueue function tries to put a message onto the specified \
 * queue.
 * It takes in input a message and its size (in bytes) and enqueue the
 * message if the queue is not locked for writing operation and if the
 * queue has enough space to contain the message. Whenever these two
 * conditions are not met, the behavior of the function depends on the
 * value of @marios_blocking_queue_op_t passed.
 *
 * If ::MARIOS_BLOCKING_QUEUE_OP is passed and the queue cannot contain the
 * whole message to enqueue, the function suspend the task that will be
 * eventually resumed once the queue has been dequeued by another task.
 * Otherwise, if the queue is just locked for writing operation, the task
 * is simply preempted and will be eventually rescheduled.
 *
 * If ::MARIOS_NONBLOCKING_QUEUE_OP is passed, the function will return
 * error conditions related to the condition that impedes the enqueue.
 * Indeed, two possible conditions may happen:
 * 	- ::MARIOS_QUEUE_BUSY_OP, which indicates that the queue is blocked for
 * writing operations.
 * 	- ::MARIOS_QUEUE_FULL_OP, which indicates that the queue cannot contain
 * 	the whole message.
 *
 * Conversely, if the enqueue succeeds, no matter if blocking or not,
 * the function returns ::MARIOS_QUEUE_SUCCESS_OP.
 *
 * @param [in,out] queue is the mariOS_queue handler on which the message should be
 * 				   enqueued
 * @param [in] msg is the pointer to the message that has to be enqueued
 * @param [in] size is the message size (in bytes)
 * @param [in] blocking specifies if the enqueue is blocking or not
 * @return operation success or failure
 */
mariOS_queue_op_status_t enqueue(mariOS_queue* queue, int8_t* msg, unsigned int size, mariOS_blocking_queue_op_t blocking);

/**
 * @brief The dequeue function tries to pull a message from the specified
 * queue.
 * It takes in input the size of the message to read and the area on which
 * store it. The message is read from the queue if it is not locked for
 * reading operations and if the queue contains at least the amount of
 * requested data. Whenever these two conditions are not met, the behavior
 * of the function depends on the value of ::marios_blocking_queue_op_t
 * passed.
 *
 * If ::MARIOS_BLOCKING_QUEUE_OP is passed and the queue does not contain
 * the message to dequeue, the function suspend the task that will be
 * eventually resumed once the queue has been enqueued by another task.
 * Otherwise, if the queue is just locked for reading operation, the task
 * is simply preempted and will be eventually rescheduled.
 *
 * If ::MARIOS_NONBLOCKING_QUEUE_OP is passed, the function will return
 * error conditions related to the condition that impedes the dequeue.
 * Indeed, two possible conditions may happen:
 * 	- ::MARIOS_QUEUE_BUSY_OP, which indicates that the queue is blocked for
 * reading operations.
 * 	- ::MARIOS_QUEUE_EMPTY_OP, which indicates that the queue does not
 * 	contain requested message.
 *
 * Conversely, if the dequeue succeeds, no matter if blocking or not,
 * the function returns ::MARIOS_QUEUE_SUCCESS_OP.
 *
 * @param [in,out] queue is the mariOS_queue handler from which the message should be
 * 				   dequeued
 * @param [out] msg is the pointer on which the message will be stored
 * @param [in] size is the message size (in bytes)
 * @param [in] blocking specifies if the dequeue is blocking or not
 * @return operation success or failure
 */
mariOS_queue_op_status_t dequeue(mariOS_queue* queue, int8_t* msg, unsigned int size, mariOS_blocking_queue_op_t blocking);

/**
 * @brief The function restore the queue in a pristine state.
 *
 * @param [in,out] queue is the queue handler
 * @retval None
 */
void reset_queue(mariOS_queue* queue);

#endif /* QUEUE_H_ */
