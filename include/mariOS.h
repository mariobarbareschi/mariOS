#ifndef MARIOS_H
#define MARIOS_H

#include "marios_config.h"

typedef uint32_t marios_stack_t;
typedef uint16_t marios_task_id_t;
void marios_init(void);
marios_task_id_t marios_task_init(void (*handler)(void), uint32_t stack_size);
int marios_start(uint32_t systick_ticks);
void marios_scheduler(void);

void marios_task_yield(void);
void marios_delay(uint32_t ticks);

void marios_systick_handler(void);

#endif
