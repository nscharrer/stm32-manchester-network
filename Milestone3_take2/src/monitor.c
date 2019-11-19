/*
 * monitor.c
 *
 *  Created on: Sep 22, 2019
 *      Author: scharrernf
 */

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "monitor.h"
#include "led.h"
#include "timer.h"
#include "receiver.h"
#include "transmitter.h"

extern t_state state;
extern int manchester_rec_arr_index;
extern char manchester_rec_arr[MAX_MANCHESTER_ARR_SIZE];
extern int retransmitting;


void InitGpioInterrupts(void)
{
	// Define initialization structures
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;

	// Enable clock for GPIO C
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	//Enable clock for SYSCFG
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// Set Pin 8 as input
	// Set GPIO to input mode
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	// Set output type as Push Pull (doesn't matter, we are in input mode)
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	// Pin 8
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	// Set to pull-up mode
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	// Set to 100MHz
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	// Init the GPIO
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	// Tell system that you will use PC8 for EXTI_Line8
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource8);

	// PC8 connected to EXTI8
	EXTI_InitStruct.EXTI_Line = EXTI_Line8;
	// Enable interrupt
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	// Interrupt mode
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	// Triggers on rising and falling edge
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	// Add to EXTI
	EXTI_Init(&EXTI_InitStruct);

	// Add IRQ vector to NVIC
	// PC8 is connected to EXTI_Line8, which has EXTI9_5_IRQn vector
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	// Set priority
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	// Set sub priority
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	// Enable interrupt
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	// Add to NVIC
	NVIC_Init(&NVIC_InitStruct);

}

uint8_t ReadRX(void)
{
	// Read the current value on the GPIO input line
	return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8);
}

void EXTI9_5_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line8) != RESET) {
		// Do your stuff when PC8 is changed

		// Grab the current bit off the line
		//TODO will be used in the future
		uint8_t current_bit = ReadRX();

		switch(state) {
			case IDLE:
				if(retransmitting == 0)
				{
					// Caught an edge when in IDLE, now must be BUSY
					state = BUSY;
					LightLED(YELLOW);

					InitReceiverArr();
					EnableReceiver();

					manchester_rec_arr[manchester_rec_arr_index] = current_bit;
					manchester_rec_arr_index++;
				}

				break;
			case BUSY:
				if(retransmitting == 0)
				{
					// Edge caught while already in busy, stay there and grab the bit for the receiver
					manchester_rec_arr[manchester_rec_arr_index] = current_bit;
					manchester_rec_arr_index++;
				}

				break;
			case COLLISION:
				if(retransmitting == 0)
				{
					// Caught an edge in the COLLISION state, must now be back to BUSY
					state = BUSY;
					LightLED(YELLOW);

					InitReceiverArr();
					EnableReceiver();
				}
				break;
			default:
				//TODO raise an error
				break;
		}

		// Reset the TIM2 timer as a new edge was detected
		ResetTIM2Cnt();
		ResetTIM4Cnt();

		//Clear interrupt flag
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}

