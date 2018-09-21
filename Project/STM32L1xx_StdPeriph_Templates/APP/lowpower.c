#include "lowpower.h"



void Enter_Stop_Mode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;  
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOH,ENABLE);
	
	DBGMCU_Config(DBGMCU_STOP,ENABLE);
	
    //RCC_AHBPeriphClockLPModeCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
	GPIO_Init(GPIOH, &GPIO_InitStructure);
	
	
	//wifi power down
	//Wireless enter low power 
//433M AUX --->PA1
//433M_MO  --->PB8
//433M_M1  --->PB9
//433M_POWER  --->PB10
//433M_TX  --->PA2
//433M_RX  --->PA3
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All&(~(GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_9|GPIO_Pin_10));
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All&(~(GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10));
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    

//     GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
//     GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
//     GPIO_Init(GPIOA, &GPIO_InitStructure);
    
	/* Enable Wakeup Counter */
	RTC_WakeUpCmd(ENABLE);
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
	RTC_WakeUpCmd(DISABLE);	
}


void RTC_Config(uint8_t second)
{
  uint16_t RTC_CW=0;  
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


  if(second >30)
    second=30;
  RTC_CW = second*2048;

  RTC_SetWakeUpCounter(RTC_CW);

  /* Enable the Wakeup Interrupt */
  RTC_ITConfig(RTC_IT_WUT, ENABLE);  
}

void To_Exit_Stop(void)
{

  RTC_WakeUpCmd(DISABLE);
    /* Enable HSI Clock */
  RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
    {}
    
    /* Enable PLL */
    RCC_PLLCmd(ENABLE);
    
    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {}
    
    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    
    /* Wait till PLL is used as system clock source */
    while (RCC_GetSYSCLKSource() != 0x0C)
    {}

//    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC |
//                        RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH |
//                          RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_GPIOG, ENABLE);

//    GPIO_DeInit(GPIOA);
//    GPIO_DeInit(GPIOB);
//    GPIO_DeInit(GPIOC);
//    GPIO_DeInit(GPIOD);
//    GPIO_DeInit(GPIOE);
//    GPIO_DeInit(GPIOF);
//    GPIO_DeInit(GPIOG);
//    GPIO_DeInit(GPIOH);


}
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
