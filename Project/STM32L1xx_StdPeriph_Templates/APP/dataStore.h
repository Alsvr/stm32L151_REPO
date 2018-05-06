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
    uint8_t terminal_addr[4];     //终端地址 IP
    uint8_t route_addr[4];        //终端对应的协调器地址 IP
    uint8_t node_num;        //终端节点号
    int16_t   ADC_OFFSET;         //终端adc偏移
    int16_t   ADC_LEN;            //终端adc长度 N*1024
    int16_t   ADC_Speed;            //终端adc 采样率
    uint8_t   boot_flag;          //初始化标识
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
