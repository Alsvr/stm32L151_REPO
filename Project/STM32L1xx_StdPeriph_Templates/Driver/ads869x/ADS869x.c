#include "ADS869x.h"
#include "GPIO.h"
#include "delay.h"
#include "math.h"
#include "stdio.h"
#include "fifoqueue.h"
#include "mempool.h"
#include "dataStore.h"
#include "FM25WXX.h"
#define N  5  //��ֵ�˲��㷨 ���ݸ���

//����ȫ�ֱ���
unsigned char G_ACTIVE_FLAG=0x00;  //���������ѹ���߱�־λ

unsigned long G_VAL=131076; 
float G_RANGE=6;
unsigned char G_F_Flag =0;

static uint16_t adc_buffer[512];
static uint16_t buffer_cnt=0;

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
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOAʱ�� GPIOBʱ��

    //ADC_CON
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_3;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15|GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  //����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  //����
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  //��ʼ��GPIOD3,6

    ADS869x_RST_Clr();
    delay_ms(10);
    ADS869x_RST_Set();
    ADS869x_CS_Set();
}

void ADS869x_DeInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOAʱ�� GPIOBʱ��

    //ADC_CON
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_3;
    GPIO_InitStructure.GPIO_OType = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15|GPIO_Pin_13;
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  //��ʼ��GPIOD3,6

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
		//delay_us(20);
		if(dat&0x80) 	
            ADS869x_MOSI_Set(); 
		else			
            ADS869x_MOSI_Clr();
		dat<<=1;
		ADS869x_SCLK_Set();
		//delay_us(10);
		temp<<=1;
		if(ADS869x_MISO) temp++;	
	}
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
	unsigned long sum=0;
	
	ADS869x_SendCMD(ADS869x_READ_HWORD,addr+2); //�ȶ���16λ��ַ��ֵ  #define ADS869x_READ_HWORD    0xC8
//	delay_ms(10);
	sum = ADS869x_ReadData(); //����16λ
	ADS869x_SendCMD(ADS869x_READ_HWORD,addr);   //�����16λ��ַ��ֵ  #define ADS869x_READ_HWORD    0xC8
	sum |= (ADS869x_ReadData()>>16); 
	
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
	unsigned char dat_H=0,dat_L=0;
	dat_L = dat;
	dat_H = dat>>8;

	ADS869x_CS_Clr();										 //����Ƭѡ����������
	ADS869x_ReadWrite(ADS869x_WRITE);    //����д����
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
	ADS869x_CS_Clr();
	delay_us(25);            						//��ʱʱ��
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
	ADS869x_WriteREG(0x04,0x6900); //��Ĵ�����ַ05H��д������69H����RST�Ĵ���
	ADS869x_WriteREG(0x04,0x0001); //��Ĵ�����ַ04H��д������69H����RST�Ĵ���	
//	ADS869x_ReadREG(0x04);	
}
/*************************
�����ܣ�ADS869x_PD_EXIT�˳�PDģʽ
��������
����ֵ�����յ�32 λ����
**************************/
void ADS869x_PD_EXIT(void)  
{
	ADS869x_WriteREG(0x04,0x6900); //��Ĵ�����ַ05H��д������69H����RST�Ĵ���
	ADS869x_WriteREG(0x04,0x0000); //��Ĵ�����ַ04H��д������69H����RST�Ĵ���	
//	ADS869x_ReadREG(0x04);	
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

void Timinit(void)
{
    //timer init
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    
    PowerControl_Init();
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 127;
    TIM_TimeBaseStructure.TIM_Prescaler = 80;
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
static uint16_t adc_packet_len=0;
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
    ADS869x_Init();
    FM25VXX_Init();
    ignore_num=0;
    Timinit();
    PowerControl_Init();
    //while(1){}
    //    ��ʼ�ȴ�ADC���ݲɼ����
    for(send_pkt=0;;)  //Ϊ�˲����������ѭ������
    {
        delay_ms(1);  //��ʱ1ms
        adc_wait_overflow++;
        if(adc_wait_overflow>=1000*15)  //> 15S break;
            break;
//      ��ͣ�Ļ�ȡadc��ֵ
        result=QueueOut(GetFifo_Piot(),&sdat);
        if(result==QueueOperateOk){    //���µ�����
 
            //NvRam_Write_Data(sdat,recv_pkt*ADC_SIZE_16*2,ADC_SIZE_16*2);
            FM25VXX_Write(sdat,send_pkt*adc_packet_len,adc_packet_len);   
            //FM25VXX_Read(_test_buffer,send_pkt*adc_packet_len,adc_packet_len);
                        
            //for(ii=0;ii<adc_packet_len;ii++)
            //{
            //    if(sdat[ii]!=_test_buffer[ii])
            //        printf("error data !\n");
            //}

            send_pkt++;
#ifdef ADC_DEBUG_MODE
        UART_DATA(sdat,ADC_SIZE_16*2);
#endif
           
            memfree(sdat);
            sdat=NULL; 
        }
        if(send_pkt>=adc_packet_len){
            break;
        }
    }
#ifdef ADC_DEBUG_MODE
    //while(1);
#endif
    ADS869x_DeInit();
    //FM25VXX_DisInit();
    DeintTim();
    PowerControl_DeInit();
    delay_ms(10);

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
    Timinit();
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
            for(i=0;i<(adc_packet_len>>1);i++)
            {
                if(adc_current<adc_p[i])
                    adc_current=adc_p[i];
                adc_sum+=adc_p[i];
            }
            adc_sum/=(adc_packet_len>>1);
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
    ADS869x_DeInit();
    DeintTim();
    delay_ms(10);

}


void ADS869x_Stop_Sample(void)  
{
    ADS869x_DeInit();
    DeintTim();

}

static uint16_t adc_data=0;
static uint8_t *buffer;
#define IGNORE_NUM 32

void TIM2_IRQHandler(void)
{
    uint16_t result;
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

        if(collection_cnt>=adc_packet_len){
                return;
        }
        adc_data = ADS869x_ReadADCData();
        
        if(ignore_num<IGNORE_NUM)
        {
            ignore_num++;
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
                
            }
            buffer_cnt=0;
        }

    }
}

/************************************************************/
