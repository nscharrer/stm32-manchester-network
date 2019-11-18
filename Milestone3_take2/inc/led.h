/*
 * led.h
 *
 *  Created on: Sep 22, 2019
 *      Author: scharrernf
 */

#ifndef LED_H_
#define LED_H_

#define GREEN_BANK GPIOB
#define YELLOW_BANK GPIOB
#define RED_BANK GPIOB

#define	GREEN GPIO_Pin_3
#define YELLOW GPIO_Pin_4
#define RED GPIO_Pin_5

void InitializeLEDGPIOs();

void LightLED(uint16_t LED_Pin);

#endif /* LED_H_ */
