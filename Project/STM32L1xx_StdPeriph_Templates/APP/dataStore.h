#ifndef DATASTORE_H
#define DATASTORE_H

#include "stm32l1xx.h" 
#include "stdio.h" 
#include "stdint.h" 

#define BIT_0                 ((uint16_t)0x0001)  /*!< Pin 0 selected */
#define BIT_1                 ((uint16_t)0x0002)  /*!< Pin 1 selected */
#define BIT_2                 ((uint16_t)0x0004)  /*!< Pin 2 selected */
#define BIT_3                 ((uint16_t)0x0008)  /*!< Pin 3 selected */
#define BIT_4                 ((uint16_t)0x0010)  /*!< Pin 4 selected */
#define BIT_5                 ((uint16_t)0x0020)  /*!< Pin 5 selected */
#define BIT_6                 ((uint16_t)0x0040)  /*!< Pin 6 selected */
#define BIT_7                 ((uint16_t)0x0080)  /*!< Pin 7 selected */
#define BIT_8                 ((uint16_t)0x0100)  /*!< Pin 8 selected */
#define BIT_9                 ((uint16_t)0x0200)  /*!< Pin 9 selected */
#define BIT_10                ((uint16_t)0x0400)  /*!< Pin 10 selected */
#define BIT_11                ((uint16_t)0x0800)  /*!< Pin 11 selected */
#define BIT_12                ((uint16_t)0x1000)  /*!< Pin 12 selected */
#define BIT_13                ((uint16_t)0x2000)  /*!< Pin 13 selected */
#define BIT_14                ((uint16_t)0x4000)  /*!< Pin 14 selected */
#define BIT_15                ((uint16_t)0x8000)  /*!< Pin 15 selected */



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
