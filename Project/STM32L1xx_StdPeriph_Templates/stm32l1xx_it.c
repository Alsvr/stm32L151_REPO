/**
  ******************************************************************************
  * @file    USART/HyperTerminal_Interrupt/stm32l1xx_it.c 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    16-May-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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
#include "stm32l1xx_it.h"
#include "delay.h"
#include "GPIO.h"

/** @addtogroup STM32L1xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup USART_HyperTerminal_Interrupt
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define USARTx_IRQHANDLER   USART1_IRQHandler

#define TXBUFFERSIZE   (countof(TxBuffer) - 1)
#define RXBUFFERSIZE   0x80

/* Private macro -------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))

/* Private variables ---------------------------------------------------------*/
uint8_t TxBuffer[] = "\n\rUSART Hyperterminal Interrupts Example: USART-Hyperterminal\
 communication using Interrupt\n\r";
uint8_t RxBuffer[RXBUFFERSIZE];
uint8_t NbrOfDataToTransfer = TXBUFFERSIZE;
uint8_t NbrOfDataToRead = RXBUFFERSIZE;
__IO uint8_t TxCounter = 0; 
__IO uint16_t RxCounter = 0; 
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*            STM32L1xx Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles USARTx global interrupt request.
  * @param  None
  * @retval None
  */
char i=0;

void USARTx_IRQHANDLER(void)
{
   int j=0,k=0;
   if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET) //判断是否产生接收中断
   {
		RxBuffer[i]=USART_ReceiveData(USART1);
		if(RxBuffer[i++]==0x0a)
		{
			Wireless_wakeup_mode();
			for(k=0;k<128;k++)
			{
				
			USART_SendData(USART1,0x00); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
		    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送数据完毕);	
			USART_SendData(USART1,0x01); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
		     while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送数据完毕);			
		     
			USART_SendData(USART1,0x50); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送数据完毕);	
			for(j=0;j<40;j++)
			{
				
				


			  USART_SendData(USART1,RxBuffer[j]+j); //当产生接收中断的时候,接收该数据，然后再从串口1把数据发送出去
		      while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送数据完毕);	
			}
			while(!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1));
			
		}
			i=0;
			
		}
		
		if(i==RXBUFFERSIZE)
		   i=0;

   }


}

/******************************************************************************/
/*                 stm32l1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l1xx_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void)
{
  if (EXTI_GetITStatus(GPIO_Pin_1) != RESET)
  {
    /* Clear the EXTI Line 0 */
    EXTI_ClearITPendingBit(GPIO_Pin_1);
	
  }
}



void RTC_WKUP_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
  {
    /* Toggle LED1 */
    //STM_EVAL_LEDToggle(LED1);
    RTC_ClearITPendingBit(RTC_IT_WUT);
    EXTI_ClearITPendingBit(EXTI_Line20);
  } 
}

/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
