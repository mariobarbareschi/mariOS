#include "mariOS.h"


static struct
{
	marios_task_t tasks[MARIOS_CONFIG_MAX_TASKS];
	volatile uint32_t current_task;
	uint16_t size;
} m_task_table;

marios_task_t* volatile marios_curr_task;
marios_task_t* volatile marios_next_task;


static void mariOSidle()
{
	volatile uint64_t idleCount = 0;
	while (1){
		idleCount++;
		__disable_irq(); //Here the yield must be protected against other incoming interrupts
		marios_task_yield();
		__enable_irq();
	}
}

static void task_completion(void)
{
	/* This function is called when some task handler returns. */
	volatile uint32_t i = 0;
	while (1)
		i++;
}

void marios_init(void)
{
	memset(&m_task_table, 0, sizeof(m_task_table));
	mariosTicks = 0;
	marios_task_init(mariOSidle, 40);
}

marios_task_id_t marios_task_init(void (*handler)(void), uint32_t stack_size)
{
	if (m_task_table.size >= MARIOS_CONFIG_MAX_TASKS-1)
		return -1;

	/* Initialize the task structure and set SP to the top of the stack
	   minus 16 words (64 bytes) to leave space for storing 16 registers: */
	marios_task_t *p_task = &m_task_table.tasks[m_task_table.size];
	p_task->handler = handler;
	marios_stack_t *p_stack = (marios_stack_t*) malloc(stack_size*sizeof(marios_stack_t));
	p_task->sp = (uint32_t)(p_stack+stack_size-16);
	p_task->status = MARIOS_TASK_STATUS_READY;
	p_task->wait_ticks = 0;

	/* Here some special registers are defined and they  will be restored on exception return, namely:
	   - XPSR: Default value (0x01000000)
	   - PC: Point to the handler function
	   - LR: Point to a function to be called when the handler returns */
	p_stack[stack_size-1] = 0x01000000;
	p_stack[stack_size-2] = (uint32_t)handler;
	p_stack[stack_size-3] = (uint32_t) &task_completion;

	m_task_table.size++;

	return (m_task_table.size-1);
}

int marios_start(uint32_t systick_ticks)
{
	configureSystick(systick_ticks);

	/* Start the first task: should be the first non-idle */
	marios_curr_task = &m_task_table.tasks[m_task_table.current_task];

	loadFirstTask();
	//This point should be never reached
	return 0;
}

void marios_systick_handler(void)
{
	++mariosTicks;

	int i;
	for(i = 1; i < m_task_table.size; i++) //loop over tasks except for idle one
	{	//Remove tasks from suspend state if "wait_ticks" elapses
		//Does not matter mariosTicks overflow, since we are checking the equality
		if(MARIOS_TASK_STATUS_WAIT == m_task_table.tasks[i].status && m_task_table.tasks[i].wait_ticks == mariosTicks){
			m_task_table.tasks[i].status = MARIOS_TASK_STATUS_READY;
			m_task_table.tasks[i].wait_ticks = 0;
		}
	}
	marios_task_yield();
}

void marios_task_yield(void)
{
	marios_scheduler();
	marios_next_task->timer = mariosTicks;
	if(marios_next_task != marios_curr_task)
	{
		yield(); //Actually, we are triggering the activation of the PendSV_Handler
	}
}

void marios_scheduler(void)
{
	marios_curr_task = &m_task_table.tasks[m_task_table.current_task];

	if(MARIOS_TASK_STATUS_ACTIVE == marios_curr_task->status)
		marios_curr_task->status = MARIOS_TASK_STATUS_READY;

	/* Select next task: */
	int i = 0;
	do{
		++i;
		m_task_table.current_task++;
		if (m_task_table.current_task >= m_task_table.size)
			m_task_table.current_task = 1; //let's skip idle

	} while(i < m_task_table.size && MARIOS_TASK_STATUS_READY != m_task_table.tasks[m_task_table.current_task].status);
	if( i == m_task_table.size ) //if we cycled over the task table, the only suitable task is the idle
	{
		m_task_table.current_task = 0;
	}

	marios_next_task = &m_task_table.tasks[m_task_table.current_task];
	marios_next_task->status = MARIOS_TASK_STATUS_ACTIVE;
}

void marios_delay(uint32_t ticks){
	if(0 != ticks)
	{
		enterCriticalRegion(); //Here the yield and scheduling must be protected against other incoming interrupts
		{
			m_task_table.tasks[m_task_table.current_task].status = MARIOS_TASK_STATUS_WAIT;
			m_task_table.tasks[m_task_table.current_task].wait_ticks = mariosTicks+ticks;
			marios_task_yield();
		}
		exitCriticalRegion();
	}
}

unsigned int get_current_task_id(void)
{
	return m_task_table.current_task;
}

void set_current_task_status(marios_task_status_t status)
{
	set_task_status(m_task_table.current_task, status);
}

void set_task_status(unsigned int task_id, marios_task_status_t status)
{
	m_task_table.tasks[task_id].status = status;
}

marios_task_status_t get_task_status(unsigned int task_id)
{
	return m_task_table.tasks[task_id].status;
}

marios_task_status_t get_current_task_status(void)
{
	return get_task_status(m_task_table.current_task);
}
