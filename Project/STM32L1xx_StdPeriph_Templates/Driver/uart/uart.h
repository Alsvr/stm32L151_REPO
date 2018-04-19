#ifndef __UART_H
#define __UART_H	

#include "stm32l1xx.h" 
typedef struct 
{
    uint8_t at_cmd[4][64];
    uint8_t at_cmd_flag[4];
}PC_CMD_Structure;
 


void Uart_Log_Configuration(void);  
void Handler_PC_Command(void);

#endif 















