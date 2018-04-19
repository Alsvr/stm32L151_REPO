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
#include "adc.h"
#include "lowpower.h"
#include "FM25WXX.h" 
#include "dataStore.h"
#include "uart.h"
#include "ds18b20.h"

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

//extern uint8_t MDK;
//extern uint8_t NbrOfDataToRead;
//extern __IO uint8_t TxCounter; 
//extern __IO uint16_t RxCounter; 
/* Private function prototypes -----------------------------------------------*/
void NVIC_Config(void);
 void RTC_Config(void); 
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
  



uint8_t Rx[1024];
short temp;

int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32l1xx_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32l1xx.c file
     */  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Led_Init();
    Uart_Log_Configuration();
    delay_init(32);
    FM25VXX_Init();
    GetGlobalData();
    
    //printf("USART1 TEST\r\n");
    //Init_CC3200(0xC0,0x1210,115200,80);
    //test_FM25V05();
    bsp_InitDS18B20();
    //PowerControl_DeInit();
    //ADCStartSample();
    //while(DS18B20_Init())
    //{
     //   delay_ms(4);
    //}
    temp = DS18B20_ReadTempReg();
    printf("temp is %d\n",temp);
    bsp_DeInitDS18B20();
    while(1)
    {
        //PowerControl_Init();
        delay_ms(4000);
        Handler_PC_Command();
        PowerControl_DeInit();
        delay_ms(10);
    }



         

	RTC_Config();
	
	while (1)
	{
		//Led_Init();
		//delay_init(32);
		//Led_Open();
		//delay_ms(10);
		//Led_Close();
		//Wireless_power_down();
        Enter_Stop_Mode();
		
		To_Exit_Stop();
		//Wireless_power_on();
		delay_ms(1000);
//		delay_ms(30000);	
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
	}
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



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
