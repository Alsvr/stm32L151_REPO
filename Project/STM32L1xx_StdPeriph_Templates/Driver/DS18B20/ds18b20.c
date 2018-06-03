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

/* �ж�DQ�����Ƿ�Ϊ�� */
#define DQ_IS_LOW()	(GPIO_ReadInputDataBit(PORT_DQ, PIN_DQ) == Bit_RESET)



void bsp_InitDS18B20(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    
    

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOAʱ��
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);
    
	/* ��GPIOʱ�� */
    RCC_AHBPeriphClockCmd(RCC_DQ, ENABLE);	//GPIOAʱ��
	//RCC_AHB1PeriphClockCmd(RCC_DQ, ENABLE);
  
	DQ_1();
    delay_ms(50);
	/* ����DQΪ��©��� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* ��Ϊ��©ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = PIN_DQ;
	GPIO_Init(PORT_DQ, &GPIO_InitStructure);
}


void bsp_DeInitDS18B20(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOAʱ��
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
*	�� �� ��: DS18B20_Reset
*	����˵��: ��λDS18B20�� ����DQΪ�ͣ���������480us��Ȼ��ȴ�
*	��    ��: ��
*	�� �� ֵ: 0 ʧ�ܣ� 1 ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t DS18B20_Reset(void)
{
	/*
		��λʱ��, ��DS18B20 page 15

		������������DQ���������� 480us
		Ȼ���ͷ�DQ���ȴ�DQ�������������ߣ�Լ 15-60us
		DS18B20 ������DQΪ�� 60-240us�� ����źŽ� presence pulse  (��λ����,��ʾDS18B20׼������ ���Խ�������)
		���������⵽�����Ӧ���źţ���ʾDS18B20��λ�ɹ�
	*/

	uint8_t i;
	uint16_t k;

	//DISABLE_INT();/* ��ֹȫ���ж� */

	/* ��λ�����ʧ���򷵻�0 */
	for (i = 0; i < 1; i++)
	{
		DQ_0();				/* ����DQ */
		delay_us(520);	/* �ӳ� 520uS�� Ҫ������ӳٴ��� 480us */
		DQ_1();				/* �ͷ�DQ */

		delay_us(15);	/* �ȴ�15us */

		/* ���DQ��ƽ�Ƿ�Ϊ�� */
		for (k = 0; k < 10; k++)
		{
			if (DQ_IS_LOW())
			{
				break;
			}
			delay_us(10);	/* �ȴ�65us */
		}
		if (k >= 10)
		{
			continue;		/* ʧ�� */
		}

		/* �ȴ�DS18B20�ͷ�DQ */
		for (k = 0; k < 30; k++)
		{
			if (!DQ_IS_LOW())
			{
				break;
			}
			delay_us(10);	/* �ȴ�65us */
		}
		if (k >= 30)
		{
			continue;		/* ʧ�� */
		}

		break;
	}

	//ENABLE_INT();	/* ʹ��ȫ���ж� */

	delay_us(5);

	if (i >= 1)
	{
		return 0;
	}

	return 1;
}

/*
*********************************************************************************************************
*	�� �� ��: DS18B20_WriteByte
*	����˵��: ��DS18B20д��1�ֽ�����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DS18B20_WriteByte(uint8_t _val)
{
	/*
		д����ʱ��, ��DS18B20 page 16
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
*	�� �� ��: DS18B20_ReadByte
*	����˵��: ��DS18B20��ȡ1�ֽ�����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint8_t DS18B20_ReadByte(void)
{
	/*
		д����ʱ��, ��DS18B20 page 16
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
*	�� �� ��: DS18B20_ReadTempReg
*	����˵��: ���¶ȼĴ�����ֵ��ԭʼ���ݣ�
*	��    ��: ��
*	�� �� ֵ: �¶ȼĴ������� ������16�õ� 1���϶ȵ�λ, Ҳ����С����ǰ�������)
*********************************************************************************************************
*/
int16_t DS18B20_ReadTempReg(void)
{
	uint8_t temp1, temp2;
    uint8_t TL , TH , config;
	/* ���߸�λ */
	if (DS18B20_Reset() == 0)
	{
		return 0;
	}		
    printf("PASS!\n");
  
    DS18B20_WriteByte(0xcc);	/* ������ */
	DS18B20_WriteByte(0x44);	/* ��ת������ */
    
    delay_ms(700);
    
    if (DS18B20_Reset() == 0)
	{
		return 0;
	}	
    DS18B20_WriteByte(0xcc);	/* ������ */
	DS18B20_WriteByte(0xbe);
	temp1 = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
	temp2 = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
    
	TL = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
	TH = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
    

	config = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
    
    printf("TL is %X,TH is %X config is %X!\n",TL,TH,config);
    DS18B20_Reset();
	return ((temp2 << 8) | temp1);	/* ����16λ�Ĵ���ֵ */
}



