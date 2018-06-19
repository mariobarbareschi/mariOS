#include "marios.h"

typedef enum
{
	OS_TASK_STATUS_IDLE = 1,
	OS_TASK_STATUS_ACTIVE,
	OS_TASK_STATUS_SUSPEND
} os_task_status_t;

volatile uint32_t mariosTicks;

typedef struct
{
	/* The stack pointer (sp) has to be the first element as it is located
	   at the same address as the structure itself (which makes it possible
	   to locate it safely from assembly implementation of PendSV_Handler).
	   The compiler might add padding between other structure elements. */
	volatile uint32_t sp;
	void (*handler)(void);
	volatile os_task_status_t status;
	volatile uint32_t timer;
	volatile uint32_t wait_ticks;
} marios_task_t;

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

static void task_finished(void)
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
	marios_stack_t *p_stack = (marios_stack_t*) malloc(128*sizeof(marios_stack_t));
	p_task->sp = (uint32_t)(p_stack+stack_size-16);
	p_task->status = OS_TASK_STATUS_IDLE;
	p_task->wait_ticks = 0;

	/* Here some special registers are defined and they  will be restored on exception return, namely:
	   - XPSR: Default value (0x01000000)
	   - PC: Point to the handler function
	   - LR: Point to a function to be called when the handler returns */
	p_stack[stack_size-1] = 0x01000000;
	p_stack[stack_size-2] = (uint32_t)handler;
	p_stack[stack_size-3] = (uint32_t) &task_finished;

	p_stack[stack_size-10] = (uint32_t) (m_task_table.size+1);

	m_task_table.size++;

	return (m_task_table.size-1);
}

//This function relies on the Core CM4 of ARM (core_cm4.h)
int marios_start(uint32_t systick_ticks)
{
	NVIC_SetPriority(PendSV_IRQn, 0xff); /* Lowest possible priority */
	NVIC_SetPriority(SysTick_IRQn, 0x00); /* Highest possible priority */

	/* Start the SysTick timer: */
	uint32_t ret_val = SysTick_Config(systick_ticks);
	if (ret_val != 0)
		return -1;

	/* Start the first task: should be the first non-idle */
	marios_curr_task = &m_task_table.tasks[m_task_table.current_task];

	__asm volatile(		//TODO: take the pristine system stack from VTOR
						" ldr r0,=_estack 		\n" //Take the original master stack pointer from the system startup
						" msr msp, r0			\n" // Set the msp back to the start of the stack
						" cpsie i				\n" //enable interrupt for getting the SWI 0
						" cpsie f				\n"
						" dsb					\n"
						" isb					\n" //flush the pipe
						" svc 0					\n" // we invoke a SWI to start first task. */
						" nop					\n"
					);
	//This point should be never reached
	return 0;
}

void marios_systick_handler(void)
{
	++mariosTicks;

	int i;
	for(i = 1; i < m_task_table.size; i++) //loop over tasks except for idle
	{	//Remove tasks from suspend state if "wait_ticks" elapses
		//Does not matter mariosTicks overflow, since we are checking the equality
		if(OS_TASK_STATUS_SUSPEND == m_task_table.tasks[i].status && m_task_table.tasks[i].wait_ticks == mariosTicks){
			m_task_table.tasks[i].status = OS_TASK_STATUS_IDLE;
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
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
		__DSB(); //__asm volatile( "dsb" );
		__ISB(); //__asm volatile( "isb" );
	}
}

void marios_scheduler(void)
{
	marios_curr_task = &m_task_table.tasks[m_task_table.current_task];

	if(OS_TASK_STATUS_SUSPEND != marios_curr_task->status)
		marios_curr_task->status = OS_TASK_STATUS_IDLE;

	/* Select next task: */
	int i = 0;
	do{
		++i;
		m_task_table.current_task++;
		if (m_task_table.current_task >= m_task_table.size)
			m_task_table.current_task = 1; //let's skip idle

	} while(i < m_task_table.size && OS_TASK_STATUS_IDLE != m_task_table.tasks[m_task_table.current_task].status);
	if( i == m_task_table.size ) //if we cycled over the task table, the only suitable task is the idle
	{
		m_task_table.current_task = 0;
	}

	marios_next_task = &m_task_table.tasks[m_task_table.current_task];
	marios_next_task->status = OS_TASK_STATUS_ACTIVE;
}

void marios_delay(uint32_t ticks){
	__disable_irq(); //Here the yield and scheduling must be protected against other incoming interrupts
	m_task_table.tasks[m_task_table.current_task].status = OS_TASK_STATUS_SUSPEND;
	m_task_table.tasks[m_task_table.current_task].wait_ticks = mariosTicks+ticks;
	marios_task_yield();
	__enable_irq();
}
