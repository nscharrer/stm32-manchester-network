/*
 * monitor.h
 *
 *  Created on: Sep 22, 2019
 *      Author: scharrernf
 */

#ifndef MONITOR_H_
#define MONITOR_H_

#include <stdint.h>

typedef enum {
	IDLE,
	BUSY,
	COLLISION
} t_state;

void InitGpioInterrupts(void);

uint8_t ReadRX(void);


#endif /* MONITOR_H_ */
