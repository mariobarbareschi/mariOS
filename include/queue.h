/*
 * queue.h
 *
 *  Created on: 02 lug 2018
 *      Author: mariobarbareschi
 */

#ifndef QUEUE_H_
#define QUEUE_H_
#include "marios_config.h"
#include "mariOS.h"

typedef enum
{
	MARIOS_QUEUE_LOCKED = 1,
	MARIOS_QUEUE_UNLOCKED,
} marios_queue_status_t;

typedef struct queue_t
{
	volatile unsigned int head;					/*the head of the queue */
	volatile unsigned int tail;					/*the tail of the queue */

	unsigned int size;
	unsigned int freeMemory;
	int8_t* queueMemory;

	uint32_t tasks_waiting_to_send[MARIOS_CONFIG_MAX_TASKS];		/* array of tasks that are blocked waiting to write into the queue. */
	uint32_t tasks_waiting_to_receive[MARIOS_CONFIG_MAX_TASKS];;	/* array of tasks that are blocked waiting to read from the queue. */

	volatile uint8_t rLock;
	volatile uint8_t wLock;
} marios_queue;

marios_queue* createQueue(unsigned int size);
void enqueue(marios_queue* queue, int8_t* msg, unsigned int size);
void dequeue(marios_queue* queue, int8_t* msg, unsigned int size);
void resetQueue(marios_queue* queue);

unsigned int freeSpace(marios_queue* queue);

#endif /* QUEUE_H_ */
