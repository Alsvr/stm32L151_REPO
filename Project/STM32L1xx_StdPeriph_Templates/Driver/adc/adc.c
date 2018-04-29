#include "adc.h"
#include "GPIO.h"
#include "stdio.h"
#include "delay.h"
//ADC_IN    PA8
//ADC_CON   PB5
//ADC_SCLK  PB13
//ADC_DOUT  PB14




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


