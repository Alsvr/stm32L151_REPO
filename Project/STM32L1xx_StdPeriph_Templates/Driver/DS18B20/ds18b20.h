#ifndef __DS18B20_H
#define __DS18B20_H 

#include "stm32l1xx.h" 
#include "stdio.h"
//IO��������
//#define DS18B20_IO_IN()    {GPIOA->CRL&=0XFFFFFFF0;GPIOA->CRL|=8<<0;}
//#define DS18B20_IO_OUT()   {GPIOA->CRL&=0XFFFFFFF0;GPIOA->CRL|=3<<0;}
////IO��������											   
//#define	DS18B20_DQ_OUT PAout(0) //���ݶ˿�	PA0 
//#define	DS18B20_DQ_IN  PAin(0)  //���ݶ˿�	PA0 
   	
void bsp_InitDS18B20(void);//��ʼ��DS18B20
void bsp_DeInitDS18B20(void);

int16_t DS18B20_ReadTempReg(void);


#endif















