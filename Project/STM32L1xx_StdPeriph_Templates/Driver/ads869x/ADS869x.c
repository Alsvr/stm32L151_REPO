#include "ADS869x.h"
#include "GPIO.h"
#include "delay.h"
#include "math.h"
#include "stdio.h"
#include "fifoqueue.h"
#include "mempool.h"
#include "dataStore.h"
#include "FM25WXX.h"
//#define N  5  //中值滤波算法 数据个数

//定义全局变量
//unsigned char G_ACTIVE_FLAG=0x00;  //定义输入电压过高标志位

//unsigned long G_VAL=131076; 
//float G_RANGE=6;
//unsigned char G_F_Flag =0;

static uint16_t adc_buffer[512];
static uint16_t buffer_cnt=0;

#if 0
//滤波算法  中值滤波算法
/*************************
程序功能：滤波算法  中值滤波算法
参数：无
返回值：滤波 实际电压值的1000,000倍
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
程序功能：获取电压值
参数：无
返回值：实际电压值的1000,000倍
**************************/
unsigned long Get_Vol(void)
{
	unsigned long vol=0;
	static unsigned long Rvol=0;
	float temp=0;

	ADS869x_NAP_EXIT();    //退出休眠模式 进行采样
//	delay_us(20);
	vol = Get_ADC_Value(); //进行ADC采样	
	
	ADS869x_GO_NAP();      //采样完成进入休眠模式
	ADS869x_GO_PD();       //采样完成进入PD模式
	
	if(vol>=G_VAL)
	{
		vol = vol-G_VAL;	
		temp = vol*4.096/pow(2,18)*G_RANGE;  //计算实际的电压值 
		vol =	temp*1000000;     //实际电压值*1 v 000 mv 000 uv	  将实际电压值放大 1000 000倍
		G_F_Flag=0;  //正数标记
	}
	else
	{
		vol = G_VAL - 1 - vol;	
		temp = vol*4.096/pow(2,18)*G_RANGE;
		vol =	temp*1000000;	 
		G_F_Flag=1;  //负数标记
	}
	/*配置输入范围
   0000b 正负    3*4.096  0x0000		6
   0001b 正负  2.5*4.096  0x0001
   0010b 正负  1.5*4.096  0x0002		3
   0011b 正负 1.25*4.096  0x0003		
   0100b 正负0.626*4.096  0x0004	 	1.25 
*/	
	if ( temp<2 && G_RANGE!=1.25 ) 
	{
//			ADS869x_WriteREG(ADS869x_RANGE_SEL,0x0004);		//配置 输入范围为 0000b 正负2.5*4.096	
//			G_RANGE=1.25;	
//			OLED_ShowString(110,6,"2 ");	
//			G_VAL=131087; 
			return Rvol;		
	}
	else if( temp>=2 && temp<5 && G_RANGE!=3)
	{
//			ADS869x_WriteREG(ADS869x_RANGE_SEL,0x0002);		//配置 输入范围为 0000b 正负1.5*4.096	
//			G_RANGE=3;
//			OLED_ShowString(110,6,"5 ");
//			G_VAL=131078; 		
			return Rvol;		
	}	
	else if(temp >=5 && G_RANGE!=6 )
	{
//			ADS869x_WriteREG(ADS869x_RANGE_SEL,0x0000);		//配置 输入范围为 0000b 正负2.5*4.096	
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
程序功能：ADS869x_IO_Init
参数：无
返回值：无
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
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB,ENABLE); //GPIOA时钟 GPIOB时钟

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
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  //输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  //输入
    GPIO_Init(ADS869x_MISO_GPIO_PORT, &GPIO_InitStructure);	  //初始化GPIOD3,6


    GPIO_InitStructure.GPIO_Pin = ADS869x_DO1_GPIO|ADS869x_RVS_GPIO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;  //输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  //输入
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
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOA时钟 GPIOB时钟


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
程序功能：ADS869x读写8位数据
参数： 
			dat：    写入的值
返回值：无
**************************/
unsigned char ADS869x_ReadWrite(unsigned char dat)
{
	unsigned char i,temp=0;

	for(i=0;i<8;i++)
	{
		ADS869x_SCLK_Clr(); //拉低时钟
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
    ADS869x_SCLK_Clr(); //拉低时钟
	return temp;	
}
/*************************
程序功能：ADS869x_SendCMD  写命令
参数：cmd：    写入的命令 
			dat：    写入的地址
返回值：无
**************************/
void ADS869x_SendCMD(unsigned char cmd, uint8_t addr)
{
	ADS869x_CS_Clr();          //拉低片选，开始接受数据或者命令。
	ADS869x_ReadWrite(cmd+addr);    //发送命令。
	ADS869x_ReadWrite(addr);	 //发送地址
	ADS869x_ReadWrite(0x00);
	ADS869x_ReadWrite(0x00);
	ADS869x_CS_Set();	         //拉高片选，开始命令转换。
}

/*************************
程序功能：ADS869x_ReadREG读数据
参数：无
返回值：接收的32 位数据
**************************/
unsigned long ADS869x_ReadREG(unsigned char addr)  
{
	uint16_t sum=0;
    uint32_t time_out=0;
	ADS869x_CS_Clr();										 //拉低片选，接受命令
	while(ADS869x_RVS)
    {
        time_out++;
        if(time_out>320000)
            printf("time_out %d\n",time_out);
    }
	ADS869x_ReadWrite(((ADS869x_READ_HWORD<<8)+addr)>>8);    //发送写命令
	ADS869x_ReadWrite(addr);
	sum = ADS869x_ReadWrite(0)<<8;
	sum += ADS869x_ReadWrite(0);
	ADS869x_CS_Set();											//拉高片选，进入命令操作。	
	
	return sum;                          //返回数据
}
/************************************************************/
/*************************
程序功能：ADS869x_WriteREG 写寄存器
参数：regaddr：寄存器地址
      dat：    写入的值  16bit
返回值：无
**************************/
void ADS869x_WriteREG(unsigned char regaddr,unsigned short dat)
{
	uint8_t dat_H=0,dat_L=0;
    uint32_t time_out=0;
	dat_L = dat&0xFF;
	dat_H = (dat>>8)&0xFF;
    
	ADS869x_CS_Clr();										 //拉低片选，接受命令
	while(ADS869x_RVS)
    {
        time_out++;
        if(time_out>320000)
            printf("time_out %d\n",time_out);
    }
	ADS869x_ReadWrite(((ADS869x_WRITE<<8)+regaddr)>>8);    //发送写命令
	ADS869x_ReadWrite(regaddr);	
	ADS869x_ReadWrite(dat_H);
	ADS869x_ReadWrite(dat_L);
	ADS869x_CS_Set();											//拉高片选，进入命令操作。	
}
/*************************
程序功能：ADS869x_ReadData读数据
参数：无
返回值：接收的32 位数据
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
	for(i=0;i<4;i++)                      //接收数据32位
	{
		sum = sum << 8;
		r = ADS869x_ReadWrite(0x00);    //接收数据
		sum |= r;             
	}
	ADS869x_CS_Set();				
	return sum&0xffff;                          //返回数据
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
	for(i=0;i<4;i++)                      //接收数据32位
	{
		sum = sum << 8;
		r = ADS869x_ReadWrite(0x00);    //接收数据
		sum |= r;             
	}
	ADS869x_CS_Set();				
	return sum&0xffff;                          //返回数据
}


/*************************
程序功能：ADS869x_Over_Judgment FLAG判断
参数：无
返回值：无
**************************/
void ADS869x_Over_Judgment(void)  
{
	unsigned char i=0;
	unsigned long sum=0;
	unsigned long r=0;

	//IWDG_Feed();
//	while(!ADS869x_RVS);              //当DRDY为低是才能写指令 
	ADS869x_CS_Clr();
	delay_us(25);            						//延时时间
	for(i=0;i<4;i++)                      //接收数据32位
	{
		sum = sum << 8;
		r = ADS869x_ReadWrite(0x00);    //接收数据
		sum |= r;             
	}		
	ADS869x_CS_Set();
	
	//判断范围
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
程序功能：ADS869x_GO_NAP进入睡眠模式
参数：无
返回值：接收的32 位数据
**************************/
void ADS869x_GO_NAP(void)  
{
	ADS869x_WriteREG(0x04,0x6900); //向寄存器地址05H中写入数据69H解锁RST寄存器
	ADS869x_WriteREG(0x04,0x0002); //向寄存器地址04H中写入数据69H解锁RST寄存器	
//	ADS869x_ReadREG(0x04);	
    
}
/*************************
程序功能：ADS869x_NAP_EXIT退出睡眠模式
参数：无
返回值：接收的32 位数据
**************************/
void ADS869x_NAP_EXIT(void)  
{
	ADS869x_WriteREG(0x05,0x6900); //向寄存器地址05H中写入数据69H解锁RST寄存器
	ADS869x_WriteREG(0x04,0x0000); //向寄存器地址04H中写入数据69H解锁RST寄存器	
//	ADS869x_ReadREG(0x04);	
}
/*************************
程序功能：ADS869x_GO_PD进入PD模式
参数：无
返回值：接收的32 位数据
**************************/
void ADS869x_GO_PD(void)  
{
    uint32_t reg_data=0;
	ADS869x_WriteREG(0x04,0x6900); //向寄存器地址05H中写入数据69H解锁RST寄存器
	ADS869x_WriteREG(0x04,0x0001); //向寄存器地址04H中写入数据69H解锁RST寄存器	
    reg_data = ADS869x_ReadREG(0x04);
    printf("reg4 is %d\n",reg_data);
}
/*************************
程序功能：ADS869x_PD_EXIT退出PD模式
参数：无
返回值：接收的32 位数据
**************************/
void ADS869x_PD_EXIT(void)  
{
	ADS869x_WriteREG(0x14,0x0000); //向寄存器地址05H中写入数据69H解锁RST寄存器
	ADS869x_WriteREG(0x04,0x0000); //向寄存器地址04H中写入数据69H解锁RST寄存器	
//	ADS869x_ReadREG(0x04);	
}	

void ADS869x_SetRefMode(void)  
{
    uint32_t reg_data=0;
	ADS869x_WriteREG(0x14,0x000B); //向寄存器地址05H中写入数据69H解锁RST寄存器
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
		ADS869x_SCLK_Clr(); //拉低时钟
		//delay_us(20);
        ADS869x_MOSI_Clr();
		ADS869x_SCLK_Set();
		temp<<=1;
		if(ADS869x_MISO) 
            temp++;	
	}
    ADS869x_CS_Set();				
    return (temp>>16);//返回数据
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
    //    开始等待ADC数据采集完成
    for(send_pkt=0;;)  //为了不死在这个死循环里面
    {
        delay_ms(1); //延时1
        adc_wait_overflow++;
        if(adc_wait_overflow>=1000*15)  //> 15S break;
        {
            printf("adc time out:%d \n",adc_wait_overflow);
            break;
        }
//      不停的获取adc的值
        result=QueueOut(GetFifo_Piot(),&sdat);
        if(result==QueueOperateOk){    //有新的数据
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
    //开始等待ADC数据采集完成
    for(send_pkt=0;;)  //为了不死在这个死循环里面
    {
        delay_ms(1);  //延时1ms
        adc_wait_overflow++;
        if(adc_wait_overflow>=1000*15)  //> 15S break;
            break;
//      不停的获取adc的值
        result=QueueOut(GetFifo_Piot(),&sdat);
        if(result==QueueOperateOk){    //有新的数据
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

                    //错误
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
