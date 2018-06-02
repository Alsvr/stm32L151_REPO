#ifndef _WIRELESS_H
#define _WIRELESS_H

#include "stm32l1xx_gpio.h"
#include "stm32l1xx_rcc.h"
#include "delay.h"
#include "dataStore.h"


#define NODE_INSTRU_HEAD1       0xFC
#define NODE_INSTRU_HEAD2       0xFE

#define NODE_INSTRU_TAIL1       0xFA
#define NODE_INSTRU_TAIL2       0xFB

#define SERVER_TO_NODE_CMD     0x01

#define SERVER_TO_NODE_CMD_START_ADC      BIT_0
#define SERVER_TO_NODE_CMD_SET_ADC        BIT_1
#define SERVER_TO_NODE_CMD_CON_SAMP       BIT_2
#define SERVER_TO_NODE_CMD_SAMP_POWER     BIT_3
#define SERVER_TO_NODE_CMD_SET_THRESHOLD  BIT_4



//#define SERVER_TO_NODE_CMD_SLEEP_TIME    0x20



#define NODE_TO_SERVER_INST_HEART            0x10
#define NODE_TO_SERVER_INST_ADC_DATA         0x20
#define NODE_TO_SERVER_INST_REPORT_STATUS    0x40

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

typedef struct{
    unsigned char header1;
    unsigned char header2;
    unsigned char length;
    unsigned char instru;
    unsigned char node_addr;
    unsigned char adc_valid;
    unsigned char adc_config_valid;
    unsigned char continue_sample_valid;
    unsigned char power_sample_valid;
    unsigned char adc_len[2];
    unsigned char adc_speed[2];
    unsigned char adc_continue_packet[2];
    unsigned char thres_hold_valid[2];
    unsigned char thres_hold_accelebration[2];
    unsigned char thres_hold_temperature[2];
    unsigned char data[116];
    unsigned char check_sum;
    unsigned char tail1;
    unsigned char tail2;
}Server_Instru_Packet;

typedef struct {
	unsigned char header1;
	unsigned char header2;
	unsigned char length;
	unsigned char instru;
	unsigned char node_addr;
	unsigned char commend1;
	unsigned char commend2;
	unsigned char commend3;
	unsigned char commend4;
	unsigned char power[2];
	unsigned char temp_adc_num[2];
	unsigned char acceleration_data[40];
	unsigned char temperature_data[40];
	unsigned char data[42];
	unsigned char udp_index[2];
	unsigned char check_sum;
	unsigned char tail1;
	unsigned char tail2;
}Node_Report_Packet;






void init_wireless(	uint8_t HEAD,
					uint16_t ADD,
					uint32_t baund,
					uint8_t channel);
void Init_CC3200(uint8_t first,
                uint32_t baund);

void Wireless_wakeup_mode(void);
void Wireless_power_down(void);
void Wireless_power_on(void);
uint8_t WiFi_WaitLinkOk(void);
uint8_t WiFi_Exit_CMD_mode(void);
uint8_t WiFi_Enter_CMD_mode(void);
uint8_t WireLess_Send_data(Node_Instru_Packet *node_instru_packet,uint32_t len );
uint8_t  WiFi_GetUDPData(void);
uint8_t WireLess_Send_ADC_data(void);
uint8_t WiFi_Send_Report(Node_Instru_Packet *node_instru_packet,
                        uint16_t temp,uint16_t adc1,uint16_t adc_2,
                        uint16_t power,uint8_t addr,
                        uint16_t udp_index);

uint8_t WiFi_EnterLowPowerMode(void);
uint8_t WiFi_EnterPowerDownMode(void);
uint8_t WiFi_Send_Report_new(Node_Report_Packet *node_instru_packet);

void Reset_CC3200(void);
uint8_t Wireless_Get_link_status(void);

#endif
