#include "uart.h"
#include "stdio.h"
#include <string.h>
#include "dataStore.h"

static uint8_t uart1_buffer[64];
static uint16_t uart1_cnt=0;
static PC_CMD_Structure g_pc_cmd;
static char cmd_SSID[10];
static uint8_t cmd_node_num;


int fputc(int ch,FILE *f)
{
    USART_SendData(USART1,(uint8_t)ch);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送数据完毕);
    return ch;
}


void Uart_Log_Configuration(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//使能USART1
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	    //GPIOA时钟

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    USART_DeInit(USART1);  //复位串口1

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

    

    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//开启USART1的接收中断
    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;  //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		 //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
    
    USART_Cmd(USART1,ENABLE);
    
    USART_ClearFlag(USART1,USART_FLAG_TC);  //清除发送完成标志位 
		
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
const char ARM_AT_SET_NUM[] = {"AT+SETNUM"};
const char ARM_AT_CONFIG[] = {"AT+CONFIG="};

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
    char *str_start=0,*str_end=0;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET) //判断是否产生接收中断
    {
        uart_data=USART_ReceiveData(USART1);
        if(uart1_cnt<sizeof(uart1_buffer))
            uart1_buffer[uart1_cnt++] = uart_data ;
        
        if(uart_data == '\n')
        {
            uart1_cnt = 0;

            str_start = strstr((const char *)uart1_buffer,ARM_AT_CONFIG);
            if(str_start)
            {
                printf("Get AT\n");
                str_start+=strlen(ARM_AT_CONFIG);
                if( !(str_end = strchr((const char  *)str_start,',')))
                    return ;
                //printf("%s\n",str_start);
                memcpy(cmd_SSID,str_start,str_end-str_start);
                cmd_SSID[str_end-str_start]=0;
                //printf("%s\n",cmd_SSID);
                str_start=str_end+1;
                
                cmd_node_num=(*str_start);
                if(!(cmd_node_num>='1'&&cmd_node_num<='4'))
                    //printf("node is %d\n",cmd_node_num-'0');
                    return ;
                Set_Node_NUM(cmd_node_num-'0');
                SetWifiSSID((void *)cmd_SSID,sizeof(cmd_SSID));
                SetGlobalData();              

            }
            /*
            for(i=0;i<sizeof(g_pc_cmd.at_cmd_flag);i++)
            {
                if(g_pc_cmd.at_cmd_flag[i] == 0)  //empty fifo
                {
                    memcpy(g_pc_cmd.at_cmd[i],uart1_buffer,uart1_cnt);
                    uart1_cnt = 0;
                    g_pc_cmd.at_cmd_flag[i] =1;
                    Handler_PC_Command();
                    break;
                }          
            }
            */

        }
    }

}
