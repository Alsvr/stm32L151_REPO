#include "adc.h"
#include "GPIO.h"
#include "stdio.h"
#include "delay.h"
//ADC_IN    PA8
//ADC_CON   PB5
//ADC_SCLK  PB13
//ADC_DOUT  PB14

const uint16_t power_rate_list[10]={00,2600,2700,2800,2900,3100,3200,3400,3500,3600};

static uint16_t power_rate;
static uint32_t power_adc =0;

static uint32_t adc_value_raw =0;
static uint16_t adc_Vrefint_cal =0;
uint16_t ADC_Config(void)
{

  GPIO_InitTypeDef GPIO_InitStructure;
  ADC_InitTypeDef   ADC_InitStructure;
  uint8_t i=0;
  /* Enable The HSI (16Mhz) */
  RCC_HSICmd(ENABLE);

  /* Enable the GPIOA Clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  /* Configure PA.0 (ADC Channe0)in analog mode */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Check that HSI oscillator is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);

  /* ADC1 Configuration ------------------------------------------------------*/
  
  /* Enable ADC1 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  ADC_DeInit(ADC1);
  ADC_StructInit(&ADC_InitStructure);
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel0 configuration */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_384Cycles);
  
  /* Define delay between ADC1 conversions */
  ADC_DelaySelectionConfig(ADC1, ADC_DelayLength_Freeze);
  
  /* Enable ADC1 Power Down during Delay */
  ADC_PowerDownCmd(ADC1, ADC_PowerDown_Idle_Delay, ENABLE);
  
  /* Enable ADC1 */
  ADC_TempSensorVrefintCmd(ENABLE);

  ADC_Cmd(ADC1, ENABLE);
  

  /* Wait until ADC1 ON status */
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
  {
  }

  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConv(ADC1);

  for(i=0;i<16;i++)
  {
      while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
      {
      }
      adc_value_raw += ADC_GetConversionValue(ADC1);
  }
  adc_value_raw >>=4;
  
  //printf("Adc mease is %d\n",adc_value_raw);
  
  /**************/

  power_adc=2600*adc_value_raw;

  ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_384Cycles);

  ADC_Cmd(ADC1, ENABLE);
  

  /* Wait until ADC1 ON status */
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
  {
  }

  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConv(ADC1);
  //while(1)
  //{
  /* Wait until ADC1 ON status */
  for(i=0;i<16;i++)
  {
      while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
      {
      }
      adc_value_raw += ADC_GetConversionValue(ADC1);
  }
  adc_value_raw >>=4;
  //printf("Adc ref is %d\n",adc_value_raw);
  //adc_Vrefint_cal = *(__IO uint16_t *)(0X1FF80078);
  power_adc/=adc_value_raw + 10;

  

  ADC_DeInit(ADC1);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
  
  for(i=9;i>=0;i--)
  {
      if(power_adc>power_rate_list[i])
      {
          power_rate=(i+1)*10;
          printf("power voltage is %d%% v=%d\n",power_rate,power_adc);
          return power_rate;
      }
      
  }
  
  return 100;

}


//void ADCStartSample(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOAÊ±ÖÓ GPIOBÊ±ÖÓ

//    //ADC_CON
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
//	GPIO_InitStructure.GPIO_Pin = GPIO_ADC_CON;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//	GPIO_Init(PORT_ADC_CON, &GPIO_InitStructure);
//	GPIO_ResetBits(PORT_ADC_CON,GPIO_ADC_CON);
//	//ADC_SCLK
//	
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
//	GPIO_InitStructure.GPIO_Pin = GPIO_ADC_SCLK;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//	GPIO_Init(PORT_ADC_SCLK, &GPIO_InitStructure);
//	GPIO_ResetBits(PORT_ADC_SCLK,GPIO_ADC_SCLK);
//	
//	//DIN = DVDD
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
//	GPIO_InitStructure.GPIO_Pin = GPIO_ADC_IN;
//	GPIO_Init(PORT_ADC_IN, &GPIO_InitStructure);
//	GPIO_SetBits(PORT_ADC_IN,GPIO_ADC_IN);
//	
//	//ADC_DOUT
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_InitStructure.GPIO_Pin = GPIO_ADC_DOUT;
//	GPIO_Init(PORT_ADC_DOUT, &GPIO_InitStructure);	
//   
//}	

static int16_t buffer_cnt=0;

void EXTI15_10_IRQHandler(void)
{ 
    uint32_t adc_da=0;
    int16_t j=0;
    uint32_t adc_data=0;
    if (EXTI_GetITStatus(EXTI_Line14) != RESET)
    {
        
        /* Clear the EXTI Line 0 */
        //adc_da=Sampleone();

        //adc_buffer[buffer_cnt++]=adc_da;//(adc_da>>14);//&0xFFFC0000;
        //if(buffer_cnt==1024)
        //    buffer_cnt=0;
        //EXTI_ClearITPendingBit(EXTI_Line14);


    }
}
static uint16_t ADCVal;

void ADC1_IRQHandler(void)
{
  if(ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET)
  {
    /* Get converted value */
    ADCVal = ADC_GetConversionValue(ADC1);
    printf("adc value is %d \n ",ADCVal);
    /* Clear EOC Flag */
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
  }
}


