#include "ADS869x.h"
#include "GPIO.h"
#include "delay.h"
#include "math.h"
#include "stdio.h"
#include "fifoqueue.h"
#include "mempool.h"
#include "dataStore.h"
#include "FM25WXX.h"
//#define N  5  //��ֵ�˲��㷨 ���ݸ���

//����ȫ�ֱ���
//unsigned char G_ACTIVE_FLAG=0x00;  //���������ѹ���߱�־λ

//unsigned long G_VAL=131076; 
//float G_RANGE=6;
//unsigned char G_F_Flag =0;

static uint16_t adc_buffer[512];
static uint16_t buffer_cnt=0;

#if 0
//�˲��㷨  ��ֵ�˲��㷨
/*************************
�����ܣ��˲��㷨  ��ֵ�˲��㷨
��������
����ֵ���˲� ʵ�ʵ�ѹֵ��1000,000��
**************************/
unsigned long value_buf[N];  
unsigned long Get_ADC_Value(void)  
{  
   unsigned long value_buf[N];  
   char count,i,j;
	 unsigned long temp;  
	
   for ( count=0;count<N;count++)  
   {  
//		 ADS869x_ReadREG(0x04);	
		 value_buf[count] = ADS869x_ReadData()>>14;
//		 ADS869x_ReadREG(0x04);	
   }  
   for (j=0;j<N-1;j++)  
   {  
      for (i=0;i<N-j;i++)  
      {  
         if ( value_buf[i]>value_buf[i+1] )  
         {  
            temp = value_buf[i];  
            value_buf[i] = value_buf[i+1];   
             value_buf[i+1] = temp;  
         }  
      }  
   }  
   return value_buf[(N-1)/2];  
}


/*************************
�����ܣ���ȡ��ѹֵ
��������
����ֵ��ʵ�ʵ�ѹֵ��1000,000��
**************************/
unsigned long Get_Vol(void)
{
	unsigned long vol=0;
	static unsigned long Rvol=0;
	float temp=0;

	ADS869x_NAP_EXIT();    //�˳�����ģʽ ���в���
//	delay_us(20);
	vol = Get_ADC_Value(); //����ADC����	
	
	ADS869x_GO_NAP();      //������ɽ�������ģʽ
	ADS869x_GO_PD();       //������ɽ���PDģʽ
	
	if(vol>=G_VAL)
	{
		vol = vol-G_VAL;	
		temp = vol*4.096/pow(2,18)*G_RANGE;  //����ʵ�ʵĵ�ѹֵ 
		vol =	temp*1000000;     //ʵ�ʵ�ѹֵ*1 v 000 mv 000 uv	  ��ʵ�ʵ�ѹֵ�Ŵ� 1000 000��
		G_F_Flag=0;  //�������
	}
	else
	{
		vol = G_VAL - 1 - vol;	
		temp = vol*4.096/pow(2,18)*G_RANGE;
		vol =	temp*1000000;	 
		G_F_Flag=1;  //�������
	}
	/*�������뷶Χ
   0000b ����    3*4.096  0x0000		6
   0001b ����  2.5*4.096  0x0001
   0010b ����  1.5*4.096  0x0002		3
   0011b ���� 1.25*4.096  0x0003		
   0100b ����0.626*4.096  0x0004	 	1.25 
*/	
	if ( temp<2 && G_RANGE!=1.25 ) 
	{
//			ADS869x_WriteREG(ADS869x_RANGE_SEL,0x0004);		//���� ���뷶ΧΪ 0000b ����2.5*4.096	
//			G_RANGE=1.25;	
//			OLED_ShowString(110,6,"2 ");	
//			G_VAL=131087; 
			return Rvol;		
	}
	else if( temp>=2 && temp<5 && G_RANGE!=3)
	{
//			ADS869x_WriteREG(ADS869x_RANGE_SEL,0x0002);		//���� ���뷶ΧΪ 0000b ����1.5*4.096	
//			G_RANGE=3;
//			OLED_ShowString(110,6,"5 ");
//			G_VAL=131078; 		
			return Rvol;		
	}	
	else if(temp >=5 && G_RANGE!=6 )
	{
//			ADS869x_WriteREG(ADS869x_RANGE_SEL,0x0000);		//���� ���뷶ΧΪ 0000b ����2.5*4.096	
//			G_RANGE=6;		
//			OLED_ShowString(110,6,"10");	
			G_VAL=131076; 
			return Rvol;	
	}		
	else if (temp >10.05)
	{
	//	ALARM_ON();
	}
	else
	{
		Rvol = vol;		
	}
	return Rvol;		
}

