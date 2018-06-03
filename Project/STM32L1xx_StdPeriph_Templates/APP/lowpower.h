#ifndef __lowpower_H
#define __lowpower_H 			   
#include "stm32l1xx.h" 	


void Enter_Stop_Mode(void);
void To_Exit_Stop(void);

void RTC_Config(uint8_t second);

#endif





























