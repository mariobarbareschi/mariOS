#include "cmsis_os.h"

osStatus osKernelInitialize (void){
	marios_init();
}

osStatus osKernelStart (void)
{
  marios_start(SystemCoreClock/MARIOS_CONFIG_SYSTICK_DIV);
  return osOK;
}

uint32_t osKernelSysTick(void)
{
	marios_systick_handler();
}

osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument)
{
	return marios_task_init(thread_def->pthread, thread_def->stacksize);
  
}

osThreadId osThreadGetId (void)
{
	return NULL;
}

osStatus osThreadTerminate (osThreadId thread_id)
{
  return osErrorOS;
}

osStatus osThreadYield (void)
{
	marios_task_yield();
  return osOK;
}

osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority)
{
  return osErrorOS;
}


osPriority osThreadGetPriority (osThreadId thread_id)
{
  return osPriorityError;
}

osStatus osDelay (uint32_t millisec)
{
	marios_delay(MARIOS_CONFIG_SYSTICK_DIV/1000*millisec);
	return osOK;
}

