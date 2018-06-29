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
/***************/
#define UE_REST_SELF_PERIOD30S      (12*60*2)  //节点自我复位的时间



//#define   DETECTION_DEBUG


#ifdef DETECTION_DEBUG
#define UE_SLEEP_TIME_S         10

#define UE_UPDATE_DATA_30S_NUM  6              //上报实时数据的间隔  
#define UE_UPDATE_DATA_30S_EMERGENCY_NUM  2     //警报模式 上报实时数据的间隔

#define UE_UPDATE_DATA_PERIOD_EMERGENCY_NUM  2*UE_UPDATE_DATA_30S_EMERGENCY_NUM     //警报模式 上报实时数据的间隔 持续的间隔数目
#define UE_UPDATE_DATA_PERIOD_PRETECION_NUM  1*UE_UPDATE_DATA_30S_EMERGENCY_NUM    //保护模式 上报实时数据的间隔 下 持续的间隔数目 12*30

#define ACCELEBRATION_THRESHOLD_DEFALUT  1000
#define TEMPERATURE_THRESHOLD_DEFALUT    (30<<4)

#else  // realeas mode
#define UE_SLEEP_TIME_S         30

#define UE_UPDATE_DATA_30S_NUM  20               //上报实时数据的间隔  
#define UE_UPDATE_DATA_30S_EMERGENCY_NUM      2  //警报模式 上报实时数据的间隔

#define UE_UPDATE_DATA_PERIOD_EMERGENCY_NUM   10*UE_UPDATE_DATA_30S_EMERGENCY_NUM //警报模式 上报实时数据的间隔 持续的间隔数目
#define UE_UPDATE_DATA_PERIOD_PRETECION_NUM   12*UE_UPDATE_DATA_30S_NUM          //保护模式 上报实时数据的间隔 下 持续的间隔数目 12*30

#define ACCELEBRATION_THRESHOLD_DEFALUT  1000
#define TEMPERATURE_THRESHOLD_DEFALUT    (65<<4)

#endif


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
    int16_t   ADC_threshold;      //adc 阈值
    int16_t   temperature_threshold;      //温度阈值
    uint8_t   boot_flag;          //初始化标识
    uint32_t  dummy;
    uint8_t   CRC_;

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
void Set_Node_Temperature_threshold(int16_t temp);
void Set_Node_accelebration_threshold(uint16_t temperature_threshold);
int16_t Get_Node_Temperature_threshold(void);
uint16_t Get_Node_accelebration_threshold(void);

#endif
