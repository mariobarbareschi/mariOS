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

static marios_queue* queueMsg;

int main(void)
{
	HAL_Init();

	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

	osKernelInitialize();
	osThreadDef_t task1 = {task1_handler, 20};
	osThreadDef_t task2 = {task2_handler, 20};
	osThreadDef_t task3 = {task3_handler, 20};
	osThreadDef_t task4 = {task4_handler, 40};
	osThreadDef_t task5 = {task5_handler, 40};
	osThreadCreate(&task1, NULL);
	osThreadCreate(&task2, NULL);
	osThreadCreate(&task3, NULL);
	osThreadCreate(&task4, NULL);
	osThreadCreate(&task5, NULL);

	queueMsg = createQueue(sizeof(uint32_t)*2);

	BSP_LED_Off(LED3);

	osKernelStart();

	//The program should never reach this point
	Error_Handler();
	for(;;);
}

static void task1_handler(void)
{
	while (1) {
		BSP_LED_Toggle(LED4);
		osDelay(1000);
	}
}

static void task2_handler(void)
{
	while (1) {
		BSP_LED_Toggle(LED5);
		osDelay(2000);
	}
}

static void task3_handler(void)
{
	while (1)
	{
		BSP_LED_Toggle(LED6);
		osDelay(4000);
	}
}

static void task4_handler(void)
{
	uint32_t msg = 0;
	while (1)
	{
		if(GPIO_PIN_SET == BSP_PB_GetState(BUTTON_KEY))
		{
			msg = 1-msg;
			enqueue(queueMsg, (uint8_t*) & msg, sizeof(uint32_t));
			while(GPIO_PIN_SET == BSP_PB_GetState(BUTTON_KEY));
		}
		osDelay(100);
	}
}

static void task5_handler(void)
{
	uint32_t msg;
	while (1)
	{
		dequeue(queueMsg, (uint8_t*) & msg, sizeof(uint32_t));
		if(msg == 1)
			BSP_LED_On(LED3);
		else
			BSP_LED_Off(LED3);

		osDelay(100);
	}
}

void Error_Handler()
{
	BSP_LED_On(LED3);
	for(;;);
}
