/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */


#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

#include "cmsis_os.h"
#include "queue.h"

static void task1_handler(void);
static void task2_handler(void);
static void task3_handler(void);
static void task4_handler(void);
static void task5_handler(void);

static mariOS_queue* queueMsgt4_t5;
static mariOS_queue* queueMsgt1_t2;
static mariOS_queue* queueMsgt2_t3;

int main(void)
{
	HAL_Init();

	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

	osKernelInitialize();
	osThreadDef_t task1 = {task1_handler, 40, 4, 500};
	osThreadDef_t task2 = {task2_handler, 40, 3, 500};
	osThreadDef_t task3 = {task3_handler, 40, 2, 500};
	osThreadDef_t task4 = {task4_handler, 40, 99, 50};
	osThreadDef_t task5 = {task5_handler, 40, 1, 50};
	osThreadCreate(&task1, NULL);
	osThreadCreate(&task2, NULL);
	osThreadCreate(&task3, NULL);
	osThreadCreate(&task4, NULL);
	osThreadCreate(&task5, NULL);

	queueMsgt1_t2 = createQueue(sizeof(uint32_t)*10);
	queueMsgt2_t3 = createQueue(sizeof(uint32_t)*10);
	queueMsgt4_t5 = createQueue(sizeof(uint32_t)*10);

	BSP_LED_Off(LED3);

	NVIC_SetPriority(PendSV_IRQn, 0xff); /* Lowest possible priority */
	NVIC_SetPriority(SysTick_IRQn, 0x00); /* Highest possible priority */

	osKernelStart();

	//The program should never reach this point
	Error_Handler();
	for(;;);
}

mariOS_Task(task1_handler)
{
	uint32_t outcoming_msg = 1;
	mariOS_begin_periodic
	{
		BSP_LED_Toggle(LED4);
		outcoming_msg = 1-outcoming_msg;
		enqueue(queueMsgt1_t2, (uint8_t*)&outcoming_msg, sizeof(uint32_t), MARIOS_NONBLOCKING_QUEUE_OP);
	}
	mariOS_end_periodic;
}

mariOS_Task(task2_handler)
{
	uint32_t incoming_msg;
	uint32_t outcoming_msg = 1;
	mariOS_begin_periodic
	{
		mariOS_queue_op_status_t status = dequeue(
											queueMsgt1_t2,
											(uint8_t*)&incoming_msg,
											sizeof(uint32_t),
											MARIOS_NONBLOCKING_QUEUE_OP);
		if(MARIOS_QUEUE_SUCCESS_OP == status && incoming_msg == 1)
		{
			BSP_LED_Toggle(LED5);
			outcoming_msg = 1-outcoming_msg;
			enqueue(queueMsgt2_t3, (uint8_t*)&outcoming_msg, sizeof(uint32_t), MARIOS_NONBLOCKING_QUEUE_OP);
		}
	}
	mariOS_end_periodic;
}

mariOS_Task(task3_handler)
{
	uint32_t incoming_msg;
	mariOS_begin_periodic
	{
		mariOS_queue_op_status_t status = dequeue(
										    queueMsgt2_t3,
											(uint8_t*)&incoming_msg,
											sizeof(uint32_t),
											MARIOS_NONBLOCKING_QUEUE_OP);
		if(MARIOS_QUEUE_SUCCESS_OP == status && incoming_msg == 1)
		BSP_LED_Toggle(LED6);
	}
	mariOS_end_periodic;
}

mariOS_Task(task4_handler)
{
	uint32_t msg = 0;
	mariOS_begin_periodic
	{
		if(GPIO_PIN_SET == BSP_PB_GetState(BUTTON_KEY))
		{
			msg = 1-msg;
			while(MARIOS_QUEUE_SUCCESS_OP != enqueue(queueMsgt4_t5, (uint8_t*) & msg, sizeof(uint32_t), MARIOS_NONBLOCKING_QUEUE_OP))
				osDelay(50);
			while(GPIO_PIN_SET == BSP_PB_GetState(BUTTON_KEY));
		}
	}
	mariOS_end_periodic;
}

mariOS_Task(task5_handler)
{
	uint32_t msg;
	mariOS_begin_periodic
	{
		dequeue(queueMsgt4_t5, (uint8_t*) & msg, sizeof(uint32_t), MARIOS_NONBLOCKING_QUEUE_OP);
		if(msg == 1)
			BSP_LED_On(LED3);
		else
			BSP_LED_Off(LED3);
	}
	mariOS_end_periodic;
}

void Error_Handler()
{
	BSP_LED_On(LED3);
	for(;;);
}
