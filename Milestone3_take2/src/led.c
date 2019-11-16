/*
 * led.c
 *
 *  Created on: Sep 24, 2019
 *      Author: scharrernf
 */

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include "led.h"
#include "stdio.h"



void InitializeLEDGPIOs(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);


    GPIO_InitTypeDef gpioStructure;
    gpioStructure.GPIO_Pin = GREEN | YELLOW | RED;
    gpioStructure.GPIO_Mode = GPIO_Mode_OUT;
    gpioStructure.GPIO_OType = GPIO_OType_PP;
    gpioStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &gpioStructure);

    GPIO_WriteBit(GPIOB, GREEN | YELLOW | RED, Bit_RESET);

}

void LightLED(uint16_t LED_Pin)
{
	// Light the specified LED and turn the rest off
	if(LED_Pin == GREEN)
	{
		GPIO_ToggleBits(GREEN_BANK, GREEN);
		GPIO_WriteBit(YELLOW_BANK, YELLOW, Bit_RESET);
		GPIO_WriteBit(RED_BANK, RED, Bit_RESET);
	} else if(LED_Pin == YELLOW) {
		GPIO_ToggleBits(YELLOW_BANK, YELLOW);
		GPIO_WriteBit(GREEN_BANK, GREEN, Bit_RESET);
		GPIO_WriteBit(RED_BANK, RED, Bit_RESET);
	} else if(LED_Pin == RED) {
		GPIO_ToggleBits(RED_BANK, RED);
		GPIO_WriteBit(YELLOW_BANK, YELLOW, Bit_RESET);
		GPIO_WriteBit(GREEN_BANK, GREEN, Bit_RESET);
	} else {
		printf("-E- Unknown pin.\n");
	}
}
