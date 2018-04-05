#include "adc.h"
#include "GPIO.h"
//#include "delay.h"
#include "delay.h"
//ADC_IN    PA8
//ADC_CON   PB5
//ADC_SCLK  PB13
//ADC_DOUT  PB14
#define GPIO_ADC_CON GPIO_Pin_5
#define PORT_ADC_CON GPIOB

#define GPIO_ADC_IN  GPIO_Pin_8
#define PORT_ADC_IN GPIOA

#define GPIO_ADC_SCLK GPIO_Pin_13
#define PORT_ADC_SCLK GPIOB

#define GPIO_ADC_DOUT GPIO_Pin_14
#define PORT_ADC_DOUT GPIOB

uint16_t adc_buffer[4096];
uint32_t Sample(uint16_t speed,uint32_t count) //K
{
	uint16_t adc_us,i;
	uint8_t j;
	uint32_t adc_data=0;
	adc_us=1000/speed;
	GPIO_SetBits(PORT_ADC_IN,GPIO_ADC_IN);  //indecate that adc work as 4 wire
	delay_us(100);
	GPIO_ResetBits(PORT_ADC_DOUT,GPIO_ADC_SCLK);
	for(i=0;i<count;i++)
	{
		GPIO_SetBits(PORT_ADC_CON,GPIO_ADC_CON);
		delay_us(adc_us>>1);
		GPIO_ResetBits(PORT_ADC_CON,GPIO_ADC_CON);
		delay_us(1);
		for(j=0;j<18;j++)
		{
			//GPIO_SetBits(PORT_ADC_SCLK,GPIO_ADC_SCLK);
			PORT_ADC_SCLK->BSRRL = GPIO_ADC_SCLK;
			//adc_data=(adc_data<<1)+GPIO_ReadInputDataBit(PORT_ADC_DOUT,GPIO_ADC_DOUT);
			adc_data=(adc_data<<1)|(PORT_ADC_DOUT->IDR & GPIO_ADC_DOUT);
			PORT_ADC_SCLK->BSRRH = GPIO_ADC_SCLK;
			//GPIO_ResetBits(PORT_ADC_SCLK,GPIO_ADC_SCLK);
		}
		adc_buffer[i]=(adc_data>>14);//&0xFFFC0000;
		delay_us(adc_us>>1);
	}
	
}

__inline uint32_t Sampleone(void) //K
{
	uint8_t j=0;
	uint32_t adc_data=0;
	GPIO_ResetBits(PORT_ADC_SCLK,GPIO_ADC_SCLK);
	delay_us(10);
	for(j=0;j<19;j++)
	{
		PORT_ADC_SCLK->BSRRL = GPIO_ADC_SCLK;
		__nop();
		PORT_ADC_SCLK->BSRRH = GPIO_ADC_SCLK;
		adc_data=(adc_data<<1)|(PORT_ADC_DOUT->IDR & GPIO_ADC_DOUT);
	}
	return adc_data;
}

void StartADC(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOA时钟 GPIOB时钟

    //ADC_CON
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_ADC_CON;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(PORT_ADC_CON, &GPIO_InitStructure);
	GPIO_ResetBits(PORT_ADC_CON,GPIO_ADC_CON);
	//ADC_SCLK
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_ADC_SCLK;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(PORT_ADC_SCLK, &GPIO_InitStructure);
	GPIO_ResetBits(PORT_ADC_SCLK,GPIO_ADC_SCLK);
	
	//DIN = DVDD
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_ADC_IN;
	GPIO_Init(PORT_ADC_IN, &GPIO_InitStructure);
	GPIO_SetBits(PORT_ADC_IN,GPIO_ADC_IN);
	
	//ADC_DOUT
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_ADC_DOUT;
	GPIO_Init(PORT_ADC_DOUT, &GPIO_InitStructure);	
   
    //RCC_APB2PeriphClockCmd(RCC_TOUCHOUT|RCC_APB2Periph_AFIO, ENABLE);//打开GPIO AFIO的时钟  
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);　
	//GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource2);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource14);
    /* Configure EXTI1 line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line14;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	/* Enable and set EXTI0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	
	//timer init
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 63;
    TIM_TimeBaseStructure.TIM_Prescaler = 100;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    /* TIM IT enable */
    TIM_ITConfig(TIM2,TIM_IT_Update, ENABLE);

    /* TIM2 enable counter */
    TIM_Cmd(TIM2, ENABLE);
	

	/* Enable the TIM2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
		
	PowerControl_Init();
	
	delay_ms(1000);
	//Sample(2,1024);
	
	//PowerControl_DisInit();
}	

static int16_t buffer_cnt=0;
void EXTI15_10_IRQHandler(void)
{ 
	uint32_t adc_da=0;
    int16_t j=0;
    uint32_t adc_data=0;
	if (EXTI_GetITStatus(EXTI_Line14) != RESET)
	{
		/* Clear the EXTI Line 0 */
		adc_da=Sampleone();
		EXTI_ClearITPendingBit(EXTI_Line14);
		adc_buffer[buffer_cnt++]=(adc_da>>14);//&0xFFFC0000;
		if(buffer_cnt==1014)
			buffer_cnt=0;
		

	}
}

void TIM2_IRQHandler(void)
{

  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
  {
     TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	 // GPIO_ToggleBits(PORT_ADC_SCLK,GPIO_ADC_SCLK);
	 GPIO_SetBits(PORT_ADC_CON,GPIO_ADC_CON);
	 delay_us(3);
	 GPIO_ResetBits(PORT_ADC_CON,GPIO_ADC_CON);
  }
}


