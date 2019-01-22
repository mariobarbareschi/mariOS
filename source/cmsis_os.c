#include "cmsis_os.h"

osStatus osKernelInitialize (void){
	mariOS_init();
}

osStatus osKernelStart (void)
{
  mariOS_start(MARIOS_CONFIG_SYSTICK_FREQ);
  return osOK;
}

uint32_t osKernelSysTick(void)
{
	marios_systick_handler();
}

osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument)
{
	return mariOS_task_init(thread_def->pthread, thread_def->stacksize, thread_def->priority, thread_def->period);
  
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
	mariOS_task_yield();
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
	mariOS_delay(millisec);
	return osOK;
}

