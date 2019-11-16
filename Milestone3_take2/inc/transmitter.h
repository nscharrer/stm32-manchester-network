#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#include "usart.h"

#define MAX_MESSAGE_LENGTH 30

#define FRAME_OVERHEAD_SIZE 7

#define MAX_FRAME_SIZE MAX_MESSAGE_LENGTH + FRAME_OVERHEAD_SIZE

#define MAX_MANCHESTER_ARR_SIZE MAX_FRAME_SIZE * 2 * 8

void InitTransmitGPIO(void);

void formatDataAndTransmit(char data[], int size);

#endif /* TRANSMITTER_H_ */
