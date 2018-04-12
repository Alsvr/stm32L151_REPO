#include "stdio.h"
#include "wireless.h"
#include <string.h>
//433M AUX --->PA1
//433M_MO  --->PB8
//433M_M1  --->PB9
//433M_POWER  --->PB10
//433M_TX  --->PA2
//433M_RX  --->PA3
#define M0_GPIO GPIO_Pin_8
#define M1_GPIO GPIO_Pin_9
#define POWER_GPIO GPIO_Pin_10
uint8_t wireless_rx_buff[64]; 
static int wireless_rx_cnt=0;
static uint8_t bu[2]={0,0};
static uint8_t uart2_buffer[64];
static uint8_t uart2_cnt = 0;

void wire_less_uart_init(uint32_t buand)
{
	
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART1
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	//GPIOA时钟

	USART_DeInit(USART2);  //复位串口2

	//USART2_TX   PA.2 PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure); 

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART1);


	USART_InitStructure.USART_BaudRate = buand;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2,&USART_InitStructure);

	USART_Cmd(USART2,ENABLE);

	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//开启USART1的接收中断
	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

	USART_ClearFlag(USART2,USART_FLAG_TC); //清除发送完成标志位 
}	


void Wireless_wakeup_mode(void)
{
  GPIO_ResetBits(GPIOB,M0_GPIO);
  GPIO_SetBits(GPIOB,M1_GPIO);
  delay_ms(20);
}

void Wireless_Config_mode(void)
{
  GPIO_SetBits(GPIOB,M0_GPIO);
  GPIO_SetBits(GPIOB,M1_GPIO);
  delay_ms(20);
}

void Wireless_power_down(void)
{
  GPIO_SetBits(GPIOB,POWER_GPIO);
}

void Wireless_power_on(void)
{
 
  GPIO_ResetBits(GPIOB,POWER_GPIO);
}

uint8_t uart_data=0;
static uint8_t cc3200_start_flag=0;
const char AT_CONFIG_START[] = {"+++"};  
const char AT_ENTM[] = {"AT+ENTM\r"};  
const char AT_E[] = {"AT+E\r"};
const char AT_a[] = {"a"};
const char AT_OK[] = {"+OK"};
const char AT_WANN[] = {"AT+WANN\r"};
const char AT_WSLK[] = {"AT+WSLK\r"};
const char AT_WSCAN[] = {"AT+WSCAN\r"};
const char AT_UART[] = {"AT+UART\r"};
const char AT_SOCKA[] = {"AT+SOCKA\r"};
const char AT_MSLP[] = {"AT+MSLP\r"};

static uint16_t At_cmd_state = 0;
#define AT_CMD_WAIT_A    1 
#define AT_CMD_WAIT_OK   2 
#define AT_CMD_WAIT_OK_PASS   3 
#define AT_CMD_WAIT_AT_E_BACK 4

#define AT_CMD_WAIT_AT_TEMP   0xff
void Send_At_Cmd(const char * p,uint16_t cmd_state )
{
    uint8_t i = 0;
    uint8_t num = strlen(p);
    //USART_SendData(USART1,'-');
    //while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送数据完毕);
    
    for(i=0;i<num;i++)
    {
        USART_SendData(USART2,(uint8_t)*(p++)); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);//等待发送数据完毕);

        //USART_SendData(USART1,(uint8_t)*(p++));
        //while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送数据完毕);		
    }
    At_cmd_state = cmd_state;
    uart2_cnt = 0;
}

void WireLess_Send_data(const char * p,uint32_t len )
{
    uint32_t i = 0;
    for(i=0;i<len;i++)
    {
        USART_SendData(USART2,(uint8_t)*(p++)); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);//等待发送数据完毕);

        //USART_SendData(USART1,(uint8_t)*(p++));
        //while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送数据完毕);		
    }
   
}