int16_t DS18B20_ReadTempStep1(void)
{
    
    uint8_t temp1, temp2;
    uint8_t TL , TH , config;
    bsp_InitDS18B20();
    /* ���߸�λ */
    if (DS18B20_Reset() == 0)
    {
        return 0;
    }
    DS18B20_WriteByte(0xcc);	/* ������ */
    DS18B20_WriteByte(0x44);	/* ��ת������ */
    return 1;
}




int16_t DS18B20_ReadTempStep2(void)
{
	uint8_t temp1, temp2;
    uint8_t TL , TH , config;
	/* ���߸�λ */
    
    if (DS18B20_Reset() == 0)
	{
		return 0;
	}	
    DS18B20_WriteByte(0xcc);	/* ������ */
	DS18B20_WriteByte(0xbe);
	temp1 = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
	temp2 = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
    
	TL = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
	TH = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
    

	config = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
    
    //printf("TL is %X,TH is %X config is %X!\n",TL,TH,config);
    DS18B20_Reset();
	return ((temp2 << 8) | temp1);	/* ����16λ�Ĵ���ֵ */
}


/*
*********************************************************************************************************
*	�� �� ��: DS18B20_ReadID
*	����˵��: ��DS18B20��ROM ID�� �����ϱ���ֻ��1��оƬ
*	��    ��: _id �洢ID
*	�� �� ֵ: 0 ��ʾʧ�ܣ� 1��ʾ��⵽��ȷID
*********************************************************************************************************
*/
uint8_t DS18B20_ReadID(uint8_t *_id)
{
	uint8_t i;

	/* ���߸�λ */
	if (DS18B20_Reset() == 0)
	{
		return 0;
	}

	DS18B20_WriteByte(0x33);	/* ������ */
	for (i = 0; i < 8; i++)
	{
		_id[i] = DS18B20_ReadByte();
	}

	DS18B20_Reset();		/* ���߸�λ */
	
	return 1;
}

/*
*********************************************************************************************************
*	�� �� ��: DS18B20_ReadTempByID
*	����˵��: ��ָ��ID���¶ȼĴ�����ֵ��ԭʼ���ݣ�
*	��    ��: ��
*	�� �� ֵ: �¶ȼĴ������� ������16�õ� 1���϶ȵ�λ, Ҳ����С����ǰ�������)
*********************************************************************************************************
*/
int16_t DS18B20_ReadTempByID(uint8_t *_id)
{
	uint8_t temp1, temp2;
	uint8_t i;

	DS18B20_Reset();		/* ���߸�λ */

	DS18B20_WriteByte(0x55);	/* ������ */
	for (i = 0; i < 8; i++)
	{
		DS18B20_WriteByte(_id[i]);
	}
	DS18B20_WriteByte(0x44);	/* ��ת������ */

	DS18B20_Reset();		/* ���߸�λ */

	DS18B20_WriteByte(0x55);	/* ������ */
	for (i = 0; i < 8; i++)
	{
		DS18B20_WriteByte(_id[i]);
	}	
	DS18B20_WriteByte(0xbe);

	temp1 = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */
	temp2 = DS18B20_ReadByte();	/* ���¶�ֵ���ֽ� */

	return ((temp2 << 8) | temp1);	/* ����16λ�Ĵ���ֵ */
}





/*
void Init_18B20_OUT(void) //PB12 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOAʱ��
    
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

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOAʱ��
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//��λDS18B20
void DS18B20_Rst(void)	   
{                 
	Init_18B20_OUT(); //SET PA0 OUTPUT
    DS18B20_DQ_L; //����DQ
    delay_us(750);    //����750us
    DS18B20_DQ_H; //DQ=1 
	delay_us(15);     //15US
}
//�ȴ�DS18B20�Ļ�Ӧ
//����1:δ��⵽DS18B20�Ĵ���
//����0:����
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
//��DS18B20��ȡһ��λ
//����ֵ��1/0
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
//��DS18B20��ȡһ���ֽ�
//����ֵ������������
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
//дһ���ֽڵ�DS18B20
//dat��Ҫд����ֽ�
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
//��ʼ�¶�ת��
void DS18B20_Start(void)// ds1820 start convert
{   						               
    DS18B20_Rst();	   
	DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
} 
//��ʼ��DS18B20��IO�� DQ ͬʱ���DS�Ĵ���
//����1:������
//����0:����    	 
uint8_t DS18B20_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOAʱ��
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
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	//GPIOAʱ��
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
    
}

//��ds18b20�õ��¶�ֵ
//���ȣ�0.1C
//����ֵ���¶�ֵ ��-550~1250�� 
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
        temp=0;//�¶�Ϊ��  
    }else temp=1;//�¶�Ϊ��	  	  
    tem=TH; //��ø߰�λ
    tem<<=8;    
    tem+=TL;//��õװ�λ
    tem=(float)tem*0.625;//ת��     
	if(temp)return tem; //�����¶�ֵ
	else return -tem;    
} 

*/
 
