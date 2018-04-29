#ifndef _WIRELESS_H
#define _WIRELESS_H

#include "stm32l1xx_gpio.h"
#include "stm32l1xx_rcc.h"
#include "delay.h"


#define SERVER_TO_NODE_CMD_START_ADC     0x01
#define SERVER_TO_NODE_CMD_SLEEP_TIME    0x02



#define NODE_TO_SERVER_INST_HEART       0x01
#define NODE_TO_SERVER_INST_ADC_DATA    0x02

typedef struct
{
	uint8_t HEAD;
	uint8_t ADDH;
	uint8_t ADDL;
	uint8_t SPED;
	uint8_t CHAN;
	uint8_t OPTION;
}WireLess_DATA;

typedef struct{
    unsigned char header1;
    unsigned char header2;
    unsigned char length;
    unsigned char instru;
    unsigned char node_addr;
    unsigned char commend1;
    unsigned char commend2;
    unsigned char commend3;
    unsigned char commend4;
    unsigned char data[128];
    unsigned char check_sum;
    unsigned char tail1;
    unsigned char tail2;
}Node_Instru_Packet;




void init_wireless(	uint8_t HEAD,
					uint16_t ADD,
					uint32_t baund,
					uint8_t channel);
void Init_CC3200(uint8_t HEAD,
                        uint16_t ADD,
                        uint32_t baund,
                        uint8_t channel);

void Wireless_wakeup_mode(void);
void Wireless_power_down(void);
void Wireless_power_on(void);
uint8_t WiFi_WaitLinkOk(void);
uint8_t WiFi_Exit_CMD_mode(void);
uint8_t WiFi_Enter_CMD_mode(void);
uint8_t WireLess_Send_data(Node_Instru_Packet *node_instru_packet,uint32_t len );
uint8_t  WiFi_GetUDPData(void);
uint8_t WireLess_Send_ADC_data(void);


#endif
