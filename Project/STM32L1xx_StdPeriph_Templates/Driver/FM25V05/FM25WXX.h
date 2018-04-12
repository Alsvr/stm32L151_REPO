#ifndef __FM25WXX_H
#define __FM25WXX_H			    
#include "stm32l1xx.h" 

uint16_t FM25VXX_Init(void);
void FM25VXX_PowerDown(void);  
void FM25VXX_WAKEUP(void);
void FM25VXX_Read(uint8_t* pBuffer,uint16_t ReadAddr,uint16_t NumByteToRead);
void FM25VXX_Write(uint8_t* pBuffer,uint16_t WriteAddr,uint16_t NumByteToWrite);
void test_FM25V05(void);
#endif
















