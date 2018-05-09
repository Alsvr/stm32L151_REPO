/**
  ******************************************************************************
  * @file    USART/HyperTerminal_Interrupt/main.c 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    16-May-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx.h"
#include "stdio.h"
#include "wireless.h"
#include "delay.h"
#include "GPIO.h"
#include "lpm.h"
#include "adc.h"
#include "lowpower.h"
#include "FM25WXX.h" 
#include "dataStore.h"
#include "uart.h"
#include "ds18b20.h"
#include "ADS869x.h"
/** @addtogroup STM32L1xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup USART_HyperTerminal_Interrupt
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//doneload cmd

#define AUTO_UPLOAD_ADC_MAX_CNT 30 

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
typedef struct
{
  uint32_t Cmd;

} ServerCmd_TypeDef;


static uint8_t auto_upload_adc_cnt=0;

//extern uint8_t MDK;
//extern uint8_t NbrOfDataToRead;
//extern __IO uint8_t TxCounter; 
//extern __IO uint16_t RxCounter; 
/* Private function prototypes -----------------------------------------------*/
void NVIC_Config(void);
 void RTC_Config(void); 
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
  


void TaskHandler(Node_Instru_Packet *node_instru_packet)
{
    uint32_t cmd;

    
    if(node_instru_packet->commend2 ==SERVER_TO_NODE_CMD_SET_ADC)
    {
        uint16_t adc_len=0;
        uint16_t adc_speed=0;
        //adc_len =node_instru_packet->data[0]+(node_instru_packet->data[1]<<8);
        adc_speed =node_instru_packet->data[2]+(node_instru_packet->data[3]<<8);
        printf("get adc len is %d,sp is %d\n",adc_len,adc_speed);
        
        
         switch(adc_speed)
        {
            case 0:
            case 1:
                adc_len=64;
                adc_speed=1000;
            break;
            case 2:
                adc_len=128;
                adc_speed=2000;
            break;
            
            case 3:
                adc_len=256;
                adc_speed=4000;
            break;
            default:
                adc_len=64;
                adc_speed=1000;
            break;
        }

        SetADCLen(adc_len);   //传入的是256个16位数据
        SetADCSpeed(adc_speed);   //传入的是256个16位数据
        
    }    

    if(node_instru_packet->commend3 ==SERVER_TO_NODE_CMD_CON_SAMP)
    {

    }
    if(node_instru_packet->commend1 ==SERVER_TO_NODE_CMD_START_ADC ||
        auto_upload_adc_cnt>=AUTO_UPLOAD_ADC_MAX_CNT)
    {
            ADS869x_Start_Sample();
            WireLess_Send_ADC_data();
            auto_upload_adc_cnt=0;
    }
    
}
uint8_t Rx[1024];
short temp;
uint32_t ads8699_reg=0;
Node_Instru_Packet node_instru_packet;


uint8_t ConnetTheWifiServer()
{
    uint8_t ret =0;
    //进行联网
    if(WiFi_Enter_CMD_mode())
    {
        ret=WiFi_WaitLinkOk();
    }
    else
    {
        ret = 0;
    }
    
    WiFi_Exit_CMD_mode();
    //联网结束   
    return ret;
}
static uint16_t adc1,adc2,power_rate;
static uint16_t udp_index=0;
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32l1xx_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32l1xx.c file
     */  
    uint8_t delay_i =0;
    uint8_t wifi_connect_flag =0;
    GlobalData_Para *globaldata_p;
    //void globaldata_p;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Led_Init();
    ADXL362_DeInit();
    Led_Open();
    Uart_Log_Configuration();
    delay_init(32);
    FM25VXX_Init();
    globaldata_p=GetGlobalData();
    printf("Please enter AT Config AT+CONFIG\r\n");
    
    delay_ms(5000);
    
    
    
    Init_CC3200(0xC0,0x1210,115200,80);
    Led_Close();
    ADS869x_Init();
    RTC_Config();
    auto_upload_adc_cnt = AUTO_UPLOAD_ADC_MAX_CNT;
#if 1
    while(1)
    {
        Reset_CC3200();
        wifi_connect_flag=0;
        bsp_InitDS18B20();
        if(!DS18B20_ReadTempStep1())
                printf("temp step1 fail!\n");
        //try to connect the wifi server 
        udp_index++;
        wifi_connect_flag=ConnetTheWifiServer();
        if(wifi_connect_flag)
        {
            PowerControl_Init();  //open vibrating sensor power
            
            //delay_ms(100);
            
            //delay for temperature and  vibrating sensor power stable
            temp =DS18B20_ReadTempStep2();  //读取温度
            printf("temp is %f ^C\n",temp * 0.0625);
            bsp_DeInitDS18B20();//temperature read end

            //read power
            power_rate=ADC_Config();
            
            //read vibrating sensor temp ;
            ADS869x_Start_Sample_little(&adc1,&adc2);
            
            //发送查询包到服务器
            if(WiFi_Send_Report(&node_instru_packet,temp,adc1,adc2,power_rate,Get_Node_NUM(),udp_index))
            {
                delay_ms(3000);
                printf("re node_instru_packet.instru is   0x%d\n",node_instru_packet.instru);
                printf("re node_instru_packet.commend1 is 0x%d\n",node_instru_packet.commend1);
                printf("re node_instru_packet.commend2 is 0x%d\n",node_instru_packet.commend2);
                printf("re node_instru_packet.commend3 is 0x%d\n",node_instru_packet.commend3);
                printf("re node_instru_packet.commend4 is 0x%d\n",node_instru_packet.commend4);
                printf("re node_instru_packet.node_addr is 0x%d\n",node_instru_packet.node_addr);
                //根据收到的命令执行相应的命令
                TaskHandler(&node_instru_packet);
            }
            else
            {
                printf("Don't get the Sever response!\n");
                
            }
        }
        else
        {
            printf("WIFI status is fail\n");
            
        }
        PowerControl_DeInit();
        WiFi_EnterLowPowerMode();
        printf("Main Go to Sleep\n");
#if 1
         //进入休眠模式 30s
         Enter_Stop_Mode(); //30S
         //Enter_Stop_Mode(); //30S
         To_Exit_Stop(); 
#else
         for(delay_i=0;delay_i<20;delay_i++)
         {
            delay_ms(1000);

         }
#endif
         //auto_upload_adc_cnt++; //30S  update 

   

        }



         

	
#else
	while (1)
	{
		//Led_Init();
		//delay_init(32);
		//Led_Open();
		//delay_ms(10);
		//Led_Close();
		//Wireless_power_down();
        Enter_Stop_Mode();
		
		To_Exit_Stop();
		//Wireless_power_on();
		delay_ms(1000);
//		delay_ms(30000);	
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
//		delay_ms(30000);
	}
#endif
}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
