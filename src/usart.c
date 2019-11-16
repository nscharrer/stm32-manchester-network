/*
 * usart.c
 *
 *  Created on: Oct 3, 2019
 *      Author: scharrernf
 */

#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "transmitter.h"
#include "usart.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>



static char usart_buffer[MAX_MESSAGE_LENGTH];
static int buf_index;

void USART2_Init(void)
{
	// Set initial index to 0
	buf_index = 0;

	// Define structures for peripheral initialization
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// GPIO's in alternate function mode for USART2
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// Set the alternate function for GPIOA pins 2 and 3 to USART2 and init the pins
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitTypeDef usart2_init_struct;
	// Enable USART2
	USART_Cmd(USART2, ENABLE);

	// Set baud rate to 19200, 8bit word length, 1 stop bit, no parity, Rx and Tx, and no flow control
	usart2_init_struct.USART_BaudRate = 19200;
	usart2_init_struct.USART_WordLength = USART_WordLength_8b;
	usart2_init_struct.USART_StopBits = USART_StopBits_1;
	usart2_init_struct.USART_Parity = USART_Parity_No;
	usart2_init_struct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart2_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

	USART_Init(USART2, &usart2_init_struct);

	// Enable the USART RX Interrupt
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	// Add interrupt to the NVIC
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void USART2_IRQHandler(void)
{
    // Rx handler
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        // grab the character
    	char rec = (char)USART_ReceiveData(USART2);

    	// If not a carriage return or new line, add to the buffer, otherwise send and reset the index
    	if (rec != '\r' && rec != '\n') {
    		usart_buffer[buf_index] = rec;
    		buf_index += 1;
    	} else {
    		formatDataAndTransmit(usart_buffer, buf_index);
    		buf_index = 0;
    		memset(usart_buffer, 0, sizeof(usart_buffer));
    	}

    	// If the index is now at the size limit, send the data and reset the index
    	if (buf_index == MAX_MESSAGE_LENGTH) {
    		formatDataAndTransmit(usart_buffer, buf_index);
    		buf_index = 0;
    		memset(usart_buffer, 0, sizeof(usart_buffer));
    	}

    	// Should already have cleared the pending bit by calling USART_ReceiveData(), but double check it
    	USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

void USART2_SendData(char* data, int size)
{
	for(int i = 0; i < size; i++)
	{
		while (!(USART2->SR & USART_SR_TXE))
		{
			//Wait for transmit enable to be ready
		}
		USART_SendData(USART2, data[i]);
	}
}
