#include "GPIO.h"
#include "delay.h"



void Led_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOA ±÷”


	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6);
}

void PowerControl_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	//GPIOA ±÷”


	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_11);
}

void PowerControl_DisInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	//GPIOA ±÷”
	GPIO_SetBits(GPIOA,GPIO_Pin_11);
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	//GPIO_Init(GPIOA, &GPIO_InitStructure);
	//GPIO_SetBits(GPIOA,GPIO_Pin_11);
}

void Led_Open(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);
}

void Led_Close(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_6);
}

void Led_toggle(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_6);
	if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_6))
		Led_Close();
	else
		Led_Open();
}

void LOW_Power_Init(void)
{


}
