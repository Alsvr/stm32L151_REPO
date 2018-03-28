/**
  ******************************************************************************
  * @file    USART/HyperTerminal_Interrupt/main.c 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    16-May-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx.h"
#include "stdio.h"
#include "wireless.h"
#include "delay.h"
#include "GPIO.h"
#include "lpm.h"
/** @addtogroup STM32L1xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup USART_HyperTerminal_Interrupt
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure;
extern uint8_t NbrOfDataToTransfer;
extern uint8_t NbrOfDataToRead;
extern __IO uint8_t TxCounter; 
extern __IO uint16_t RxCounter; 
/* Private function prototypes -----------------------------------------------*/
void NVIC_Config(void);
 void RTC_Config(void); 
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
  
int fputc(int ch,FILE *f)
{
   USART_SendData(USART1,(uint8_t)ch);
   while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送数据完毕);
   return ch;
}

void USART_Configuration(void)
{
USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//使能USART1
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	//GPIOA时钟
    
	  //NVIC_Configuration();
	
 	  USART_DeInit(USART1);  //复位串口1
    
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    

	  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    
    
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1,&USART_InitStructure);
		
    USART_Cmd(USART1,ENABLE);
		
		USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//开启USART1的接收中断
		//Usart1 NVIC 配置
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
		
		USART_ClearFlag(USART1,USART_FLAG_TC); //清除发送完成标志位 
		
}
uint8_t Rx[1024];
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32l1xx_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32l1xx.c file
     */ 
    GPIO_InitTypeDef GPIO_InitStructure;  	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(32);
	//USART_Configuration();
	//init_wireless(0xC0,0x1210,115200,80);
	
	//printf("USART1 TEST\r\n");
	//PWR_EnterSleepMode();
	//DBGMCU_Config(DBGMCU_STOP,ENABLE);

//RCC_AHBPeriphClockLPModeCmd(RCC_AHBPeriph_GPIOA,ENABLE);
 
//    RCC_AHBPeriphResetCmd(RCC_AHBPeriph_GPIOA, ENABLE);  
//    RCC_AHBPeriphResetCmd(RCC_AHBPeriph_GPIOB, ENABLE);  
//    RCC_AHBPeriphResetCmd(RCC_AHBPeriph_GPIOH, ENABLE);
//	RCC_AHBPeriphResetCmd(RCC_AHBPeriph_GPIOC, ENABLE);  		



//    GPIO_DeInit(GPIOA);
//    GPIO_DeInit(GPIOB);    
//    GPIO_DeInit(GPIOC);
         
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOH,ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
	GPIO_Init(GPIOH, &GPIO_InitStructure);
	RTC_Config();
	
	while (1)
	{
		Led_Init();
		delay_init(32);
		Led_Open();
		delay_ms(10);
		Led_Close();
		    /* Enable Wakeup Counter */
        RTC_WakeUpCmd(ENABLE);
		PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
		RTC_WakeUpCmd(DISABLE);
		//PWR_EnterSTANDBYMode();

	
    /* Enable Wakeup Counter */
    
    /* After wake-up from STOP reconfigure the system clock */
    /* Enable HSE */

	}
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

void RTC_Config(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;
  EXTI_InitTypeDef  EXTI_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /*!< Allow access to RTC */
  PWR_RTCAccessCmd(ENABLE);

  /*!< Reset RTC Domain */
  RCC_RTCResetCmd(ENABLE);
  RCC_RTCResetCmd(DISABLE);

  /*!< LSE Enable */
  RCC_LSEConfig(RCC_LSE_ON);

  /*!< Wait till LSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {}

  /*!< LCD Clock Source Selection */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /*!< Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /*!< Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

  /* EXTI configuration *******************************************************/
  EXTI_ClearITPendingBit(EXTI_Line20);
  EXTI_InitStructure.EXTI_Line = EXTI_Line20;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  /* Enable the RTC Wakeup Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
 
  /* RTC Wakeup Interrupt Generation: Clock Source: RTCDiv_16, Wakeup Time Base: 4s */
  RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
  RTC_SetWakeUpCounter(0x1ffF);

  /* Enable the Wakeup Interrupt */
  RTC_ITConfig(RTC_IT_WUT, ENABLE);  
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
