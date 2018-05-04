#include "uart.h"
#include "stdio.h"
#include <string.h>
static uint8_t uart1_buffer[512];
static uint16_t uart1_cnt=0;
static PC_CMD_Structure g_pc_cmd;




int fputc(int ch,FILE *f)
{
    USART_SendData(USART1,(uint8_t)ch);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//�ȴ������������);
    return ch;
}


void Uart_Log_Configuration(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//ʹ��USART1
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	    //GPIOAʱ��

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    USART_DeInit(USART1);  //��λ����1

    //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	
    GPIO_Init(GPIOA, &GPIO_InitStructure); 

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1,&USART_InitStructure);

    

    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//����USART1�Ľ����ж�
    //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//����1�ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;  //��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		 //�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 //IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����
    
    USART_Cmd(USART1,ENABLE);
    
    USART_ClearFlag(USART1,USART_FLAG_TC);  //���������ɱ�־λ 
		
}

//const char AT_CONFIG_START[] = {"+++"};  
//const char AT_ENTM[] = {"AT+ENTM\r"};  
//const char AT_E[] = {"AT+E\r"};
//const char AT_a[] = {"a"};
//const char PC_AT_OK[] = {"+OK"};
//const char PC_AT_WANN[] = {"AT+WANN\r"};
//const char AT_WSLK[] = {"AT+WSLK\r"};
//const char AT_WSCAN[] = {"AT+WSCAN\r"};
//const char AT_UART[] = {"AT+UART\r"};
//const char AT_SOCKA[] = {"AT+SOCKA\r"};
//const char AT_MSLP[] = {"AT+MSLP\r"};
const char ARM_AT_SET_NUM[] = {"AT+SETNUM=\r"};

void Handler_PC_Command(void)
{
    uint8_t i=0,*str_start=0;
    for(i=0;i<sizeof(g_pc_cmd.at_cmd_flag);i++)
    {
        if(g_pc_cmd.at_cmd_flag[i] == 1)
        {
            if(str_start = strstr((const char *)(g_pc_cmd.at_cmd[i]),ARM_AT_SET_NUM))
            {
                printf("%d\n",str_start-g_pc_cmd.at_cmd[i]);
                printf("AT+SETNUM= AT OK\n");
                g_pc_cmd.at_cmd_flag[i] =0;

            }
            break;
        }          
    }   


}


void USART1_IRQHandler(void)
{
    uint8_t uart_data=0, i=0;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET) //�ж��Ƿ���������ж�
    {
        uart_data=USART_ReceiveData(USART1);
        uart1_buffer[uart1_cnt++] = uart_data ;
        if(uart_data == '\n')
        {
            for(i=0;i<sizeof(g_pc_cmd.at_cmd_flag);i++)
            {
                if(g_pc_cmd.at_cmd_flag[i] == 0)
                {
                    memcpy(g_pc_cmd.at_cmd[i],uart1_buffer,uart1_cnt);
                    uart1_cnt = 0;
                    g_pc_cmd.at_cmd_flag[i] =1;
                    //printf("i=%d,%s",i,(char *)g_pc_cmd.at_cmd[i]);
                    Handler_PC_Command();
                    break;
                }          
            }
            if(i == sizeof(g_pc_cmd.at_cmd_flag))
                uart1_cnt = 0;
        }
    }

}
