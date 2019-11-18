/*
 * receiver.c
 *
 *  Created on: Oct 23, 2019
 *      Author: scharrernf
 */

#include "monitor.h"
#include "transmitter.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "receiver.h"
#include "hashmap.h"
#include <stdio.h>
#include <string.h>

volatile char manchester_rec_arr[MAX_MANCHESTER_ARR_SIZE];
volatile int manchester_rec_arr_index;

void InitReceiverTimer(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	// Get the clock frequencies
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	TIM_TimeBaseInitTypeDef TIMInitStruct;
	// Scale the clock to 1Mhz
	TIMInitStruct.TIM_Prescaler = (RCC_Clocks.PCLK2_Frequency/1000000) - 1;
	TIMInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	// Set period to 0.700 ms
	TIMInitStruct.TIM_Period = 700 - 1;
	TIMInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIMInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIMInitStruct);


	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

	// Set up the timer interrupts
	NVIC_InitTypeDef NVICInitStructure;
	NVICInitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVICInitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVICInitStructure.NVIC_IRQChannelSubPriority = 4;
	NVICInitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVICInitStructure);

	NVIC_DisableIRQ(TIM4_IRQn);

	ClearReceiverArr();
}

void TIM4_IRQHandler()
{
	// Clear the pending bit for the timer
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

	// Grab the current bit on the line
	uint8_t current_bit = ReadRX();

	// Should still be busy if this timer gets to the end
	// Thus we pull the current bit and add it to the array and go on
	manchester_rec_arr[manchester_rec_arr_index] = current_bit;

	manchester_rec_arr_index++;
}

void ResetTIM4Cnt(void)
{
	TIM_SetCounter(TIM4, 0);
}

void InitReceiverArr(void)
{
	manchester_rec_arr_index = 0;
	manchester_rec_arr[manchester_rec_arr_index] = 1;
	manchester_rec_arr_index++;

}

void ClearReceiverArr(void)
{
	for(int i = 0; i < MAX_MANCHESTER_ARR_SIZE; i++)
	{
		manchester_rec_arr[i] = 0;
	}
	manchester_rec_arr_index = 0;
}

void EnableReceiver(void)
{

	TIM_Cmd(TIM4, ENABLE);
	NVIC_EnableIRQ(TIM4_IRQn);
}

void DisableReceiver(void)
{
	TIM_Cmd(TIM4, DISABLE);
	NVIC_DisableIRQ(TIM4_IRQn);
}

int CheckHeader(char* rec_arr)
{
	if(rec_arr[0] != 0x55)
	{
		return -1;
	}

	if(rec_arr[1] != 0x01)
	{
		return -1;
	}

	char source = rec_arr[2];

	if(rec_arr[3] != MY_ADDRESS)
	{
		return 0;
	}

	char length = rec_arr[4];

	if(rec_arr[5] != 0x01)
	{
		//todo CRC Flag - not sure if this is constant
		return -1;
	}


	return length;
}

int CheckCRC(char crc_fcs)
{
	//todo more elaborate checking in future
	if(crc_fcs == 0xFF)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void ProcessReceivedMessage(void)
{
	DisableReceiver();

	// Adjust the received array, the last value is extraneous from the long timer not timing out yet
	manchester_rec_arr[manchester_rec_arr_index - 1] = 0x0;
	manchester_rec_arr_index--;

	struct map *manchester_decode_table = initManchesterDecodeMap();

	// Initialize and clear the receive array
	char rec_arr[MAX_MANCHESTER_ARR_SIZE / 2];
	int rec_index = 0;
	for(int i = 0; i < MAX_MANCHESTER_ARR_SIZE / 2; i++)
	{
		rec_arr[i] = 0;
	}

	int shift = 0;
	char current_char = 0x0;
	// convert the manchester array that we received to the translated hex array
	for(int i = 0; i < manchester_rec_arr_index; i++)
	{
		current_char = current_char | manchester_rec_arr[i]<<shift;
		if(shift == 7)
		{
			rec_arr[rec_index] = lookup(manchester_decode_table, current_char);
			rec_index++;
			shift = 0;
			current_char = 0x0;
		}
		else
		{
			shift++;
		}
	}

	int j = 0;
	for(int i = 0; i < rec_index; i = i + 2)
	{
		current_char = 0x0;
		current_char = rec_arr[i]<<4 | rec_arr[i+1];
		rec_arr[j] = current_char;
		j++;
	}

	for(j; j < rec_index; j++)
	{
		rec_arr[j] = 0;
	}

	int length = CheckHeader(rec_arr);

	if(length == -1)
	{
		// error in message formatting, raise error
	}
	else if (length == 0)
	{
		// not my message, discard it and reset arrays
	}
	else
	{
		// my message, get the content, check the trailer, and print the message through the usart
		char message[length];
		for(int i = 0; i < length; i++)
		{
			message[i] = rec_arr[i + 6];
		}

		char crc_fcs = rec_arr[length + 6];

		if(CheckCRC(crc_fcs) == 0)
		{
			int send_message_size = 20 + length;
			char send_message[send_message_size];
			sprintf(send_message, "Received Message: %s", message);

			send_message[send_message_size - 2] = '\r';
			send_message[send_message_size - 1] = '\n';

			//FCS check pass, print the message
			USART2_SendData(send_message, send_message_size);
		}
		else
		{
			//FCS check fails, raise error and don't print the message
			printf("FCS Check Fail.\n");
		}
	}
	ClearReceiverArr();

}