#endif

/*************************
�����ܣ�ADS869x_IO_Init
��������
����ֵ����
**************************/
//PB15 DO0
//PB14 DO1
//PB13 RVS
//PB5  CON
//PB4  RST
//PB3  SDI
//PA8  SCLK

void ADS869x_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB,ENABLE); //GPIOAʱ�� GPIOBʱ��

    //ADC_CON
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Pin = ADS869x_MOSI_GPIO|ADS869x_RST_GPIO|ADS869x_CS_GPIO;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(ADS869x_MOSI_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ADS869x_SCLK_GPIO;
    GPIO_Init(ADS869x_SCLK_GPIO_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ADS869x_MISO_GPIO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  //����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  //����
    GPIO_Init(ADS869x_MISO_GPIO_PORT, &GPIO_InitStructure);	  //��ʼ��GPIOD3,6


    GPIO_InitStructure.GPIO_Pin = ADS869x_DO1_GPIO|ADS869x_RVS_GPIO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;  //����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  //����
    GPIO_Init(ADS869x_DO1_GPIO_PORT, &GPIO_InitStructure);   //

    ADS869x_RST_Clr();
    delay_ms(20);
    ADS869x_RST_Set();
    ADS869x_CS_Set();
    ADS869x_SCLK_Clr();
    ADS869x_SetRefMode();
}

void ADS869x_DeInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOAʱ�� GPIOBʱ��


    ADS869x_GO_PD();
    //ADC_CON
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_3|GPIO_Pin_14|GPIO_Pin_15|GPIO_Pin_13;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


}


/*************************
�����ܣ�ADS869x��д8λ����
������ 
			dat��    д���ֵ
����ֵ����
**************************/
unsigned char ADS869x_ReadWrite(unsigned char dat)
{
	unsigned char i,temp=0;

	for(i=0;i<8;i++)
	{
		ADS869x_SCLK_Clr(); //����ʱ��
		delay_us(2);
		if(dat&0x80) 	
            ADS869x_MOSI_Set(); 
		else			
            ADS869x_MOSI_Clr();
		dat<<=1;
		ADS869x_SCLK_Set();
		delay_us(2);
		temp<<=1;
		if(ADS869x_MISO) temp++;	
	}
    delay_us(2);
    ADS869x_SCLK_Clr(); //����ʱ��
	return temp;	
}
/*************************
�����ܣ�ADS869x_SendCMD  д����
������cmd��    д������� 
			dat��    д��ĵ�ַ
����ֵ����
**************************/
void ADS869x_SendCMD(unsigned char cmd, uint8_t addr)
{
	ADS869x_CS_Clr();          //����Ƭѡ����ʼ�������ݻ������
	ADS869x_ReadWrite(cmd+addr);    //�������
	ADS869x_ReadWrite(addr);	 //���͵�ַ
	ADS869x_ReadWrite(0x00);
	ADS869x_ReadWrite(0x00);
	ADS869x_CS_Set();	         //����Ƭѡ����ʼ����ת����
}

