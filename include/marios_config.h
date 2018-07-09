/*
 * marios_config.h
 *
 *  Created on: 14 giu 2018
 *      Author: mariobarbareschi
 */

#ifndef MARIOS_CONFIG_H_
#define MARIOS_CONFIG_H_

#include<stm32f4xx.h>

#define MARIOS_CONFIG_MAX_TASKS			10
#define MARIOS_CONFIG_SYSTICK_DIV		1000

#define MARIOS_IDLE_TASK_STACK			40
#define MARIOS_MINIMUM_TASK_STACK_SIZE	MARIOS_IDLE_TASK_STACK



#endif /* MARIOS_CONFIG_H_ */
