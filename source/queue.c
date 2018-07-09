/*
 * queue.c
 *
 *  Created on: 02 lug 2018
 *      Author: mariobarbareschi
 */

#include "queue.h"
#include "mariOS.h"

marios_queue* createQueue(unsigned int size)
{
	marios_queue* queue = (marios_queue*) malloc(sizeof(marios_queue));
	queue->size = size;
	queue->queueMemory = (uint8_t*) malloc(size);
	queue->rLock = MARIOS_QUEUE_UNLOCKED;
	queue->wLock = MARIOS_QUEUE_UNLOCKED;
	resetQueue(queue);
	return queue;
}

unsigned int freeSpace(marios_queue* queue)
{
	return queue->freeMemory;
}

marios_queue_op_status_t enqueue(marios_queue* queue, int8_t* msg, unsigned int msg_size, marios_blocking_queue_op_t blocking)
{
	int writtenFlag = 0;
	while(0 == writtenFlag) //This flag will be set once the writing is achieved
	{
		enterCriticalRegion();
		{
			if(MARIOS_QUEUE_UNLOCKED == queue->wLock)
			{
				queue->wLock = MARIOS_QUEUE_LOCKED;
				if(msg_size > queue->freeMemory) //Check available space and yield if no enough space
				{
					if(MARIOS_BLOCKING_QUEUE_OP == blocking)
					{
						queue->tasks_waiting_to_send[get_current_task_id()] = 1;
						set_current_task_status(MARIOS_TASK_STATUS_SUSPEND);
						mariOS_task_yield();
					}
					else
					{
						queue->wLock = MARIOS_QUEUE_UNLOCKED;
						exitCriticalRegion();
						return MARIOS_QUEUE_FULL_OP;
					}
				}
				else //Space available
				{
					if(queue->head+msg_size <= queue->size) //The message just is append
					{
						memcpy(queue->queueMemory+queue->head, msg, msg_size);
						queue->head += msg_size;
					}
					else //We need to split the copy
					{
						memcpy(queue->queueMemory+queue->head, msg, queue->size-queue->head);
						memcpy(queue->queueMemory, msg+queue->size-queue->head, msg_size-(queue->size-queue->head));
						queue->head = msg_size-(queue->size-queue->head); //The head just point to the first free location
					}
					queue->freeMemory -= msg_size;
					//Let's make awaken suspended tasks
					int i;
					for(i = 0; i < MARIOS_CONFIG_MAX_TASKS; i++)
						if(1 == queue->tasks_waiting_to_receive[i])
						{
							set_task_status(i, MARIOS_TASK_STATUS_READY);
							queue->tasks_waiting_to_receive[i] = 0;
						}
					writtenFlag = 1;
				}
				queue->wLock = MARIOS_QUEUE_UNLOCKED;
			}
			else //The queue is just locked for sending
			{
				if(MARIOS_BLOCKING_QUEUE_OP == blocking)
				{
					mariOS_task_yield();
				}
				else
				{
					exitCriticalRegion();
					return MARIOS_QUEUE_BUSY_OP;
				}
			}
		}
		exitCriticalRegion();
	}
	return MARIOS_QUEUE_SUCCESS_OP;
}

marios_queue_op_status_t dequeue(marios_queue* queue, int8_t* msg, unsigned int msg_size, marios_blocking_queue_op_t blocking)
{
	int receivedFlag = 0;
	while(0 == receivedFlag) //This flag will be set once the writing is achieved
	{
		enterCriticalRegion();
		{
			if(MARIOS_QUEUE_UNLOCKED == queue->rLock)
			{
				queue->rLock = MARIOS_QUEUE_LOCKED;
				if(msg_size > (queue->size - queue->freeMemory)) //Check whether the queue is containing at least one msg
				{
					if(MARIOS_BLOCKING_QUEUE_OP == blocking)
					{
						queue->tasks_waiting_to_receive[get_current_task_id()] = 1;
						set_current_task_status(MARIOS_TASK_STATUS_SUSPEND);
						mariOS_task_yield();
					}
					else
					{
						queue->wLock = MARIOS_QUEUE_UNLOCKED;
						exitCriticalRegion();
						return MARIOS_QUEUE_EMPTY_OP;
					}
				}
				else //There is at least one msg
				{
					if(queue->tail+msg_size <= queue->size) //If the message is stored along the queue
					{
						memcpy(msg, queue->queueMemory+queue->tail,msg_size);
						queue->tail += msg_size;
					}
					else //We need to split the copy
					{
						memcpy(msg, queue->queueMemory+queue->tail, queue->size-queue->tail);
						memcpy(msg+queue->size-queue->tail, queue->queueMemory, msg_size-(queue->size-queue->tail));
						queue->tail = msg_size-(queue->size-queue->tail); //The tail just point to the next non-free region
					}
					queue->freeMemory += msg_size;
					//Let's make awaken suspended tasks
					int i;
					for(i = 0; i < MARIOS_CONFIG_MAX_TASKS; i++)
						if(1 == queue->tasks_waiting_to_send[i])
						{
							set_task_status(i, MARIOS_TASK_STATUS_READY);
							queue->tasks_waiting_to_send[i] = 0;
						}
					receivedFlag = 1;
				}
				queue->rLock = MARIOS_QUEUE_UNLOCKED;
			}
			else //The queue is just locked for receiving
			{
				if(MARIOS_BLOCKING_QUEUE_OP == blocking)
				{
					mariOS_task_yield();
				}
				else
				{
					exitCriticalRegion();
					return MARIOS_QUEUE_BUSY_OP;
				}
			}
		}
		exitCriticalRegion();
	}
	return MARIOS_QUEUE_SUCCESS_OP;
}

void resetQueue(marios_queue* queue)
{
	if(MARIOS_QUEUE_UNLOCKED == queue->rLock && MARIOS_QUEUE_UNLOCKED == queue->wLock)
	{
		int i;
		for(i = 0; i < queue->size; i++)
			queue->queueMemory[i] = 0;
		queue->head = 0;
		queue->tail = 0;
		queue->freeMemory = queue->size;
		for(i = 0; i < MARIOS_CONFIG_MAX_TASKS; i++)
		{
			queue->tasks_waiting_to_receive[i] = 0;
			queue->tasks_waiting_to_send[i] = 0;
		}
	}
}
