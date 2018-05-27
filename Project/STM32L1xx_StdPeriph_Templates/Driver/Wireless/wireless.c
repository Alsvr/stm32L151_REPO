#include "stdio.h"
#include "wireless.h"
#include "GPIO.h"
#include <string.h>
#include "dataStore.h"
#include "FM25WXX.h"
#include "dataStore.h"
#include "ADS869x.h"
//433M AUX --->PA1
//433M_MO  --->PB8
//433M_M1  --->PB9
//433M_POWER  --->PB10
//433M_TX  --->PA2
//433M_RX  --->PA3
#define M0_GPIO GPIO_Pin_8
#define M1_GPIO GPIO_Pin_9
#define POWER_GPIO GPIO_Pin_10
uint8_t wireless_rx_buff[512]; 
#define wireless_rx_buff_size  512
uint8_t wireless_process_buff[512];  
static int wireless_rx_cnt=0;
static uint8_t bu[2]={0,0};
static uint8_t uart2_buffer[64];
static uint8_t uart2_cnt = 0;

uint8_t  WiFi_SetWifiConfig(GlobalData_Para *globaldata_p);
void wire_less_uart_init(uint32_t buand)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART1
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	//GPIOA时钟
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
    
	USART_DeInit(USART2);  //复位串口2

	//USART2_TX   PA.2 PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
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
	USART_Init(USART2,&USART_InitStructure);

	

	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//开启USART1的接收中断
	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	
    USART_Cmd(USART2,ENABLE);
    
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
    GPIO_InitTypeDef GPIO_InitStructure;
    //GPIO_SetBits(GPIOB,POWER_GPIO);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOA, ENABLE);
    /* Power config*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

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
const char AT_WANN[] = {"AT+WANN=DHCP\r"};
const char AT_WSLK[] = {"AT+WSLK\r"};
const char AT_WSCAN[] = {"AT+WSCAN\r"};
const char AT_UART[] = {"AT+UART\r"};
const char AT_SOCKA[] = {"AT+SOCKA\r"};
const char AT_MSLP[] = {"AT+MSLP\r"};
//const char AT_WSTA[] = {"AT+WSTA=Widora-859E,12345678\r"};
const char AT_WSTA_HEAD[] = {"AT+WSTA="};
const char AT_WSTA_PASSWORD[] = {",12345678\r"};
const char AT_WKMOD[] = {"AT+WKMOD=TRANS\r"};
const char AT_WMODE[] = {"AT+WMODE=STA\r"};
const char AT_SLPTYPE[] = {"AT+SLPTYPE=4,10\r"};
const char AT_SOCKA_S[] = {"AT+SOCKA=UDPC,192.168.8.1,8989\r"};
const char AT_BACK_DISCONNECTION[] = {"+OK=DISCONNECTED"};

static uint16_t At_cmd_state = 0;
#define AT_CMD_WAIT_A                0X0001 
#define AT_CMD_WAIT_OK               0X0002  
#define AT_CMD_WAIT_OK_PASS          0X0004 
#define AT_CMD_WAIT_OK_PASS_FINISH   0X0008  
#define AT_CMD_WAIT_AT_E_BACK        0X0010
#define AT_CMD_WAIT_AT_BACK          0X0020
#define AT_CMD_WAIT_AT_BACK_PASS     0X0040
#define AT_CMD_MASK                  0X00FF

#define AT_DATA_WAIT_DATA            0x0100
#define AT_DATA_MASK                 0XFF00




void Reset_CC3200(void)
{
    uint8_t i=0 ,re =0;
    uint8_t * at_p=0;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    /* Power config*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB,GPIO_Pin_8);

    delay_ms(50);
    GPIO_SetBits(GPIOB,GPIO_Pin_8);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);  

}

                
void Send_At_Cmd(const char * p,uint8_t num)
{
    uint8_t i = 0;
    memset(uart2_buffer,0,sizeof(uart2_buffer));
    uart2_cnt = 0;
    for(i=0;i<num;i++)
    {
        USART_SendData(USART2,(uint8_t)*(p++)); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);//等待发送数据完毕);	
    }
}
/*
void WireLess_Send_data(const char * p,uint32_t len )
{
    uint32_t i = 0;
    for(i=0;i<len;i++)
    {
        USART_SendData(USART2,(uint8_t)*(p++)); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);//等待发送数据完毕);
    }
   
}
*/

