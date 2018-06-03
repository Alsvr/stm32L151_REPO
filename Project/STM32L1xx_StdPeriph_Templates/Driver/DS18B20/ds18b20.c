#include "ds18b20.h"
#include "delay.h"	
//#include "stdio.h"
//#define DS18B20_DQ_IN   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)
//#define DS18B20_DQ_H    GPIO_SetBits(GPIOB,GPIO_Pin_12)
//#define DS18B20_DQ_L    GPIO_ResetBits(GPIOB,GPIO_Pin_12)

#define RCC_DQ		RCC_AHBPeriph_GPIOB
#define PORT_DQ		GPIOB
#define PIN_DQ		GPIO_Pin_12


#define DQ_0()		GPIO_ResetBits(PORT_DQ, PIN_DQ)
#define DQ_1()		GPIO_SetBits(PORT_DQ, PIN_DQ)

/* 判断DQ输入是否为低 */
#define DQ_IS_LOW()	(GPIO_ReadInputDataBit(PORT_DQ, PIN_DQ) == Bit_RESET)



void bsp_InitDS18B20(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    
    

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOA时钟
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);
    
	/* 打开GPIO时钟 */
    RCC_AHBPeriphClockCmd(RCC_DQ, ENABLE);	//GPIOA时钟
	//RCC_AHB1PeriphClockCmd(RCC_DQ, ENABLE);
  
	DQ_1();
    delay_ms(50);
	/* 配置DQ为开漏输出 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* 设为开漏模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = PIN_DQ;
	GPIO_Init(PORT_DQ, &GPIO_InitStructure);
}


void bsp_DeInitDS18B20(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOA时钟
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11|PIN_DQ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_11);
    
}


/*
*********************************************************************************************************
*	函 数 名: DS18B20_Reset
*	功能说明: 复位DS18B20。 拉低DQ为低，持续最少480us，然后等待
*	形    参: 无
*	返 回 值: 0 失败； 1 表示成功
*********************************************************************************************************
*/
uint8_t DS18B20_Reset(void)
{
	/*
		复位时序, 见DS18B20 page 15

		首先主机拉低DQ，持续最少 480us
		然后释放DQ，等待DQ被上拉电阻拉高，约 15-60us
		DS18B20 将驱动DQ为低 60-240us， 这个信号叫 presence pulse  (在位脉冲,表示DS18B20准备就绪 可以接受命令)
		如果主机检测到这个低应答信号，表示DS18B20复位成功
	*/

	uint8_t i;
	uint16_t k;

	//DISABLE_INT();/* 禁止全局中断 */

	/* 复位，如果失败则返回0 */
	for (i = 0; i < 1; i++)
	{
		DQ_0();				/* 拉低DQ */
		delay_us(520);	/* 延迟 520uS， 要求这个延迟大于 480us */
		DQ_1();				/* 释放DQ */

		delay_us(15);	/* 等待15us */

		/* 检测DQ电平是否为低 */
		for (k = 0; k < 10; k++)
		{
			if (DQ_IS_LOW())
			{
				break;
			}
			delay_us(10);	/* 等待65us */
		}
		if (k >= 10)
		{
			continue;		/* 失败 */
		}

		/* 等待DS18B20释放DQ */
		for (k = 0; k < 30; k++)
		{
			if (!DQ_IS_LOW())
			{
				break;
			}
			delay_us(10);	/* 等待65us */
		}
		if (k >= 30)
		{
			continue;		/* 失败 */
		}

		break;
	}

	//ENABLE_INT();	/* 使能全局中断 */

	delay_us(5);

	if (i >= 1)
	{
		return 0;
	}

	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_WriteByte
*	功能说明: 向DS18B20写入1字节数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DS18B20_WriteByte(uint8_t _val)
{
	/*
		写数据时序, 见DS18B20 page 16
	*/
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		DQ_0();
		delay_us(5);

		if (_val & 0x01)
		{
			DQ_1();
		}
		else
		{
			DQ_0();
		}
		delay_us(60);
		DQ_1();
		delay_us(2);
		_val >>= 1;
	}
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_ReadByte
*	功能说明: 向DS18B20读取1字节数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t DS18B20_ReadByte(void)
{
	/*
		写数据时序, 见DS18B20 page 16
	*/
	uint8_t i;
	uint8_t read = 0;
    
	for (i = 0; i < 8; i++)
	{
		read >>= 1;
		DQ_0();
		delay_us(1);
		DQ_1();
		delay_us(1);

		if (DQ_IS_LOW())
		{
			;
		}
		else
		{
			read |= 0x80;
		}
		delay_us(60);
	}

	return read;
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_ReadTempReg
*	功能说明: 读温度寄存器的值（原始数据）
*	形    参: 无
*	返 回 值: 温度寄存器数据 （除以16得到 1摄氏度单位, 也就是小数点前面的数字)
*********************************************************************************************************
*/
int16_t DS18B20_ReadTempReg(void)
{
	uint8_t temp1, temp2;
    uint8_t TL , TH , config;
	/* 总线复位 */
	if (DS18B20_Reset() == 0)
	{
		return 0;
	}		
    printf("PASS!\n");
  
    DS18B20_WriteByte(0xcc);	/* 发命令 */
	DS18B20_WriteByte(0x44);	/* 发转换命令 */
    
    delay_ms(700);
    
    if (DS18B20_Reset() == 0)
	{
		return 0;
	}	
    DS18B20_WriteByte(0xcc);	/* 发命令 */
	DS18B20_WriteByte(0xbe);
	temp1 = DS18B20_ReadByte();	/* 读温度值低字节 */
	temp2 = DS18B20_ReadByte();	/* 读温度值高字节 */
    
	TL = DS18B20_ReadByte();	/* 读温度值低字节 */
	TH = DS18B20_ReadByte();	/* 读温度值高字节 */
    

	config = DS18B20_ReadByte();	/* 读温度值高字节 */
    
    printf("TL is %X,TH is %X config is %X!\n",TL,TH,config);
    DS18B20_Reset();
	return ((temp2 << 8) | temp1);	/* 返回16位寄存器值 */
}



int16_t DS18B20_ReadTempStep1(void)
{
    
    uint8_t temp1, temp2;
    uint8_t TL , TH , config;
    bsp_InitDS18B20();
    /* 总线复位 */
    if (DS18B20_Reset() == 0)
    {
        return 0;
    }
    DS18B20_WriteByte(0xcc);	/* 发命令 */
    DS18B20_WriteByte(0x44);	/* 发转换命令 */
    return 1;
}




int16_t DS18B20_ReadTempStep2(void)
{
	uint8_t temp1, temp2;
    uint8_t TL , TH , config;
	/* 总线复位 */
    
    if (DS18B20_Reset() == 0)
	{
		return 0;
	}	
    DS18B20_WriteByte(0xcc);	/* 发命令 */
	DS18B20_WriteByte(0xbe);
	temp1 = DS18B20_ReadByte();	/* 读温度值低字节 */
	temp2 = DS18B20_ReadByte();	/* 读温度值高字节 */
    
	TL = DS18B20_ReadByte();	/* 读温度值低字节 */
	TH = DS18B20_ReadByte();	/* 读温度值高字节 */
    

	config = DS18B20_ReadByte();	/* 读温度值高字节 */
    
    //printf("TL is %X,TH is %X config is %X!\n",TL,TH,config);
    DS18B20_Reset();
	return ((temp2 << 8) | temp1);	/* 返回16位寄存器值 */
}


/*
*********************************************************************************************************
*	函 数 名: DS18B20_ReadID
*	功能说明: 读DS18B20的ROM ID， 总线上必须只有1个芯片
*	形    参: _id 存储ID
*	返 回 值: 0 表示失败， 1表示检测到正确ID
*********************************************************************************************************
*/
uint8_t DS18B20_ReadID(uint8_t *_id)
{
	uint8_t i;

	/* 总线复位 */
	if (DS18B20_Reset() == 0)
	{
		return 0;
	}

	DS18B20_WriteByte(0x33);	/* 发命令 */
	for (i = 0; i < 8; i++)
	{
		_id[i] = DS18B20_ReadByte();
	}

	DS18B20_Reset();		/* 总线复位 */
	
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_ReadTempByID
*	功能说明: 读指定ID的温度寄存器的值（原始数据）
*	形    参: 无
*	返 回 值: 温度寄存器数据 （除以16得到 1摄氏度单位, 也就是小数点前面的数字)
*********************************************************************************************************
*/
int16_t DS18B20_ReadTempByID(uint8_t *_id)
{
	uint8_t temp1, temp2;
	uint8_t i;

	DS18B20_Reset();		/* 总线复位 */

	DS18B20_WriteByte(0x55);	/* 发命令 */
	for (i = 0; i < 8; i++)
	{
		DS18B20_WriteByte(_id[i]);
	}
	DS18B20_WriteByte(0x44);	/* 发转换命令 */

	DS18B20_Reset();		/* 总线复位 */

	DS18B20_WriteByte(0x55);	/* 发命令 */
	for (i = 0; i < 8; i++)
	{
		DS18B20_WriteByte(_id[i]);
	}	
	DS18B20_WriteByte(0xbe);

	temp1 = DS18B20_ReadByte();	/* 读温度值低字节 */
	temp2 = DS18B20_ReadByte();	/* 读温度值高字节 */

	return ((temp2 << 8) | temp1);	/* 返回16位寄存器值 */
}





/*
void Init_18B20_OUT(void) //PB12 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOA时钟
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Init_18B20_IN(void) //PB12 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOA时钟
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//复位DS18B20
void DS18B20_Rst(void)	   
{                 
	Init_18B20_OUT(); //SET PA0 OUTPUT
    DS18B20_DQ_L; //拉低DQ
    delay_us(750);    //拉低750us
    DS18B20_DQ_H; //DQ=1 
	delay_us(15);     //15US
}
//等待DS18B20的回应
//返回1:未检测到DS18B20的存在
//返回0:存在
uint8_t DS18B20_Check(void) 	   
{   
	uint8_t retry=0;
	Init_18B20_IN();//SET PA0 INPUT	 
    while (DS18B20_DQ_IN&&retry<200)
	{
		retry++;
		delay_us(1);
	};	 
	if(retry>=200)return 1;
	else retry=0;
    while (!DS18B20_DQ_IN&&retry<240)
	{
		retry++;
		delay_us(1);
	};
	if(retry>=240)return 1;	    
	return 0;
}
//从DS18B20读取一个位
//返回值：1/0
uint8_t DS18B20_Read_Bit(void) 			 // read one bit
{
    uint8_t data;
	Init_18B20_OUT();//SET PA0 OUTPUT
    DS18B20_DQ_L; 
	delay_us(2);
    DS18B20_DQ_H; 
	Init_18B20_IN();//SET PA0 INPUT
	delay_us(12);
	if(DS18B20_DQ_IN)
        data=1;
    else 
        data=0;  
    delay_us(50);           
    return data;
}
//从DS18B20读取一个字节
//返回值：读到的数据
uint8_t DS18B20_Read_Byte(void)    // read one byte
{    
    uint8_t i,j,dat;
    Init_18B20_IN();
    dat=0;
    for (i=1;i<=8;i++) 
    {
        j=DS18B20_Read_Bit();
        dat=(j<<8)|(dat>>1);
    }   
    return dat;
}
//写一个字节到DS18B20
//dat：要写入的字节
void DS18B20_Write_Byte(uint8_t dat)     
 {             
    uint8_t j;
    uint8_t testb;
	Init_18B20_OUT();//SET PA0 OUTPUT;
    for (j=1;j<=8;j++) 
	{
        testb=dat&0x01;
        dat=dat>>1;
        if (testb) 
        {
            DS18B20_DQ_L;// Write 1
            delay_us(2);                            
            DS18B20_DQ_H;
            delay_us(60);             
        }
        else 
        {
            DS18B20_DQ_L;// Write 0
            delay_us(60);             
            DS18B20_DQ_H;
            delay_us(2);                          
        }
    }
}
//开始温度转换
void DS18B20_Start(void)// ds1820 start convert
{   						               
    DS18B20_Rst();	   
	DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
} 
//初始化DS18B20的IO口 DQ 同时检测DS的存在
//返回1:不存在
//返回0:存在    	 
uint8_t DS18B20_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOA时钟
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);
    delay_ms(20);
    Init_18B20_OUT(); //SET  OUTPUT
    //DS18B20_Rst();
    return 0;
}  

void DS18B20_DeInit(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOA时钟
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
    
}

//从ds18b20得到温度值
//精度：0.1C
//返回值：温度值 （-550~1250） 
short DS18B20_Get_Temp(void)
{
    uint8_t temp;
    uint8_t TL,TH;
	short tem;
    DS18B20_Start();                    // ds1820 start convert
    DS18B20_Rst();
    DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0xbe);// convert	
    
          
    TL=DS18B20_Read_Byte(); // LSB   
    TH=DS18B20_Read_Byte(); // MSB  
	    	  
    if(TH>7)
    {
        TH=~TH;
        TL=~TL; 
        temp=0;//温度为负  
    }else temp=1;//温度为正	  	  
    tem=TH; //获得高八位
    tem<<=8;    
    tem+=TL;//获得底八位
    tem=(float)tem*0.625;//转换     
	if(temp)return tem; //返回温度值
	else return -tem;    
} 

*/
 
