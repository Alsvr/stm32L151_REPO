#ifndef DATASTORE_H
#define DATASTORE_H

#include "stm32l1xx.h" 
#include "stdio.h" 

#define  ADC_OFFSET_DEFAULT 0
#define  ADC_LEN_DEFAULT    4  //K
#define  ADC_SPEED_DEFAULT  1  //K
#define  BOOT_ALREADY       0x45

typedef struct GlobalData_Para{
    uint8_t terminal_addr[4];     //终端地址 IP
    uint8_t route_addr[4];        //终端对应的协调器地址 IP
    int16_t   ADC_OFFSET;         //终端adc偏移
    int16_t   ADC_LEN;            //终端adc长度 N*1024
    int16_t   ADC_Speed;            //终端adc 采样率
    uint8_t   boot_flag;          //初始化标识
    uint8_t   CRC_;
    uint32_t  dummy;
}GlobalData_Para;

uint8_t GetGlobalData(void);

#endif
