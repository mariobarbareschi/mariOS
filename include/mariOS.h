#ifndef MARIOS_H
#define MARIOS_H

#include "marios_config.h"
#include "port.h"

typedef uint32_t marios_stack_t;
typedef uint16_t marios_task_id_t;

typedef enum
{
	MARIOS_TASK_STATUS_READY = 1,
	MARIOS_TASK_STATUS_ACTIVE,
	MARIOS_TASK_STATUS_WAIT,
	MARIOS_TASK_STATUS_SUSPEND
} marios_task_status_t;

volatile uint32_t mariosTicks;

typedef struct
{
	/* The stack pointer (sp) has to be the first element as it is located
	   at the same address as the structure itself (which makes it possible
	   to locate it safely from assembly implementation of PendSV_Handler).
	   The compiler might add padding between other structure elements. */
	volatile uint32_t sp;
	void (*handler)(void);
	volatile marios_task_status_t status;
	volatile uint32_t timer;
	volatile uint32_t wait_ticks;
} marios_task_t;

void marios_init(void);
marios_task_id_t marios_task_init(void (*handler)(void), uint32_t stack_size);
int marios_start(uint32_t systick_ticks);
void marios_scheduler(void);

void marios_task_yield(void);
void marios_delay(uint32_t ticks);

void marios_systick_handler(void);

unsigned int get_current_task_id(void);
void set_current_task_status(marios_task_status_t status);
void set_task_status(unsigned int task_id, marios_task_status_t status);
marios_task_status_t get_current_task_status(void);
marios_task_status_t get_task_status(unsigned int task_id);


#endif
