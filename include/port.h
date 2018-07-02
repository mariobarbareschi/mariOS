/*
 * port.h
 *
 *  Created on: 29 giu 2018
 *      Author: mariobarbareschi
 */

#ifndef PORT_H_
#define PORT_H_

void loadFirstTask();
void SVC_Handler();
void PendSV_Handler() __attribute__ (( naked ));

void enterCriticalRegion();
void exitCriticalRegion();

#endif /* PORT_H_ */
