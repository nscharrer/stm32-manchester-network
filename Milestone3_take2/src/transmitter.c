/*
 * transmitter.c
 *
 *  Created on: Oct 7, 2019
 *      Author: scharrernf
 */
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "hashmap.h"
#include "monitor.h"
#include "transmitter.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


extern t_state state;

static char manchester_arr[MAX_MANCHESTER_ARR_SIZE];
static int manchester_len;
static int manchester_index;
static int char_index;

// Frame we are trying to send
#define FRAME_PREAMBLE 0x55
#define FRAME_VERSION 0x01
#define FRAME_SRC 0x20

// static char frame_source;
static char frame_dest;
static char frame_length;
static char frame_crc_flag;
static char frame_crc_fcs;
static char frame[MAX_FRAME_SIZE];

void buildFrame(char *data, int size)
{
	frame[0] = FRAME_PREAMBLE;
	frame[1] = FRAME_VERSION;
	frame[2] = FRAME_SRC;
	frame[3] = frame_dest;
	frame[4] = frame_length;
	frame[5] = frame_crc_flag;

	int length = size;
	for(int i = 0; i < length; i++)
	{
		frame[i + 6] = data[i];
	}

	frame[length + 6] = frame_crc_fcs;
}

void calculateAndSetCRC(void)
{
	// todo next milestone
	frame_crc_flag = 0x01;
	frame_crc_fcs = 0xFF;
}

void setFrameDest(char dest)
{
	// todo next milestone
	frame_dest = dest;
}

void setFrameLength(int size)
{
	frame_length = 0x0;
	for(int i = 0; i < size; i++)
	{
		frame_length++;
	}
}

void InitTransmitterTimer(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	// Get the clock frequencies
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	TIM_TimeBaseInitTypeDef TIMInitStruct;
	// Prescale the clock to 1MHz
	TIMInitStruct.TIM_Prescaler = (RCC_Clocks.PCLK2_Frequency/1000000) - 1;
	TIMInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	// Set the period to 0.5ms or 500us
	TIMInitStruct.TIM_Period = 500 - 1;
	TIMInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIMInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIMInitStruct);
	TIM_Cmd(TIM3, ENABLE);

	// Enable the timer
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	// Set the timer interrupts
	NVIC_InitTypeDef NVICInitStructure;
	NVICInitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVICInitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVICInitStructure.NVIC_IRQChannelSubPriority = 3;
	NVICInitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVICInitStructure);
}

void ResetTIM3Cnt(void)
{
	TIM_SetCounter(TIM3, 0);
}

void DisableTIM3(void)
{
	TIM_Cmd(TIM3, DISABLE);
	ResetTIM3Cnt();
}

void TIM3_IRQHandler()
{
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	if(state == COLLISION)
	{
		// Collision hit, need to pause/stop transmission
		//TODO need to implement waiting and re-transmitting
		// For now, we will just halt the transmission and clear the array
		DisableTIM3();
		manchester_len = 0;
		manchester_index = 0;
		char_index = 0;
		memset(manchester_arr, 0, sizeof(manchester_arr));
	}
	else if(manchester_index == manchester_len)
	{
		// Index is equal to the length of the manchester array, need to stop transmission
		// End the timer
		DisableTIM3();

		// Return the line to High to indicate IDLE
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);

		// Reset the length, index, char_index, and clear the array
		manchester_len = 0;
		manchester_index = 0;
		char_index = 0;
		memset(manchester_arr, 0, sizeof(manchester_arr));
	}
	else
	{
		// Transmit the current bit
		// Grab the byte we are on
		char current_byte = manchester_arr[manchester_index];

		// Get the current bit - shift the byte right until in the first position and then mask out all other bits
		int current_bit = current_byte>>(7-char_index) & 0x1;

		// If the bit is a 1, set the line high, otherwise set it low
		if(current_bit == 1)
		{
			GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
		}
		else
		{
			GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
		}

		// update the char index to get the next bit
		char_index += 1;

		// if the index is at 8 (past the end of the char), increment the array index and reset the char index
		if(char_index == 8)
		{
			manchester_index += 1;
			char_index = 0;
		}

	}
}

void InitTransmitGPIO(void)
{
	// Define initialization structures
	GPIO_InitTypeDef GPIO_InitStruct;

	// Enable clock for GPIO C
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	// Set Pin 6 as output
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	// Set output type as Push Pull
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	// Pin 6
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	// Set to 100MHz
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	// Add to GPIO
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	// TODO remove this - simulating the line going high for IDLE, but might have stayed low if we hit COLLISION
	GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
}

void formatDataAndTransmit(char *data, int size)
{
	setFrameLength(size);
	setFrameDest(0x20);
	calculateAndSetCRC();

	buildFrame(data, size);

	int table_index = 0;
	manchester_len = 0;
	manchester_index = 0;
	char_index = 0;
	struct map *manchester_table = initManchesterMap();

	// Loop through the full frame grabbing each hex value, getting its Manchester encoding, and adding to the array
	for(int i = 0; i < strlen(frame); i++)
	{
		char upper = frame[i]>>4;
		char lower = frame[i] & 0x0F;
		manchester_arr[table_index] = lookup(manchester_table, upper);
		manchester_arr[table_index+1] = lookup(manchester_table, lower);
		table_index += 2;
	}

	manchester_len = table_index;

	// Begin the sending timer if the line is IDLE
	if(state == IDLE)
	{
		InitTransmitterTimer();
	}
	else
	{
		//TODO wait pseudorandom amount of time and then try to re-transmit message
		// For now, just clear the array, will need the user to type another message
		memset(manchester_arr, 0, sizeof(manchester_arr));
	}
}
