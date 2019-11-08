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

mariOS_Task_Define(task1_handler, task1_stack, 40);
mariOS_Task_Define(task2_handler, task2_stack, 40);
mariOS_Task_Define(task3_handler, task3_stack, 40);
mariOS_Task_Define(task4_handler, task4_stack, 40);
mariOS_Task_Define(task5_handler, task5_stack, 40);

mariOS_Queue_Define(queueMsgt4_t5, queuet1_t4_buffer, 10*sizeof(uint32_t));
mariOS_Queue_Define(queueMsgt1_t2, queuet2_t3_buffer, 10*sizeof(uint32_t));
mariOS_Queue_Define(queueMsgt2_t3, queuet4_t5_buffer, 10*sizeof(uint32_t));

int main(void)
{
	HAL_Init();

	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

	osKernelInitialize();
	osThreadDef_t task1 = {task1_handler, task1_stack, 40, 4, 40};
	osThreadDef_t task2 = {task2_handler, task2_stack, 40, 3, 500};
	osThreadDef_t task3 = {task3_handler, task3_stack, 40, 2, 500};
	osThreadDef_t task4 = {task4_handler, task4_stack, 40, 99, 50};
	osThreadDef_t task5 = {task5_handler, task5_stack, 40, 1, 50};
	osThreadCreate(&task1, NULL);
	osThreadCreate(&task2, NULL);
	osThreadCreate(&task3, NULL);
	osThreadCreate(&task4, NULL);
	osThreadCreate(&task5, NULL);

	queueMsgt1_t2 = createQueue(queuet1_t4_buffer, sizeof(uint32_t)*10);
	queueMsgt2_t3 = createQueue(queuet2_t3_buffer, sizeof(uint32_t)*10);
	queueMsgt4_t5 = createQueue(queuet4_t5_buffer, sizeof(uint32_t)*10);

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
		mariOS_queue_op_status_t status = enqueue(queueMsgt1_t2,
												  (uint8_t*)&outcoming_msg,
												  sizeof(uint32_t),
												  MARIOS_NONBLOCKING_QUEUE_OP);
		if(MARIOS_QUEUE_SUCCESS_OP == status)
			outcoming_msg=1-outcoming_msg;
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
