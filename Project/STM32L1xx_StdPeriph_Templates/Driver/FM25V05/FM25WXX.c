#include "FM25WXX.h" 
#include "stdio.h"  
#include "GPIO.h"

//u16 W25QXX_TYPE;	//默认是W25Q128

#define FM25VXX_SCLK_H   P0_1=1   //P01
#define FM25VXX_SCLK_L   P0_1=0

#define FM25VXX_DIN_H    P0_5=1   //P05
#define FM25VXX_DIN_L    P0_5=0

#define FM25VXX_DOUT     P0_0     //P14
#define	FM25VXX_CS 		 P0_6     //W25QXX的片选信号 P06

#define	WREN        0x06        // Set Write Enable Latch
#define	WRDI        0x04        // Write Disable
#define	RDSR        0x05        // Read Status Register
#define	WRSR        0x01        // Write Status Register
#define	RDMD        0x03        // Read Memory Data
#define	WRMD        0x02        // Write Memory Data
#define SLEEP       0xb9        //Sleep

void FM25VXX_Write_Disable(void);
void FM25VXX_Write_SR(uint8_t sr);
uint8_t FM25VXX_ReadSR(void);
void FM25VXX_Write_Enable(void);


#define GPIO_FM25V05_CS  GPIO_Pin_13
#define PORT_FM25V05_CS  GPIOC	
#define FM25V05_MAX_SIZE 65535


#define sEE_SPI                          SPI1
#define sEE_SPI_CLK                      RCC_APB2Periph_SPI1
   
#define sEE_SPI_SCK_PIN                  GPIO_Pin_5                  /* PA.05 */
#define sEE_SPI_SCK_GPIO_PORT            GPIOA                       /* GPIOA */
#define sEE_SPI_SCK_GPIO_CLK             RCC_AHBPeriph_GPIOA
#define sEE_SPI_SCK_SOURCE               GPIO_PinSource5
#define sEE_SPI_SCK_AF                   GPIO_AF_SPI1

#define sEE_SPI_MISO_PIN                 GPIO_Pin_6                  /* PA.06 */
#define sEE_SPI_MISO_GPIO_PORT           GPIOA                       /* GPIOA */
#define sEE_SPI_MISO_GPIO_CLK            RCC_AHBPeriph_GPIOA
#define sEE_SPI_MISO_SOURCE              GPIO_PinSource6
#define sEE_SPI_MISO_AF                  GPIO_AF_SPI1

#define sEE_SPI_MOSI_PIN                 GPIO_Pin_7                 /* PA.07 */
#define sEE_SPI_MOSI_GPIO_PORT           GPIOA                       /* GPIOA */
#define sEE_SPI_MOSI_GPIO_CLK            RCC_AHBPeriph_GPIOA
#define sEE_SPI_MOSI_SOURCE              GPIO_PinSource7
#define sEE_SPI_MOSI_AF                  GPIO_AF_SPI1

#define sEE_SPI_CS_PIN                   GPIO_Pin_13                  /* PC.13*/
#define sEE_SPI_CS_GPIO_PORT             GPIOC                       /* GPIOC */
#define sEE_SPI_CS_GPIO_CLK              RCC_AHBPeriph_GPIOC   


#define sEE_CS_HIGH()      GPIO_SetBits(sEE_SPI_CS_GPIO_PORT, sEE_SPI_CS_PIN) 
#define sEE_CS_LOW()       GPIO_ResetBits(sEE_SPI_CS_GPIO_PORT, sEE_SPI_CS_PIN)


