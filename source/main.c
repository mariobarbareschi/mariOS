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

static void task1_handler(void);
static void task2_handler(void);
static void task3_handler(void);

int main(void)
{
	HAL_Init();

	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);

	osKernelInitialize();
	osThreadDef_t task1 = {task1_handler, 128};
	osThreadDef_t task2 = {task2_handler, 128};
	osThreadDef_t task3 = {task3_handler, 128};
	osThreadCreate(&task1, NULL);
	osThreadCreate(&task2, NULL);
	osThreadCreate(&task3, NULL);

	BSP_LED_Off(LED3);

	osKernelStart();

	//The program should never reach this point
	Default_Handler();
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
	while (1) {
		BSP_LED_Toggle(LED6);
		osDelay(4000);
	}
}

void Error_Handler(){
	BSP_LED_On(LED3);
}
