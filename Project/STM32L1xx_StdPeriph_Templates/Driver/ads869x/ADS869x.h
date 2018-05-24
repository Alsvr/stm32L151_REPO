#ifndef __ADS869x_H
#define __ADS869x_H	 
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_rcc.h"

//#define ADC_DEBUG
//PB15 DO0
//PB14 DO1
//PB13 RVS
//PB5  CON
//PB4  RST
//PB3  SDI
//PA8  SCLK

#define ADS869x_DO1_GPIO    GPIO_Pin_14
#define ADS869x_DO1_GPIO_PORT    GPIOB

#define ADS869x_RVS_GPIO    GPIO_Pin_13
#define ADS869x_RVS_GPIO_PORT    GPIOB

#define ADS869x_MISO_GPIO    GPIO_Pin_15
#define ADS869x_MISO_GPIO_PORT    GPIOB

#define ADS869x_CS_GPIO    GPIO_Pin_5
#define ADS869x_CS_GPIO_PORT    GPIOB

#define ADS869x_MOSI_GPIO    GPIO_Pin_3
#define ADS869x_MOSI_GPIO_PORT    GPIOB

#define ADS869x_SCLK_GPIO    GPIO_Pin_8
#define ADS869x_SCLK_GPIO_PORT    GPIOA

#define ADS869x_RST_GPIO    GPIO_Pin_4
#define ADS869x_RST_GPIO_PORT    GPIOB



#define ADS869x_RVS         (ADS869x_RVS_GPIO_PORT->IDR & ADS869x_RVS_GPIO)  // GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) 
#define ADS869x_MISO        (ADS869x_MISO_GPIO_PORT->IDR & ADS869x_MISO_GPIO) //GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) 

#define ADS869x_CS_Clr()    ADS869x_CS_GPIO_PORT->BSRRH = ADS869x_CS_GPIO   //GPIO_ResetBits(GPIOB,GPIO_Pin_5)
#define ADS869x_CS_Set()    ADS869x_CS_GPIO_PORT->BSRRL = ADS869x_CS_GPIO                        //GPIO_SetBits(GPIOB,GPIO_Pin_5)

#define ADS869x_MOSI_Clr()  ADS869x_MOSI_GPIO_PORT->BSRRH = ADS869x_MOSI_GPIO //GPIO_ResetBits(GPIOB,GPIO_Pin_3)
#define ADS869x_MOSI_Set()  ADS869x_MOSI_GPIO_PORT->BSRRL = ADS869x_MOSI_GPIO //GPIO_SetBits(GPIOB,GPIO_Pin_3)

#define ADS869x_SCLK_Clr()  ADS869x_SCLK_GPIO_PORT->BSRRH = ADS869x_SCLK_GPIO //GPIO_ResetBits(GPIOA,GPIO_Pin_8)
#define ADS869x_SCLK_Set()  ADS869x_SCLK_GPIO_PORT->BSRRL = ADS869x_SCLK_GPIO //GPIO_SetBits(GPIOA,GPIO_Pin_8)

#define ADS869x_RST_Clr()   ADS869x_RST_GPIO_PORT->BSRRH = ADS869x_RST_GPIO  //GPIO_ResetBits(GPIOB,GPIO_Pin_4)
#define ADS869x_RST_Set()   ADS869x_RST_GPIO_PORT->BSRRL = ADS869x_RST_GPIO //GPIO_SetBits(GPIOB,GPIO_Pin_4)

#define ADC_PACKET_SIZE   128
#define ADC_LEN_PACK      64


//define commands

#define ADS869x_NOP           0x00     // No operation
#define ADS869x_CLEAR_HWORD   0xC0     // Any bit marked 1 in the data field results in that particular bit of the specified register being reset to 0, leaving the other bits unchanged.
#define ADS869x_READ_HWORD    0xC8     // the device sends out 16 bits of the register in the next frame
#define ADS869x_READ          0x48     // only 8 bits of the register are returned in the next frame
#define ADS869x_WRITE         0xD0     // 16 bit of input data are written into the specified address
#define ADS869x_WRITE_H       0xD2     // only the MS byte of the 16-bit data word is written 
#define ADS869x_WRITE_L       0xD4     // only the LS byte of the 16-bit data word is written
#define ADS869x_SET_HWORD     0xD8     // Any bit marked 1 in the data field results in that particular bit of the specified register being set to 1, leaving the other bits unchanged

//define the ADS869x register values 
#define ADS869x_DEVICE_ID     0x00      //Device ID register
#define ADS869x_RST_PWRCTL    0x04      //Reset and power control register
#define ADS869x_SDI_CTL       0x08      //SDI data input control register
#define ADS869x_SDO_CTL       0x0C      //SDO-x data input control register
#define ADS869x_DATAOUT_CTL   0x10      //Output data control register
#define ADS869x_RANGE_SEL     0x14      //Input range selection control register
#define ADS869x_ALARM         0x20      //ALARM output register
#define ADS869x_ALARM_H_TH    0x24      //ALARM high threshold and hysteresis register
#define ADS869x_ALARM_L_TH    0x28      //ALARM low threshold register

#define ADS869x_RANGE_0       0x00      //Input range + - 3*ref         + - 12.288 93.75uV
#define ADS869x_RANGE_1       0x01      //Input range + - 2.5*ref       + - 10.24  78.125uV
#define ADS869x_RANGE_2       0x02      //Input range + - 1.5*ref				+ - 6.144  46.875uV
#define ADS869x_RANGE_3       0x03      //Input range + - 1.25*ref			+ - 5.12   39.06uV
#define ADS869x_RANGE_4       0x04  		//Input range + - 0.625*ref			+ - 2.56   19.53uV


unsigned long Get_ADC_Value(void);														 	//获取ADC的值
unsigned long Get_Vol(void);      															//获取电压值
void ADS869x_Init(void);        																//ADS869x初始化
unsigned char ADS869x_ReadWrite(unsigned char dat);							//ADS869x读写8位数据
void ADS869x_SendCMD(unsigned char cmd,uint8_t addr);     //程序功能：ADS869x_SendCMD  写命令
unsigned long ADS869x_ReadREG(unsigned char addr);            	//读ADS869x寄存器 32位数据
void ADS869x_WriteREG(unsigned char regaddr,unsigned short dat);//ADS869x_WriteREG 写寄存器 16位
uint32_t ADS869x_ReadData(void);                           //ADS869x读 32 位数据
void ADS869x_Over_Judgment(void);                               //判断过压，欠压

void ADS869x_GO_NAP(void);																			//进入睡眠模式
void ADS869x_NAP_EXIT(void);																			//退出睡眠模式
void ADS869x_GO_PD(void); 

void ADS869x_PD_EXIT(void);																			//退出PD模式
void ADS869x_Start_Sample(void);
uint8_t ADS869x_Start_Sample_little(uint16_t *adc1,uint16_t *adc2);
uint8_t ADS869x_Start_Sample_Debug(void);
void ADS869x_SetRefMode(void);

#endif