uint8_t WireLess_check_wifi_ok()
{
    Send_At_Cmd(AT_WSLK,AT_CMD_WAIT_AT_TEMP);
    if(   
}
void Init_CC3200(uint8_t HEAD,
                uint16_t ADD,
                uint32_t baund,
                uint8_t channel)
{
    uint8_t i=0;
    uint8_t * at_p=0;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    /* Power config*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB,GPIO_Pin_10);

    wire_less_uart_init(115200);
    delay_ms(800);
    Send_At_Cmd(AT_CONFIG_START,AT_CMD_WAIT_A);
    while(At_cmd_state != AT_CMD_WAIT_OK_PASS)
    {
       delay_us(10);
    }
    delay_ms(10);
    Send_At_Cmd(AT_E,AT_CMD_WAIT_AT_TEMP);

    
    delay_ms(2000);
    Send_At_Cmd(AT_WSLK,AT_CMD_WAIT_AT_TEMP);
    delay_ms(1000);
    Send_At_Cmd(AT_WANN,AT_CMD_WAIT_AT_TEMP);
    delay_ms(1000);
    Send_At_Cmd(AT_ENTM,AT_CMD_WAIT_AT_TEMP);
    delay_ms(10);
    WireLess_Send_data("AAAAAAAAS3333333333333333333333333333DSFDS",5000);
    delay_ms(1);
    WireLess_Send_data("AAAAAAAAS3333333333333333333333333333DSFDS",5000);
    delay_ms(1);
    WireLess_Send_data("AAAAAAAAS3333333333333333333333333333DSFDS",5000);
    delay_ms(100);
    
    Send_At_Cmd(AT_CONFIG_START,AT_CMD_WAIT_A);
    while(At_cmd_state != AT_CMD_WAIT_OK_PASS)
    {
       delay_us(10);
    }
    delay_ms(100);
    Send_At_Cmd(AT_MSLP,AT_CMD_WAIT_AT_TEMP);

    delay_ms(1000);
    WireLess_Send_data("00000",5);
    delay_ms(10);delay_ms(4000);delay_ms(4000);
    WireLess_Send_data("AAAAAAAAS3333333333333333333333333333DSFDS",5000);
}

void init_wireless(uint8_t HEAD,
                        uint16_t ADD,
                        uint32_t baund,
                        uint8_t channel)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    uint8_t i=0;

    WireLess_DATA wireless_data;
    uint8_t * wireless_data_p=NULL;
    /* Enable GPIOB clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    /* M1M0 config*/ 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    Wireless_Config_mode();

    /* AUX config*/
    /* Enable GPIOA clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    /* Configure PA1 pin as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Power config*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB,GPIO_Pin_10);


    /* Configure EXTI1 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    wire_less_uart_init(9600);
    wireless_data.HEAD=0xC0;
    wireless_data.ADDH=(ADD>>8)&0xFF;
    wireless_data.ADDL=(ADD)&0xFF;
    wireless_data.SPED=0x07;
    switch (baund)
    {
        case 9600: wireless_data.SPED|=(0x03<<3);
        break;
        case 115200: wireless_data.SPED|=(0x07<<3);
        break;
        default :break;
    } 
    wireless_data.CHAN=channel;
    wireless_data.OPTION=0xFC;
    wireless_data_p =(uint8_t *)&wireless_data;
    for(i=0;i<6;i++)
    {
        uart_data=(uint8_t)*(wireless_data_p++);
        USART_SendData(USART2,uart_data); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);//等待发送数据完毕);		

    }
    delay_ms(100);
    Wireless_wakeup_mode();

}






void USART2_IRQHandler(void)
{
   uint8_t da=0;
   if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET) //判断是否产生接收中断
	{
        
        da = USART_ReceiveData(USART2);
        USART_SendData(USART1,(uint8_t)da);
        while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送数据完毕);

        if(At_cmd_state == AT_CMD_WAIT_A)
        {
            if(da=='a')
            {
                USART_SendData(USART2,'a'); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
                while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);//等待发送数据完毕);	
                At_cmd_state = AT_CMD_WAIT_OK;
                uart2_cnt=0;
                memset(uart2_buffer, 0, sizeof(uart2_buffer));
            }   
        }
        else if(At_cmd_state == AT_CMD_WAIT_OK)
        {
            uart2_buffer[uart2_cnt++] = da;
            if(strcmp(AT_OK,(const char *)uart2_buffer) == 0)
            {
                At_cmd_state = AT_CMD_WAIT_OK_PASS;
            }
        }
        else if(At_cmd_state == AT_CMD_WAIT_AT_E_BACK)
        {
            uart2_buffer[uart2_cnt++] = da;
            if(strcmp(AT_OK,(const char *)uart2_buffer) == 0)
            {
                At_cmd_state = AT_CMD_WAIT_OK_PASS;
            }
        }
        else if(AT_CMD_WAIT_AT_TEMP ==At_cmd_state)
        {
            uart2_buffer[uart2_cnt++] = da;
            
            
        }
       
//wireless_rx_buff[wireless_rx_cnt++]=USART_ReceiveData(USART2);
	}


}