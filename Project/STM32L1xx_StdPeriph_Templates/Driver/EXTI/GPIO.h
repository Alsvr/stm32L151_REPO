#ifndef __GPIO_H
#define __GPIO_H	 

#include "stm32l1xx_gpio.h"
#include "stm32l1xx_rcc.h"
void Led_Init(void);
void Led_Open(void);
void Led_Close(void);
void Led_toggle(void);
void PowerControl_Init(void);
void LOW_Power_Init(void);
#endif

