/*************************
�����ܣ�ADS869x_ReadREG������
��������
����ֵ�����յ�32 λ����
**************************/
unsigned long ADS869x_ReadREG(unsigned char addr)  
{
	uint16_t sum=0;
    uint32_t time_out=0;
	ADS869x_CS_Clr();										 //����Ƭѡ����������
	while(ADS869x_RVS)
    {
        time_out++;
        if(time_out>320000)
            printf("time_out %d\n",time_out);
    }
	ADS869x_ReadWrite(((ADS869x_READ_HWORD<<8)+addr)>>8);    //����д����
	ADS869x_ReadWrite(addr);
	sum = ADS869x_ReadWrite(0)<<8;
	sum += ADS869x_ReadWrite(0);
	ADS869x_CS_Set();											//����Ƭѡ���������������	
	
	return sum;                          //��������
}
/************************************************************/
/*************************
�����ܣ�ADS869x_WriteREG д�Ĵ���
������regaddr���Ĵ�����ַ
      dat��    д���ֵ  16bit
����ֵ����
**************************/
void ADS869x_WriteREG(unsigned char regaddr,unsigned short dat)
{
	uint8_t dat_H=0,dat_L=0;
    uint32_t time_out=0;
	dat_L = dat&0xFF;
	dat_H = (dat>>8)&0xFF;
    
	ADS869x_CS_Clr();										 //����Ƭѡ����������
	while(ADS869x_RVS)
    {
        time_out++;
        if(time_out>320000)
            printf("time_out %d\n",time_out);
    }
	ADS869x_ReadWrite(((ADS869x_WRITE<<8)+regaddr)>>8);    //����д����
	ADS869x_ReadWrite(regaddr);	
	ADS869x_ReadWrite(dat_H);
	ADS869x_ReadWrite(dat_L);
	ADS869x_CS_Set();											//����Ƭѡ���������������	
}
/*************************
�����ܣ�ADS869x_ReadData������
��������
����ֵ�����յ�32 λ����
**************************/
uint32_t ADS869x_ReadData(void)  
{
	unsigned char i=0;
	unsigned long sum=0;
	unsigned long r=0;
    uint32_t time_out=0;
	ADS869x_CS_Clr();
    while(ADS869x_RVS)
    {
        time_out++;
        if(time_out>320000)
            printf("time_out %d\n",time_out);
    }
	for(i=0;i<4;i++)                      //��������32λ
	{
		sum = sum << 8;
		r = ADS869x_ReadWrite(0x00);    //��������
		sum |= r;             
	}
	ADS869x_CS_Set();				
	return sum&0xffff;                          //��������
}

uint32_t ADS869x_ReadData16(void)  
{
	unsigned char i=0;
	unsigned long sum=0;
	unsigned long r=0;
    uint32_t time_out=0;
	ADS869x_CS_Clr();
    while(ADS869x_RVS)
    {
        time_out++;
        if(time_out>320000)
            printf("time_out %d\n",time_out);
    }
	for(i=0;i<4;i++)                      //��������32λ
	{
		sum = sum << 8;
		r = ADS869x_ReadWrite(0x00);    //��������
		sum |= r;             
	}
	ADS869x_CS_Set();				
	return sum&0xffff;                          //��������
}


/*************************
�����ܣ�ADS869x_Over_Judgment FLAG�ж�
��������
����ֵ����
**************************/
void ADS869x_Over_Judgment(void)  
{
	unsigned char i=0;
	unsigned long sum=0;
	unsigned long r=0;

	//IWDG_Feed();
//	while(!ADS869x_RVS);              //��DRDYΪ���ǲ���дָ�� 
	ADS869x_CS_Clr();
	delay_us(25);            						//��ʱʱ��
	for(i=0;i<4;i++)                      //��������32λ
	{
		sum = sum << 8;
		r = ADS869x_ReadWrite(0x00);    //��������
		sum |= r;             
	}		
	ADS869x_CS_Set();
	
	//�жϷ�Χ
//	switch(sum&0x3C)
//	{
//		case 0x08 : G_ACTIVE_FLAG = 0x01;LED = 1;break;
//		case 0x04 : G_ACTIVE_FLAG = 0x02;LED = 1;break;
//		case 0x20 : G_ACTIVE_FLAG = 0x03;LED = 1;break;
//		case 0x10 : G_ACTIVE_FLAG = 0x04;LED = 1;break;
//		default: G_ACTIVE_FLAG=0;	LED = 0;break;
//	}
}
/************************************************************/
/*************************
�����ܣ�ADS869x_GO_NAP����˯��ģʽ
��������
����ֵ�����յ�32 λ����
**************************/
void ADS869x_GO_NAP(void)  
{
	ADS869x_WriteREG(0x04,0x6900); //��Ĵ�����ַ05H��д������69H����RST�Ĵ���
	ADS869x_WriteREG(0x04,0x0002); //��Ĵ�����ַ04H��д������69H����RST�Ĵ���	
//	ADS869x_ReadREG(0x04);	
    
}
/*************************
�����ܣ�ADS869x_NAP_EXIT�˳�˯��ģʽ
��������
����ֵ�����յ�32 λ����
**************************/
void ADS869x_NAP_EXIT(void)  
{
	ADS869x_WriteREG(0x05,0x6900); //��Ĵ�����ַ05H��д������69H����RST�Ĵ���
	ADS869x_WriteREG(0x04,0x0000); //��Ĵ�����ַ04H��д������69H����RST�Ĵ���	
//	ADS869x_ReadREG(0x04);	
}
/*************************
�����ܣ�ADS869x_GO_PD����PDģʽ
��������
����ֵ�����յ�32 λ����
**************************/
void ADS869x_GO_PD(void)  
{
    uint32_t reg_data=0;
	ADS869x_WriteREG(0x04,0x6900); //��Ĵ�����ַ05H��д������69H����RST�Ĵ���
	ADS869x_WriteREG(0x04,0x0001); //��Ĵ�����ַ04H��д������69H����RST�Ĵ���	
    reg_data = ADS869x_ReadREG(0x04);
    printf("reg4 is %d\n",reg_data);
}
/*************************
�����ܣ�ADS869x_PD_EXIT�˳�PDģʽ
��������
����ֵ�����յ�32 λ����
**************************/
void ADS869x_PD_EXIT(void)  
{
	ADS869x_WriteREG(0x14,0x0000); //��Ĵ�����ַ05H��д������69H����RST�Ĵ���
	ADS869x_WriteREG(0x04,0x0000); //��Ĵ�����ַ04H��д������69H����RST�Ĵ���	
//	ADS869x_ReadREG(0x04);	
}	

