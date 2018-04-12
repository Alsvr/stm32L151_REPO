#ifndef DATASTORE_H
#define DATASTORE_H

#include "stm32l1xx.h" 
#include "stdio.h" 

#define  ADC_OFFSET_DEFAULT 0
#define  ADC_LEN_DEFAULT    4  //K
#define  ADC_SPEED_DEFAULT  1  //K
#define  BOOT_ALREADY       0x45

typedef struct GlobalData_Para{
    uint8_t terminal_addr[4];     //�ն˵�ַ IP
    uint8_t route_addr[4];        //�ն˶�Ӧ��Э������ַ IP
    int16_t   ADC_OFFSET;         //�ն�adcƫ��
    int16_t   ADC_LEN;            //�ն�adc���� N*1024
    int16_t   ADC_Speed;            //�ն�adc ������
    uint8_t   boot_flag;          //��ʼ����ʶ
    uint8_t   CRC_;
    uint32_t  dummy;
}GlobalData_Para;

uint8_t GetGlobalData(void);

#endif
