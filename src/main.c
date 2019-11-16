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


#define STM32_DBGMCU_CR 0xE0042004


volatile t_state state = IDLE;

void tester(void)
{
	char data[] = {'H', 'E', 'L', 'L', 'O', '\n', '\r', '\0'};
	//USART2_SendData(data, strlen(data));
	//printf("%s", data);
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

	tester();
	while(1)
	{
	}

	return 0;
}