void ADS869x_SetRefMode(void)  
{
    uint32_t reg_data=0;
	ADS869x_WriteREG(0x14,0x000B); //��Ĵ�����ַ05H��д������69H����RST�Ĵ���
    reg_data = ADS869x_ReadREG(0x14);
    printf("reg14 is %d\n",reg_data);
}


uint16_t ADS869x_ReadADCData(void)  
{
    unsigned char i=0;
    uint32_t temp=0;
    ADS869x_CS_Clr();
    for(i=0;i<32;i++)
	{
		ADS869x_SCLK_Clr(); //����ʱ��
		//delay_us(20);
        ADS869x_MOSI_Clr();
		ADS869x_SCLK_Set();
		temp<<=1;
		if(ADS869x_MISO) 
            temp++;	
	}
    ADS869x_CS_Set();				
    return (temp>>16);//��������
}

void Timinit(uint16_t speed)
{
    //timer init
    uint16_t tim_prescaler =0;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    if((speed!=1000)||(speed!=2000)||(speed!=4000))
        tim_prescaler=1000;
    tim_prescaler=(500000/speed)-1;
    printf("tim_prescaler is %d\n",tim_prescaler);
    PowerControl_Init();
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 63;
    TIM_TimeBaseStructure.TIM_Prescaler = tim_prescaler;
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



}
void DeintTim(void)
{
    //timer init
    TIM_DeInit(TIM2);
}
ElemType sdat;
static uint16_t adc_packet_len=0;  //the num of packet of 128
static uint16_t collection_cnt=0;
static uint8_t _test_buffer[256];
static uint8_t ignore_num=0;



void ADS869x_Start_Sample(void)  
{
    uint8_t result,ii;
    uint16_t send_pkt=0;
    uint16_t adc_wait_overflow=0;
    adc_packet_len= Get_ADC_LEN();
    collection_cnt=0;
    delay_ms(3000);
    ADS869x_Init();
    FM25VXX_Init();
    ignore_num=0;
    buffer_cnt = 0;
    Timinit(GetADCSpeed());
    //    ��ʼ�ȴ�ADC���ݲɼ����
    for(send_pkt=0;;)  //Ϊ�˲����������ѭ������
    {
        delay_ms(1); //��ʱ1
        adc_wait_overflow++;
        if(adc_wait_overflow>=1000*15)  //> 15S break;
        {
            printf("adc time out:%d \n",adc_wait_overflow);
            break;
        }
//      ��ͣ�Ļ�ȡadc��ֵ
        result=QueueOut(GetFifo_Piot(),&sdat);
        if(result==QueueOperateOk){    //���µ�����
            FM25VXX_Write(sdat,send_pkt*ADC_PACKET_SIZE,ADC_PACKET_SIZE);   
            send_pkt++;
            memfree(sdat);
            sdat=NULL; 
        }
        if(send_pkt>=adc_packet_len){
            break;
        }
    }
    DeintTim();
    ADS869x_DeInit();
    //PowerControl_DeInit();
    //delay_ms(10);

}	

