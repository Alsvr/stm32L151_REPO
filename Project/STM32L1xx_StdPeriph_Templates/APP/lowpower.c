#include "lowpower.h"



void Enter_Stop_Mode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;  
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOH,ENABLE);
	
	//DBGMCU_Config(DBGMCU_STOP,ENABLE);
	
    //RCC_AHBPeriphClockLPModeCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
	GPIO_Init(GPIOH, &GPIO_InitStructure);
	
	
	
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
	
	/* Enable Wakeup Counter */
	RTC_WakeUpCmd(ENABLE);
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
	RTC_WakeUpCmd(DISABLE);	
}

void To_Exit_Stop(void)
{

  RTC_WakeUpCmd(DISABLE);
    /* Enable HSI Clock */
  RCC_HSICmd(ENABLE);

  /*!< Wait till HSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);

  /* Enable 64-bit access */
  //FLASH_ReadAccess64Cmd(ENABLE);

  /* Enable Prefetch Buffer */
  //FLASH_PrefetchBufferCmd(ENABLE);

  /* Flash 1 wait state */
  //FLASH_SetLatency(FLASH_Latency_1);

  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

  while (RCC_GetSYSCLKSource() != 0x04);

  RCC_HCLKConfig(RCC_SYSCLK_Div1);  
  /* PCLK2 = HCLK */
  RCC_PCLK2Config(RCC_HCLK_Div1);

  /* PCLK1 = HCLK */
  RCC_PCLK1Config(RCC_HCLK_Div1);  

  PWR_VoltageScalingConfig(PWR_VoltageScaling_Range1);
  /* Wait Until the Voltage Regulator is ready */
  while (PWR_GetFlagStatus(PWR_FLAG_VOS) != RESET);

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