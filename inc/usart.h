#ifndef USART_H_
#define USART_H_

// Max input size from the USART
#define MAX_MESSAGE_LENGTH 30

void USART2_Init(void);

void USART2_SendData(char* data, int size);

#endif /* USART_H_ */
