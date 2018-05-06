#ifndef DATASTORE_H
#define DATASTORE_H

#include "stm32l1xx.h" 
#include "stdio.h" 
#include "stdint.h" 
#define  ADC_OFFSET_DEFAULT 0
#define  ADC_LEN_DEFAULT    64  //128
#define  ADC_SPEED_DEFAULT  1000  //K
#define  NODE_NUM           1

#define  BOOT_ALREADY       0x58

typedef struct {
    uint8_t ssid[16]; 
    uint8_t terminal_addr[4];     //�ն˵�ַ IP
    uint8_t route_addr[4];        //�ն˶�Ӧ��Э������ַ IP
    uint8_t node_num;        //�ն˽ڵ��
    int16_t   ADC_OFFSET;         //�ն�adcƫ��
    int16_t   ADC_LEN;            //�ն�adc���� N*1024
    int16_t   ADC_Speed;            //�ն�adc ������
    uint8_t   boot_flag;          //��ʼ����ʶ
    uint8_t   CRC_;
    uint32_t  dummy;
}GlobalData_Para;

GlobalData_Para* GetGlobalData(void);
uint16_t Get_ADC_LEN(void);
uint8_t Get_Node_NUM(void);
uint8_t Set_Node_NUM(uint8_t num);
void SetWifiSSID(uint8_t* str,uint8_t len);
uint8_t SetGlobalData(void);
void SetADCLen(uint16_t Len);
uint8_t* GetWifiSSID(void);
void SetADCSpeed(uint16_t speed); 
uint16_t GetADCSpeed(void);  

#endif
