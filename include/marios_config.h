/**
 ******************************************************************************
 *
 * @file 	mariOS_config.h
 * @author 	Mario Barbareschi <mario.barbareschi@unina.it>
 * @version V1.0
 * @date    14-June-2018
 * @brief 	This file contains definitions and macros intended to configure and
 * 			modify some static parameters of mariOS code.
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
#ifndef MARIOS_CONFIG_H_
#define MARIOS_CONFIG_H_

#include<stm32f4xx.h>

#define MARIOS_CONFIG_MAX_TASKS			10
#define MARIOS_CONFIG_SYSTICK_FREQ		HAL_RCC_GetHCLKFreq()/MARIOS_CONFIG_SYSTICK_FREQ_DIV
#define MARIOS_CONFIG_SYSTICK_FREQ_DIV	10000

#define MARIOS_MINIMUM_TASK_STACK_SIZE			40
#define MARIOS_IDLE_TASK_STACK	MARIOS_MINIMUM_TASK_STACK_SIZE+4
#define MARIOS_MAXIMUM_PRIORITY			100

#define MARIOS_SCHEDULER_FUNCTION		priority_scheduler



#endif /* MARIOS_CONFIG_H_ */
