#ifndef DATASTORE_H
#define DATASTORE_H

#include "stm32l1xx.h" 
#include "stdio.h" 

#define  ADC_OFFSET_DEFAULT 0
#define  ADC_LEN_DEFAULT    128  //128
#define  ADC_SPEED_DEFAULT  1  //K
#define  NODE_NUM           2

#define  BOOT_ALREADY       0x44

typedef struct {
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
uint8_t Get_ADC_Node_NUM(void);
#endif
