/*
 * timer.c
 *
 *  Created on: Sep 23, 2019
 *      Author: scharrernf
 */

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "monitor.h"
#include "receiver.h"
#include "led.h"

extern t_state state;
extern int retransmitting;
extern int retransmission_count;

void InitTimerInterrupts(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	// Get the clock frequencies
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	TIM_TimeBaseInitTypeDef TIMInitStruct;
	// Scale the clock to 1Mhz
	TIMInitStruct.TIM_Prescaler = (RCC_Clocks.PCLK2_Frequency/1000000) - 1;
	TIMInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	// Set period to 1.11ms
	TIMInitStruct.TIM_Period = 1110 - 1;
	TIMInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIMInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIMInitStruct);
	TIM_Cmd(TIM2, ENABLE);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	// Set up the timer interrupts
	NVIC_InitTypeDef NVICInitStructure;
	NVICInitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVICInitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVICInitStructure.NVIC_IRQChannelSubPriority = 1;
	NVICInitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVICInitStructure);
}

void TIM2_IRQHandler()
{
	// Clear the pending bit for the timer
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	// Grab the current bit on the line
	uint8_t current_bit = ReadRX();

	switch(state) {
		case IDLE:
			if(current_bit == 0)
			{
				// Should be error as edge detection should have been caught
			}
			// If it is still 1, then we are still in idle, clock should be reset
			break;
		case BUSY:
			if(current_bit == 0)
			{
				// In BUSY and timed out with the line on 0, so go to COLLISION
				state = COLLISION;
				LightLED(RED);

				if(retransmitting == 0)
				{
					// DisableReceiver gets rid of the message received thus far, so we want to throw out what we had until the collision
					DisableReceiver();
					ClearReceiverArr();
				}

			}
			else
			{
				// Otherwise we timed out with the line on 1, so go back to IDLE
				state = IDLE;
				LightLED(GREEN);
				if(retransmitting == 0)
				{
					ProcessReceivedMessage();
				}

			}

			break;
		case COLLISION:
			// Check if we timed out on a 1, want to go back to Idle if we aren't retransmitting, or if we are and we have delayed
			//		for at least one delay period
			if(current_bit == 1)
			{
				if(retransmitting == 0 || (retransmitting == 1 && retransmission_count > 0))
				{
					state = IDLE;
					LightLED(GREEN);
				}
			}
			break;
		default:
			// TODO raise an error
			break;
	}
}

void ResetTIM2Cnt(void)
{
	TIM_SetCounter(TIM2, 0);
}