uint8_t WireLess_check_wifi_ok()
{
    Send_At_Cmd("0000",4);
    delay_ms(10);
}


uint8_t WiFi_try_CMD_mode(void)
{
    uint8_t time_out_cnt = 0;
    delay_ms(20);  //避开打包间隔
    __disable_irq();
    At_cmd_state = AT_CMD_WAIT_A;
    __enable_irq(); 
    Send_At_Cmd(AT_CONFIG_START,strlen(AT_CONFIG_START));
    do
    {
        delay_ms(10);
        if(At_cmd_state == AT_CMD_WAIT_OK)
        {
            break;
        }
        else
        {
            time_out_cnt++;

        }
    }while(time_out_cnt < 200);
    // if the wait a time more than 1S
    if(time_out_cnt >= 200)
    {

        printf("wait 'a' fail\n");   //等待 a 返回失败
        return 0;
    }
    printf("wait 'a' success, then send a\n");
    __disable_irq();
    At_cmd_state = AT_CMD_WAIT_OK;
    __enable_irq(); 
    Send_At_Cmd(AT_a,strlen(AT_a));    

    memset(uart2_buffer, 0, sizeof(uart2_buffer));
    
    time_out_cnt = 0;
    do
    {
        delay_ms(100);
        if(At_cmd_state == AT_CMD_WAIT_OK_PASS)
        {
            At_cmd_state = AT_CMD_WAIT_OK_PASS_FINISH;
            printf("AT_CMD_WAIT_OK_PASS_FINISH\n");
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < 20);
    if(time_out_cnt >= 20)
    {
        printf("wait +OK fail\n");
        return 0;
    }
    return 1;

}


uint8_t WiFi_Enter_CMD_mode(void)
{
    uint16_t i=0,re;
    do
    {
        re=WiFi_try_CMD_mode();   //尝试进入cmd 模式 return 0 means fail
        i++;
    
    }while((i<8)&&(re==0));
    if(i>=8)
    {
        printf("Enter wifi cmd mode fail!\n");
        return 0;
    }
    else
    {
        printf("Enter wifi cmd mode success!\n");    
        return 1;
    }


}
#define WIFI_WAIT_AT_BACK_DELAY 1000 //100ms
#define WIFI_WAIT_LINK_OK_DELAY 500 //times
uint8_t WiFi_GetWifiStatus(void)
{
    uint16_t time_out_cnt = 0;

    __disable_irq();
        At_cmd_state = AT_CMD_WAIT_AT_BACK;
    Send_At_Cmd(AT_WSLK,strlen(AT_WSLK));    

    __enable_irq(); 

    time_out_cnt = 0;
    do
    {
        delay_ms(500);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("WiFi_GetWifiStatus fail %d\n",time_out_cnt);
        return 0;
    }
    printf("AT_WSLK get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);

    if(strstr((const char *)(&uart2_buffer[2]),AT_BACK_DISCONNECTION))
    {   
        return 0;
    }
    else
    {
       // printf("WiFi_GetWifiStatus pass\n");
       
       return 1;
    }
}


uint8_t WiFi_WaitLinkOk(void)
{
    
    uint16_t time_out_cnt = 0;
    do
    {
        if(WiFi_GetWifiStatus())
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_LINK_OK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_LINK_OK_DELAY)
    {
        printf("WiFiWaitLinkOk fail\n");
        return 0;
    }
    return 1;

}
uint8_t WiFi_Exit_CMD_mode(void)
{
    uint16_t time_out_cnt = 0;
    __disable_irq();
    At_cmd_state = AT_CMD_WAIT_AT_BACK;  
    __enable_irq(); 
    Send_At_Cmd(AT_ENTM,strlen(AT_ENTM));

    time_out_cnt = 0;
    do
    {
        delay_ms(1);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("WiFi_Exit_CMD_mode wait back fail ,wait %d ms\n",time_out_cnt);
        return 0;
    }
    printf("AT_ENTM get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);
    At_cmd_state=AT_DATA_WAIT_DATA;
    wireless_rx_cnt = 0;
    if(strstr((const char *)(&uart2_buffer[2]),AT_OK))
    {
        printf("exit cmd mode\n");
        return 1;  
    }
    else
    {
        printf("exit cmd mode error\n");
        return 0;
    }

}


uint8_t WiFi_EnterPowerDownMode(void)
{
    
    uint16_t time_out_cnt = 0;
    
    WiFi_Enter_CMD_mode();
    __disable_irq();
    At_cmd_state = AT_CMD_WAIT_AT_BACK;
    Send_At_Cmd(AT_MSLP,strlen(AT_MSLP));    
    __enable_irq(); 

    time_out_cnt = 0;
    do
    {
        delay_ms(1);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("WiFi_EnterLowPowerMode 4 wait error %d\n",time_out_cnt);
        return 0;
    }
    printf("AT_MSLP get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);

    if(strstr((const char *)(&uart2_buffer[2]),AT_OK))
    {
        printf("WiFi_EnterLowPowerMode 4 success\n");
        return 1;  
    }
    else
    {
        printf("WiFi_EnterLowPowerMode 4 success fail\n");
        return 0; 
    }
    

}

uint8_t WiFi_EnterLowPowerMode(void)
{
    
    uint16_t time_out_cnt = 0;
    
    WiFi_Enter_CMD_mode();
    
    __disable_irq();
    At_cmd_state = AT_CMD_WAIT_AT_BACK;
    Send_At_Cmd(AT_MSLP,strlen(AT_MSLP));    
    __enable_irq(); 

    time_out_cnt = 0;
    do
    {
        delay_ms(1);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("WiFi_EnterLowPowerMode wait error %d\n",time_out_cnt);
        return 0;
    }
    printf("AT_MSLP get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);

    if(strstr((const char *)(&uart2_buffer[2]),AT_OK))
    {
        printf("WiFi_EnterLowPowerMode success\n");
        return 1;  
    }
    else
    {
        printf("WiFi_EnterLowPowerMode success fail\n");
        return 0;
    }

}

uint8_t WiFi_EnterNoPowerMode(void)
{
    
    uint16_t time_out_cnt = 0;
    
    WiFi_Enter_CMD_mode();
    
    __disable_irq();
    At_cmd_state = AT_CMD_WAIT_AT_BACK;
    Send_At_Cmd(AT_MSLP,strlen(AT_MSLP));    
    __enable_irq(); 

    time_out_cnt = 0;
    do
    {
        delay_ms(1);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("WiFi_EnterLowPowerMode wait error %d\n",time_out_cnt);
        return 0;
    }
    printf("AT_MSLP get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);

    if(strstr((const char *)(&uart2_buffer[2]),AT_OK))
    {
        printf("WiFi_EnterLowPowerMode success\n");
        return 1;  
    }
    else
    {
        printf("WiFi_EnterLowPowerMode success fail\n");
        return 0;
    }

}
void Init_CC3200(uint8_t first,
                uint32_t baund)
{
    uint8_t i=0 ,re =0;
    uint8_t * at_p=0;
    GPIO_InitTypeDef GPIO_InitStructure;
    wire_less_uart_init(baund);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    /* Power config*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    Wireless_power_on();
    //enter cmd 
    if(first)
    {
        WiFi_Enter_CMD_mode();
        WiFi_SetWifiConfig(0);
        WiFi_Exit_CMD_mode();
        Reset_CC3200();
        delay_ms(1000);
    }
    else
    {
        Reset_CC3200();
        delay_ms(1000);
        
    }
    //delay_ms(1000);
    //delay_ms(1000);
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





uint8_t  WiFi_SetSOCKA(char *str)
{
    uint16_t time_out_cnt = 0;

    __disable_irq();
    
    At_cmd_state = AT_CMD_WAIT_AT_BACK;
    Send_At_Cmd(AT_SOCKA_S,strlen(AT_SOCKA_S));    
    __enable_irq(); 

    time_out_cnt = 0;
    do
    {
        delay_ms(1);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("AT_SOCKA_S fail %d\n",time_out_cnt);
        return 0;
    }
    printf("AT_SOCKA_S get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);

    if(strstr((const char *)(&uart2_buffer[2]),AT_OK))
    {   
        return 1;
    }
    else
    {

        return 0;
    }    
}


uint8_t  WiFi_SetWKMOD(char *str)
{
    uint16_t time_out_cnt = 0;

    __disable_irq();
        At_cmd_state = AT_CMD_WAIT_AT_BACK;
    Send_At_Cmd(AT_WKMOD,strlen(AT_WKMOD));    

    __enable_irq(); 

    time_out_cnt = 0;
    do
    {
        delay_ms(1);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("WiFi_GetAT_WKMOD_OK fail %d\n",time_out_cnt);
        return 0;
    }
    printf("AT_WKMOD get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);

    if(strstr((const char *)(&uart2_buffer[2]),AT_OK))
    {   
        return 1;
    }
    else
    {

        return 0;
    }    
}

uint8_t  WiFi_SetWSTA(char *str)
{
    uint16_t time_out_cnt = 0;
    char wsta_str[64];
    strcpy (wsta_str,AT_WSTA_HEAD);
    strcat (wsta_str,GetWifiSSID());
    strcat (wsta_str,AT_WSTA_PASSWORD);
    printf("wsta string is %s\n",wsta_str);
    __disable_irq();
    At_cmd_state = AT_CMD_WAIT_AT_BACK;
    Send_At_Cmd(wsta_str,strlen(wsta_str));    
    __enable_irq(); 

    time_out_cnt = 0;
    do
    {
        delay_ms(1);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("WiFi_GetAT_WSTA_OK fail %d\n",time_out_cnt);
        return 0;
    }
    printf("AT_WSTA get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);

    if(strstr((const char *)(&uart2_buffer[2]),AT_OK))
    {   
        return 1;
    }
    else
    {

        return 0;
    }    
}

uint8_t  WiFi_SetWMODE(char *str)
{
    uint16_t time_out_cnt = 0;

    __disable_irq();
        At_cmd_state = AT_CMD_WAIT_AT_BACK;
    Send_At_Cmd(AT_WMODE,strlen(AT_WMODE));    

    __enable_irq(); 

    time_out_cnt = 0;
    do
    {
        delay_ms(1);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("WiFi_GetAT_WSTA_OK fail %d\n",time_out_cnt);
        return 0;
    }
    printf("AT_WSTA get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);

    if(strstr((const char *)(&uart2_buffer[2]),AT_OK))
    {   
        return 1;
    }
    else
    {

        return 0;
    }    
}

uint8_t  WiFi_SetSLPTYPE(char *str)
{
    uint16_t time_out_cnt = 0;

    __disable_irq();
    At_cmd_state = AT_CMD_WAIT_AT_BACK;
    Send_At_Cmd(AT_SLPTYPE,strlen(AT_SLPTYPE));    
    __enable_irq(); 

    time_out_cnt = 0;
    do
    {
        delay_ms(1);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("WiFi_GetAT_WSTA_OK fail %d\n",time_out_cnt);
        return 0;
    }
    printf("AT_WSTA get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);

    if(strstr((const char *)(&uart2_buffer[2]),AT_OK))
    {   
        return 1;
    }
    else
    {

        return 0;
    }    
}


uint8_t  WiFi_SetWANN(char *str)
{
    uint16_t time_out_cnt = 0;

    __disable_irq();
        At_cmd_state = AT_CMD_WAIT_AT_BACK;
    Send_At_Cmd(AT_WANN,strlen(AT_WANN));    

    __enable_irq(); 

    time_out_cnt = 0;
    do
    {
        delay_ms(1);
        if(At_cmd_state == AT_CMD_WAIT_AT_BACK_PASS)
        {
            break;
        }
        else
        {
            time_out_cnt++;
        }
    }while(time_out_cnt < WIFI_WAIT_AT_BACK_DELAY);
    if(time_out_cnt >= WIFI_WAIT_AT_BACK_DELAY)
    {
        printf("WiFi_GetAT_WSTA_OK fail %d\n",time_out_cnt);
        return 0;
    }
    printf("AT_WSTA get %s wait %d ms\n",&uart2_buffer[2],time_out_cnt);

    if(strstr((const char *)(&uart2_buffer[2]),AT_OK))
    {   
        return 1;
    }
    else
    {

        return 0;
    }    
}
uint8_t  WiFi_SetWifiConfig(GlobalData_Para *globaldata_p)
{
    uint16_t time_out_cnt = 0;

    if(WiFi_SetWSTA(0))
        printf("Set WiFi_SetWSTA OK\n");
    else
        printf("Set WiFi_SetWSTA FAIL\n");
    if(WiFi_SetWKMOD(0))
        printf("Set WiFi_SetWKMOD OK\n");
    else
        printf("Set WiFi_SetWKMOD FAIL\n");
    if(WiFi_SetSOCKA(0))
        printf("Set WiFi_SetSOCKA OK\n");
    else
        printf("Set WiFi_SetSOCKA FAIL\n");
    if(WiFi_SetWMODE(0))
        printf("Set WiFi_SetWMODE OK\n");
    else
        printf("Set WiFi_SetWMODE FAIL\n");
    if(WiFi_SetSLPTYPE(0))
        printf("Set WiFi_SetSLPTYPE OK\n");
    else
        printf("Set WiFi_SetSLPTYPE FAIL\n");

    if(WiFi_SetWANN(0))
        printf("Set WiFi_SetWANN OK\n");
    else
        printf("Set WiFi_SetWANN FAIL\n");
}

static uint16_t buff_ptr=0,valid_data_length=0;
uint8_t  WiFi_GetUDPData(void)
{
    uint16_t time_out_cnt = 0;
    printf("Get wifi UDP cnt is %d\n",wireless_rx_cnt);
    if(wireless_rx_cnt>0)
    {
        __disable_irq();
        //Led_Open();    
        memcpy(wireless_process_buff + buff_ptr + valid_data_length, wireless_rx_buff, wireless_rx_cnt);
        wireless_rx_cnt = 0;
        //Led_Close();
        __enable_irq();
        //printf("Get wifi UDP cnt is %d\n",wireless_rx_cnt);
    }
    return 1;
}

/*
uint8_t WireLess_Send_data(Node_Instru_Packet *node_instru_packet,uint32_t len )
{
    uint32_t i = 0;
    uint8_t *p=0;
    p=(uint8_t *)node_instru_packet;
    for(i=0;i<len;i++)
    {
        USART_SendData(USART2,(uint8_t)*(p++)); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);//等待发送数据完毕);
    }
    __disable_irq();
    At_cmd_state=AT_DATA_WAIT_DATA;
    wireless_rx_cnt=0;
    __enable_irq();
    for(i=0;i<3000;i++)
    {
        delay_ms(1);
        if(wireless_rx_cnt>=sizeof(Node_Instru_Packet))
        {
            memcpy(node_instru_packet, wireless_rx_buff, wireless_rx_cnt);
            return 1;
        }
    }
    return 0;
   
}
*/

uint8_t WiFi_Send_Report(Node_Instru_Packet *node_instru_packet,
                        uint16_t temp,uint16_t adc1,uint16_t adc_2,
                        uint16_t power,uint8_t addr,
                        uint16_t udp_index)
{
    uint32_t i = 0;
    uint8_t *p=0;
    node_instru_packet->header1= NODE_INSTRU_HEAD1;
    node_instru_packet->header2= NODE_INSTRU_HEAD2;
    node_instru_packet->instru = NODE_TO_SERVER_INST_HEART;
    node_instru_packet->node_addr =addr;
    node_instru_packet->data[0] = temp&0xff;
    node_instru_packet->data[1] =(temp>>8)&0xff;
    node_instru_packet->data[2] =adc1&0xff;
    node_instru_packet->data[3] =(adc1>>8)&0xff;

    node_instru_packet->data[4] =adc_2&0xff;
    node_instru_packet->data[5] =(adc_2>>8)&0xff;

    node_instru_packet->data[6] =power&0xff;
    node_instru_packet->data[7] =(power>>8)&0xff;

    node_instru_packet->data[8] =udp_index&0xff;
    node_instru_packet->data[9] =(udp_index>>8)&0xff;
    
    p=(uint8_t *)node_instru_packet;
    __disable_irq();
    At_cmd_state=AT_DATA_WAIT_DATA;
    wireless_rx_cnt=0;
    for(i=0;i<sizeof(Node_Instru_Packet);i++)
    {
        USART_SendData(USART2,(uint8_t)*(p++)); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);//等待发送数据完毕);
    }
 
    __enable_irq();
    printf("Send report to server\n");
    memset((void *)node_instru_packet,0,sizeof(Node_Instru_Packet));
    for(i=0;i<6000;i++)   //wait 3S
    {
        delay_ms(1);
        if(wireless_rx_cnt>=sizeof(Node_Instru_Packet))
        {
            memcpy(node_instru_packet, wireless_rx_buff, wireless_rx_cnt);
            if((node_instru_packet->header1 == NODE_INSTRU_HEAD1)&&
                (node_instru_packet->header1 == NODE_INSTRU_HEAD1))
            {
                if(node_instru_packet->instru == SERVER_TO_NODE_CMD)
                {
                    printf("Get the Server cmd\n");
                    return 1;
                }
            }
        }
    }
    return 0;
   
}

uint8_t WireLess_Send_ADC_data(void)
{
    uint16_t i = 0,snd_pkt,adc_packet_len;
    uint8_t *p=0;
    uint16_t *P_int16=0;
    Node_Instru_Packet node_instru_packet;
    adc_packet_len= Get_ADC_LEN();
    p=(uint8_t *)&node_instru_packet;
    printf("start send data to server!\n");
    node_instru_packet.instru= NODE_TO_SERVER_INST_ADC_DATA;
    node_instru_packet.header1 = NODE_INSTRU_HEAD1;
    node_instru_packet.header2 = NODE_INSTRU_HEAD2;
    node_instru_packet.node_addr =Get_Node_NUM();
    node_instru_packet.commend1 = 0x00;
    node_instru_packet.commend2 = 0x00;
    node_instru_packet.commend3 = 0x00;
    node_instru_packet.tail1=NODE_INSTRU_TAIL1;
    node_instru_packet.tail2=NODE_INSTRU_TAIL2;
    P_int16 =(uint16_t *)node_instru_packet.data;
    FM25VXX_Init();
    for(snd_pkt=0;snd_pkt<adc_packet_len;snd_pkt++){
       
        FM25VXX_Read(node_instru_packet.data,snd_pkt*ADC_PACKET_SIZE,ADC_PACKET_SIZE);
        p=(uint8_t *)&node_instru_packet;
        P_int16 =(uint16_t *)node_instru_packet.data;
        node_instru_packet.commend1=(snd_pkt>>8)&0xff;  
        node_instru_packet.commend2=(snd_pkt)&0xff; 

        if(snd_pkt==(adc_packet_len-1)){
                node_instru_packet.commend3 = 0x01;
            }

        //printf("send p is%d len is %d !\n",snd_pkt,adc_packet_len);
        for(i=0;i<sizeof(node_instru_packet);i++)
        {
            USART_SendData(USART2,(uint8_t)*(p++)); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
            while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);//等待发送数据完毕);
        }
#if 0
        for(i=0;i<(ADC_PACKET_SIZE>>1);i++)
        {
            printf("%d\n",*(P_int16++));
        }
#else
         delay_ms(10);   //  >5ms  make sure packet div
#endif


        //第一次传输  传输4S 若发送失败则放弃

         
    }
    FM25VXX_DisInit();
    printf("send data to server finish!\n");
    return 0;
   
}
////
void USART2_IRQHandler(void)
{
   uint8_t da=0;
   if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET) //判断是否产生接收中断
   {
        da = USART_ReceiveData(USART2);
        
        //printf("c%d\n",da);
        if(AT_CMD_MASK&At_cmd_state)
        {
            uart2_buffer[uart2_cnt++] = da ;
            if(uart2_cnt >= sizeof(uart2_buffer))
                uart2_cnt = 0;
            if(At_cmd_state == AT_CMD_WAIT_A)  //wait cmd back ‘a'
            {
                if(da == 'a')
                {
                    At_cmd_state = AT_CMD_WAIT_OK;
                }   
            }
            else if(At_cmd_state == AT_CMD_WAIT_OK) //wait cmd back '+OK'
            {
                if(strcmp(AT_OK,(const char *)uart2_buffer) == 0)
                {
                    At_cmd_state = AT_CMD_WAIT_OK_PASS;
                    //printf("OK PASS\n");
                }
            }
            
            else if(At_cmd_state == AT_CMD_WAIT_AT_BACK) //wait normal AT back 
            {
                
                if((da == 0xa)&&(uart2_cnt>3))
                {
                    At_cmd_state = AT_CMD_WAIT_AT_BACK_PASS;
                }
            }
        }
        else
        {
            if(AT_DATA_WAIT_DATA == At_cmd_state)
            {
               wireless_rx_buff[wireless_rx_cnt++]=da;
               if(wireless_rx_cnt>=wireless_rx_buff_size)
                   wireless_rx_cnt= 0;
            }
       }
	}


}
