#include "dataStore.h"


#define DATA_EEPROM_START_ADDR     0x08080000
#define DATA_EEPROM_END_ADDR       0x0808007F  //128 words
#define DATA_EEPROM_PAGE_SIZE      0x1
#define MAX_SUM_CRC_ERROR 20


uint8_t SetGlobalData(void);
void OutPutGloablPara(GlobalData_Para *CC2530_Global_Para);


static GlobalData_Para CC2530_Global_Para;
__IO  FLASH_Status FLASHStatus = FLASH_COMPLETE;


uint8_t Init_GlobalData()
{

    CC2530_Global_Para.terminal_addr[0]=192;
    CC2530_Global_Para.terminal_addr[1]=168;
    CC2530_Global_Para.terminal_addr[2]=1;
    CC2530_Global_Para.terminal_addr[3]=100;
    
    CC2530_Global_Para.route_addr[0]=192;
    CC2530_Global_Para.route_addr[1]=168;
    CC2530_Global_Para.route_addr[2]=1;
    CC2530_Global_Para.route_addr[3]=1;
    
    CC2530_Global_Para.ADC_OFFSET = ADC_OFFSET_DEFAULT;
    CC2530_Global_Para.ADC_LEN = ADC_LEN_DEFAULT;
    CC2530_Global_Para.ADC_Speed = ADC_SPEED_DEFAULT;
    CC2530_Global_Para.boot_flag=BOOT_ALREADY;
    CC2530_Global_Para.CRC_=0;
    return 1;
}


void Write_DATA_EEPROM(uint8_t* pBuffer,uint32_t len)
{
    uint32_t Address = 0 ,NbrOfPage = 0 ,j = 0 ;
    /* Unlock the FLASH PECR register and Data EEPROM memory */
    DATA_EEPROM_Unlock();

    /* Clear all pending flags */      
    FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR
                  | FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR | FLASH_FLAG_OPTVERRUSR);	

    /*  Data EEPROM Fast Word program of FAST_DATA_32 at addresses defined by 
      DATA_EEPROM_START_ADDR and DATA_EEPROM_END_ADDR */  
    Address = DATA_EEPROM_START_ADDR;

    NbrOfPage = ((DATA_EEPROM_END_ADDR - Address) + 1 ) >> 2; 

    /* Erase the Data EEPROM Memory pages by Word (32-bit) */
    for(j = 0; j < NbrOfPage; j++)
    {
        FLASHStatus = DATA_EEPROM_EraseWord(Address + (4 * j));
    }

    /* Program the Data EEPROM Memory pages by Word (32-bit) */
    while(Address < (DATA_EEPROM_START_ADDR + len))
    {
        FLASHStatus = DATA_EEPROM_FastProgramWord(Address, *((uint32_t * )pBuffer));

        if(FLASHStatus == FLASH_COMPLETE)
        {
            Address = Address + 4;
            pBuffer = pBuffer + 4;
        }
        else
        {
            FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR
                           | FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR);
        }
    }
    DATA_EEPROM_Lock();
}

void Read_DATA_EEPROM(uint8_t* pBuffer,uint32_t len)
{
    uint32_t Address = 0;
    Address = DATA_EEPROM_START_ADDR;

    /* Check the correctness of written data */
    while(Address < (len+DATA_EEPROM_START_ADDR))
    {
        *(uint32_t *)pBuffer = *(__IO uint32_t*)Address;
        Address = Address + 4;
        pBuffer = pBuffer + 4;
    }
}
GlobalData_Para* GetGlobalData(void)
{
     uint16_t i=0,error_cnt=0;
     uint8_t  crc_sum=0; 
     uint8_t *p=(uint8_t *)&CC2530_Global_Para;
     while(error_cnt++<10)
     {
        Read_DATA_EEPROM((uint8_t*)(&CC2530_Global_Para),sizeof(CC2530_Global_Para));
        for(i=0;i<(sizeof(CC2530_Global_Para)-1);i++)
        {
            crc_sum+=p[i];
        }
        if(crc_sum==CC2530_Global_Para.CRC_)    //符合累加校验
            break;
     }
     if(CC2530_Global_Para.boot_flag!=BOOT_ALREADY||
       (error_cnt>=MAX_SUM_CRC_ERROR))
     {
         printf("NVRAM READ FAIL\n");
         Init_GlobalData();
         SetGlobalData();
     }
     OutPutGloablPara(&CC2530_Global_Para);
     return &CC2530_Global_Para; 
}

void SetADCLen(uint8_t Len)   //传入的是256个16位数据
{
    if(Len>4)
        Len=4;
    CC2530_Global_Para.ADC_LEN=Len;
    SetGlobalData();
}

uint16_t Get_ADC_LEN(void)
{
    return CC2530_Global_Para.ADC_LEN;
    
}


uint8_t SetGlobalData(void)
{
    uint16_t i=0;
    uint8_t *p=(uint8_t *)&CC2530_Global_Para;
    CC2530_Global_Para.CRC_=0;
    // 更新CRC累加和
    for(i=0;i<(sizeof(CC2530_Global_Para)-1);i++)
        CC2530_Global_Para.CRC_ += p[i];
    Write_DATA_EEPROM((uint8_t*)(&CC2530_Global_Para),sizeof(CC2530_Global_Para)); 
    return 1;
}



uint8_t SetAdcOffset(int8_t offset)
{
    CC2530_Global_Para.ADC_OFFSET=offset;
    SetGlobalData();
    return 1;
}

int8_t GetAdcOffset(void)
{
    return CC2530_Global_Para.ADC_OFFSET;
    
}
void OutPutGloablPara(GlobalData_Para *CC2530_Global_Para)
{   
    printf("ADC_OFFSET:%d\n",CC2530_Global_Para->ADC_OFFSET); 
    printf("terminal_IP is %d.%d.%d.%d\n",CC2530_Global_Para->terminal_addr[0],
        CC2530_Global_Para->terminal_addr[1],
        CC2530_Global_Para->terminal_addr[2],
        CC2530_Global_Para->terminal_addr[3]);
    
    printf("route_addr_IP is %d.%d.%d.%d\n",CC2530_Global_Para->route_addr[0],
        CC2530_Global_Para->route_addr[1],
        CC2530_Global_Para->route_addr[2],
        CC2530_Global_Para->route_addr[3]);
	printf("ADC_LEN:%d\n",CC2530_Global_Para->ADC_LEN);
	printf("CRC_:%d\n",CC2530_Global_Para->CRC_); 
    printf("ADC_Speed:%d\n",CC2530_Global_Para->ADC_Speed);
    printf("boot_flag:%d\n",CC2530_Global_Para->boot_flag);
    printf("dummy:%d\n",CC2530_Global_Para->dummy);
}
