#include "stdio.h"
#include "wireless.h"

//433M AUX --->PA1
//433M_MO  --->PB8
//433M_M1  --->PB9
//433M_POWER  --->PB10
//433M_TX  --->PA2
//433M_RX  --->PA3
#define M0_GPIO GPIO_Pin_8
#define M1_GPIO GPIO_Pin_9
uint8_t wireless_rx_buff[64]; 
static int wireless_rx_cnt=0;
void wire_less_uart_init(uint32_t buand)
{
	
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//ʹ��USART1
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	//GPIOAʱ��

	USART_DeInit(USART2);  //��λ����2

	//USART2_TX   PA.2 PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure); 

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART1);


	USART_InitStructure.USART_BaudRate = buand;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2,&USART_InitStructure);

	USART_Cmd(USART2,ENABLE);

	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//����USART1�Ľ����ж�
	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

	USART_ClearFlag(USART2,USART_FLAG_TC); //���������ɱ�־λ 
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
	uint8_t uart_data=0;
void init_wireless(	uint8_t HEAD,
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
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
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
		USART_SendData(USART2,uart_data); //�����������жϵ�ʱ��,���ո����ݣ�Ȼ���ٴӴ���1�����ݷ��ͳ�ȥ
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);//�ȴ������������);		
		
	}
	delay_ms(100);
	Wireless_wakeup_mode();

}






void USART2_IRQHandler(void)
{
   
   if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET) //�ж��Ƿ���������ж�
	{

		wireless_rx_buff[wireless_rx_cnt++]=USART_ReceiveData(USART2);
		
	}


}