#ifndef _WIRELESS_H
#define _WIRELESS_H

#include "stm32l1xx_gpio.h"
#include "stm32l1xx_rcc.h"
#include "delay.h"

typedef struct
{
	uint8_t HEAD;
	uint8_t ADDH;
	uint8_t ADDL;
	uint8_t SPED;
	uint8_t CHAN;
	uint8_t OPTION;
}WireLess_DATA;

void init_wireless(	uint8_t HEAD,
					uint16_t ADD,
					uint32_t baund,
					uint8_t channel);
void Wireless_wakeup_mode(void);
void Wireless_power_down(void);
void Wireless_power_on(void);
#endif
