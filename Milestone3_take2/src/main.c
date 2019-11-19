/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f4xx.h"
#include "stm32f4xx_dbgmcu.h"
#include "monitor.h"
#include "timer.h"
#include "led.h"
#include "usart.h"
#include "transmitter.h"
#include "receiver.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


#define STM32_DBGMCU_CR 0xE0042004


volatile t_state state = IDLE;

extern int retransmission_count;
extern int retransmitting;

double rand_num(void)
{
	return (double)rand() / (double)RAND_MAX;
}

void delay_for_retry(double multiplier)
{
	int miliseconds = 1000 * multiplier;

	clock_t start_time = clock();
	while(clock() < start_time + miliseconds);
}


int main(void)
{
	// Enable low power mode debugging
	DBGMCU_Config(DBGMCU_SLEEP, ENABLE);
	DBGMCU_Config(DBGMCU_STOP, ENABLE);
	DBGMCU_Config(DBGMCU_STANDBY, ENABLE);

	// Reset system states
	SystemInit();

	// Initialize LEDs and set Green light on for the IDLE state
	InitializeLEDGPIOs();
	LightLED(GREEN);

	// Initialize the transmission output GPIO pin
	InitTransmitGPIO();

	// Set up the monitor input GPIO and TIM2 along with their respective interrupts
	InitGpioInterrupts();
	InitTimerInterrupts();

	// Initialize the USART for communication with PC
	USART2_Init();

	InitReceiverTimer();

	while(1)
	{
		// TODO potential race condition if we change to idle between the check for it after the delay and before this if statement
		// 		not 100% sure how we would get around this
		if(retransmitting == 1)
		{
			if(retransmission_count < 10)
			{
				delay_for_retry(rand_num());
				retransmission_count++;


				if(state == IDLE)
				{
					retransmitting = 0;
					retransmission_count = 0;
					InitTransmitterTimer();

				}
			}
		}
	}

	return 0;
}