//初始化SPI FLASH的IO口
uint16_t FM25VXX_Init(void)
{ 
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;

	/*!< sEE_CS_GPIO, sEE_MOSI_GPIO, sEE_MISO_GPIO and sEE_SCK_GPIO 
		 Periph clock enable */
	RCC_AHBPeriphClockCmd(sEE_SPI_CS_GPIO_CLK | sEE_SPI_MOSI_GPIO_CLK | sEE_SPI_MISO_GPIO_CLK |
						  sEE_SPI_SCK_GPIO_CLK , ENABLE);
	
	/*!< sEE Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); 

	GPIO_InitStructure.GPIO_Pin = sEE_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(sEE_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure sEE_SPI pins: MISO */
	GPIO_InitStructure.GPIO_Pin = sEE_SPI_MISO_PIN;
	GPIO_Init(sEE_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure sEE_SPI pins: MOSI */
	GPIO_InitStructure.GPIO_Pin = sEE_SPI_MOSI_PIN;
	GPIO_Init(sEE_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);	



	/*!< Configure sEE_SPI_CS_PIN pin: sEE_SPI Card CS pin */
	GPIO_InitStructure.GPIO_Pin = sEE_SPI_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(sEE_SPI_CS_GPIO_PORT, &GPIO_InitStructure);


	/* Connect PXx to sEE_SPI_SCK */
	GPIO_PinAFConfig(sEE_SPI_SCK_GPIO_PORT, sEE_SPI_SCK_SOURCE, sEE_SPI_SCK_AF);

	/* Connect PXx to sEE_SPI_MISO */
	GPIO_PinAFConfig(sEE_SPI_MISO_GPIO_PORT, sEE_SPI_MISO_SOURCE, sEE_SPI_MISO_AF); 

	/* Connect PXx to sEE_SPI_MOSI */
	GPIO_PinAFConfig(sEE_SPI_MOSI_GPIO_PORT, sEE_SPI_MOSI_SOURCE, sEE_SPI_MOSI_AF);  

	/*!< sEE SPI Config */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(sEE_SPI, &SPI_InitStructure);

	SPI_Cmd(sEE_SPI, ENABLE); /*!< sEE_SPI enable */
    sEE_CS_HIGH();
}  


void FM25VXX_DisInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  SPI_Cmd(sEE_SPI, DISABLE); /*!< sEE_SPI disable */
  SPI_I2S_DeInit(sEE_SPI);   /*!< DeInitializes the sEE_SPI */
  
  /*!< sEE_SPI Periph clock disable */
  RCC_APB1PeriphClockCmd(sEE_SPI_CLK, DISABLE); 

  /*!< Configure sEE_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = sEE_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(sEE_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sEE pins: MISO */
  GPIO_InitStructure.GPIO_Pin = sEE_SPI_MISO_PIN;
  GPIO_Init(sEE_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sEE pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = sEE_SPI_MOSI_PIN;
  GPIO_Init(sEE_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sEE_CS_PIN pin: sEE Card CS pin */
  GPIO_InitStructure.GPIO_Pin = sEE_SPI_CS_PIN;
  GPIO_Init(sEE_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

}  


uint8_t sEE_SendByte(uint8_t byte)
{
  /*!< Loop while DR register in not empty */
  while (SPI_I2S_GetFlagStatus(sEE_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI peripheral */
  SPI_SendData(sEE_SPI, byte);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(sEE_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  return (uint8_t)SPI_ReceiveData(sEE_SPI);
}


//读取FM25VXX的状态寄存器
uint8_t FM25VXX_ReadSR(void)   
{  
	uint8_t byte=0;   
	sEE_CS_LOW();                            //使能器件   
    sEE_SendByte(RDSR);
    byte=sEE_SendByte(0);  
	sEE_CS_HIGH();                            //取消片选     
	return byte;   
} 
//写状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void FM25VXX_Write_SR(uint8_t sr)   
{   
	sEE_CS_LOW();                             //使能器件   
    sEE_SendByte(WRSR);    //发送写取状态寄存器命令    
    sEE_SendByte(sr); //写入一个字节 
	sEE_CS_HIGH();                            //取消片选     	      
} 


//写使能	
//将WEL置位   
void FM25VXX_Write_Enable(void)   
{
	sEE_CS_LOW();                            //使能器件   
    sEE_SendByte(WREN);                     //发送写指令  
	sEE_CS_HIGH();                            //取消片选     	      
} 


//写禁止	
//将WEL清零  
void FM25VXX_Write_Disable(void)   
{  
	sEE_CS_LOW();                             //使能器件   
    sEE_SendByte(WRDI);                      //发送写禁止指令    
	sEE_CS_HIGH();                          //取消片选     	      
} 		

//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址
//NumByteToRead:要读取的字节数
void FM25VXX_Read(uint8_t* pBuffer,uint16_t ReadAddr,uint16_t NumByteToRead)   
{ 
 	uint16_t i;   										    
	sEE_CS_LOW();                           //使能器件   
    sEE_SendByte(RDMD);                     //发送读取命令   
    sEE_SendByte((uint8_t)((ReadAddr)>>8));      //发射地址16位
    sEE_SendByte((uint8_t)ReadAddr);
    
    for(i=0;i<NumByteToRead;i++)
    { 
      pBuffer[i]=sEE_SendByte(0);   //循环读数  
    }
	sEE_CS_HIGH(); 				    	      
}  
//在指定地址开始写入数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址
//NumByteToWrite:要写入的字节数


void FM25VXX_Write(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
    /*!< Enable the write access to the EEPROM */
  FM25VXX_Write_Enable();
  
  /*!< Select the EEPROM: Chip Select low */
  sEE_CS_LOW();

  sEE_SendByte(WRMD);
  /*!< Send "Write to Memory" instruction and MSB of WriteAddr  */
  sEE_SendByte((uint8_t)((WriteAddr)>>8));
  
  /*!< Send WriteAddr address byte to write to */
  sEE_SendByte((uint8_t)WriteAddr & 0xFF);
  
  /*!< while there is data to be written on the EEPROM */
  while (NumByteToWrite--)
  {
    /*!< Send the current byte */
    sEE_SendByte(*pBuffer);
    /*!< Point on the next byte to be written */
    pBuffer++;
  }
  
  /*!< Deselect the EEPROM: Chip Select high */
  sEE_CS_HIGH();

  FM25VXX_Write_Disable();
}



void test_FM25V05(void)
{
    uint32_t data=0,i=0,old_data=0;
    data=100;
	Led_Open();
    for(i=0;i<(65536/4);i++){
        FM25VXX_Write((uint8_t *)&data,i*sizeof(data),sizeof(data));
        data++;
    }
	Led_Close();
    data=0x55555555;
    //for(i=0;i<(65536/4);i++){
    //    FM25VXX_Write((uint8 *)&data,i*sizeof(data),sizeof(data));
    //    //data++;
    // }
    for(i=0;i<(65536/4);i++)
    {
		
        FM25VXX_Read((uint8_t *)&data,i*sizeof(data),sizeof(data));
		//Led_Close();
        if((data-1)!=old_data)
            printf("fm25v05 wrong\n");
		//else
			//printf("%d\n",data);
        old_data=data;
    }
            printf("fm25v05 check finish\n");
    
    
}






