uint8_t ADS869x_Start_Sample_little(uint16_t *adc1,uint16_t *adc2)  
{
    uint8_t result,i;
    uint16_t send_pkt=0,* adc_p;
    uint16_t adc_wait_overflow=0;
    uint16_t adc_last=0,adc_current=0;
    uint32_t adc_sum=0;
    adc_packet_len= Get_ADC_LEN();
    collection_cnt=0;
    ADS869x_Init();
    ignore_num=0;
    buffer_cnt = 0;
    Timinit(1000);
    //��ʼ�ȴ�ADC���ݲɼ����
    for(send_pkt=0;;)  //Ϊ�˲����������ѭ������
    {
        delay_ms(1);  //��ʱ1ms
        adc_wait_overflow++;
        if(adc_wait_overflow>=1000*15)  //> 15S break;
            break;
//      ��ͣ�Ļ�ȡadc��ֵ
        result=QueueOut(GetFifo_Piot(),&sdat);
        if(result==QueueOperateOk){    //���µ�����
            adc_p =(uint16_t *)sdat;
            for(i=0;i<(ADC_PACKET_SIZE>>1);i++)
            {
                if(adc_current<adc_p[i])
                    adc_current=adc_p[i];
                adc_sum+=adc_p[i];
            }
            adc_sum/=(ADC_PACKET_SIZE>>1);
            *adc1 =adc_current-adc_sum;
            printf("adc sum is %d \n",*adc1);
            memfree(sdat);
            sdat=NULL;
            send_pkt++;
        }
        if(send_pkt>=1){
            break;
        }
    }
    DeintTim();
    ADS869x_DeInit();
    delay_ms(10);

}

uint8_t ADS869x_Start_Sample_Debug(void)  
{
    uint8_t result,i;
    uint16_t send_pkt=0,* adc_p;
    uint16_t adc_wait_overflow=0;
    uint16_t adc_last=0,adc_current=0;
    uint32_t adc_sum=0;
    uint8_t adc_debug_buffer[128];
    adc_packet_len= Get_ADC_LEN();
    collection_cnt=0;
    ADS869x_Start_Sample();
    
    for(send_pkt=0;send_pkt<adc_packet_len;send_pkt++){
        
        FM25VXX_Read(adc_debug_buffer,send_pkt*ADC_PACKET_SIZE,ADC_PACKET_SIZE);
        
        adc_p=(uint16_t *)adc_debug_buffer;
       
        for(i=0;i<(ADC_PACKET_SIZE>>1);i++)
        {
            printf("%d\n",*(adc_p++));
        }   
    }

}

void ADS869x_Stop_Sample(void)  
{
    ADS869x_DeInit();
    DeintTim();

}

static uint16_t adc_data=0;
static uint8_t *buffer;
#define IGNORE_NUM 64

void TIM2_IRQHandler(void)
{
    
    uint16_t result;
    Led_Open();
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

        if(collection_cnt>=adc_packet_len){
            Led_Close();
            return;
        }
        adc_data = ADS869x_ReadADCData();
        if(ignore_num<IGNORE_NUM)
        {
            ignore_num++;
            Led_Close();
            return;
        }
        adc_buffer[buffer_cnt++]=adc_data;
        if(buffer_cnt>=(ADC_PACKET_SIZE>>1))
        {
            
           // printf("%d\n",adc_data);
            buffer=memalloc();
            if(buffer!=NULL){

                memcpy(buffer,(uint8_t *)adc_buffer,ADC_PACKET_SIZE);

                result=QueueIn(GetFifo_Piot(),buffer);
                if(result==QueueFull)
                {
                    memfree(buffer);
                    buffer=NULL;

                    //����
                }
                collection_cnt++;
               
            }
            else
            {
                printf("adc error\n");
            }
            buffer_cnt=0;
        }


    }
    Led_Close();
}

/************************************************************/
