/*
 * port.h
 *
 *  Created on: 29 giu 2018
 *      Author: mariobarbareschi
 */

#ifndef PORT_H_
#define PORT_H_

#include <inttypes.h>

void loadFirstTask();
void SVC_Handler();
void PendSV_Handler() __attribute__ (( naked ));

void yield();
void enterCriticalRegion();
void exitCriticalRegion();

int configureSystick(uint32_t systick_ticks);

#endif /* PORT_H_ */
