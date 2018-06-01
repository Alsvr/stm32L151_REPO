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
#define REALTIME_DATA_SIE       32 
#define UE_UPDATE_DATA_30S_NUM       20 

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
typedef struct
{
  uint32_t Cmd;

} ServerCmd_TypeDef;

typedef struct
{
    uint16_t acceleration_data[REALTIME_DATA_SIE];
    uint16_t temperature_data[REALTIME_DATA_SIE];

} RealData_TypeDef;


static uint8_t auto_upload_adc_cnt= 0;
static uint16_t adc_threshlod = 0;  
static uint16_t temperature_threshold = 0;  

static uint16_t adc1,adc2,power_rate;
static uint16_t udp_index=0;

uint8_t Rx[1024];
short temp;
uint32_t ads8699_reg=0;
Node_Instru_Packet node_instru_packet;


static RealData_TypeDef realtime_data;  
static uint8_t          realtime_data_cnt=0;

static uint8_t          flag_to_start_wifi_trans=0;

//extern uint8_t MDK;
//extern uint8_t NbrOfDataToRead;
//extern __IO uint8_t TxCounter; 
//extern __IO uint16_t RxCounter; 
/* Private function prototypes -----------------------------------------------*/
void NVIC_Config(void);
void RTC_Config(void); 
uint8_t  App_Send_ReportData(RealData_TypeDef *realtime_data_p ,
                                Node_Instru_Packet *node_instru_packet,
                                uint8_t node_addr, 
                                uint16_t bat_power, 
                                uint8_t threshold_num,
                                uint16_t udp_index);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
  


void TaskHandler(Node_Instru_Packet *node_instru_packet)
{
    uint32_t cmd;
    uint16_t adc_len=0;
    uint16_t adc_speed=0;

    
    if((node_instru_packet->commend1) & SERVER_TO_NODE_CMD_SET_ADC)
    {


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

    if((node_instru_packet->commend1) & SERVER_TO_NODE_CMD_CON_SAMP)
    {
        printf("get continue sample cmd\n");
    }
    if((node_instru_packet->commend1) & SERVER_TO_NODE_CMD_SET_THRESHOLD)
    {
        //get adc_threshold 
        adc_threshlod = node_instru_packet->data[6]+(node_instru_packet->data[7]<<8);  
        //get temp threshold
        temperature_threshold = node_instru_packet->data[8]+(node_instru_packet->data[9]<<8);  
    }
    if(((node_instru_packet->commend1) & SERVER_TO_NODE_CMD_START_ADC) ||
        auto_upload_adc_cnt>=AUTO_UPLOAD_ADC_MAX_CNT)
    {
            ADS869x_Start_Sample();
           
            WireLess_Send_ADC_data();
            auto_upload_adc_cnt=0;
    }
    
}



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
    WiFi_Exit_CMD_mode();
    //联网结束   
    return ret;
}
void App_Variable_Init()
{
    realtime_data_cnt = 0;
    flag_to_start_wifi_trans =0;

}
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32l1xx_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32l1xx.c file
     */  
    uint8_t delay_i =0;
    uint8_t   first_init_wifi=1;
    uint8_t wifi_connect_flag =0;
    uint8_t  realtime_num=0;
    RealData_TypeDef *realtime_data_p;
    GlobalData_Para *globaldata_p;
    App_Variable_Init();
    PowerControl_Init();
    To_Exit_Stop();
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
    delay_ms(2000);
    Led_Close();
#ifdef ADC_DEBUG
    
    ADS869x_Start_Sample_Debug();
    while(1){}
#endif
    RTC_Config();
    auto_upload_adc_cnt = AUTO_UPLOAD_ADC_MAX_CNT;
    while(1)
    {
        
        printf("wake up \n");
        Led_Init();
        Init_CC3200(first_init_wifi,115200);  //
        first_init_wifi=0;
        wifi_connect_flag=0;    
        bsp_InitDS18B20();
        if(!DS18B20_ReadTempStep1())
                printf("temp step1 fail!\n");
        //try to connect the wifi server 
        udp_index++;
        PowerControl_Init();
        delay_ms(500);
        realtime_num = App_get_RealtimeData(realtime_data_p);
        temp =DS18B20_ReadTempStep2();  //读取温度
        printf("temp is %f ^C\n",temp * 0.0625);


        //达到一定的数目  要发送数据给服务器
        if(flag_to_start_wifi_trans)
        {
            flag_to_start_wifi_trans = 0;
            wifi_connect_flag=ConnetTheWifiServer();
            //Wireless_power_down();
            if(wifi_connect_flag)
            {
                //read power
                power_rate=ADC_Config();
                //发送查询包到服务器
                if(App_Send_ReportData(realtime_data_p,
                                        &node_instru_packet,
                                        Get_Node_NUM(),
                                        power_rate,
                                        realtime_num,
                                        udp_index))
                {
                    printf("re node_instru_packet.ADC_sample is   0x%d\n",(node_instru_packet.commend1)&BIT_0);
                    printf("re node_instru_packet.Set_adc is 0x%d\n",(node_instru_packet.commend1)&BIT_1);
                    printf("re node_instru_packet.Con_sample is 0x%d\n",(node_instru_packet.commend1)&BIT_2);
                    printf("re node_instru_packet.Power_sample is 0x%d\n",(node_instru_packet.commend1)&BIT_3);
                    printf("re node_instru_packet.Set threshold is 0x%d\n",(node_instru_packet.commend1)&BIT_4);
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
                printf("WIFI AP is miss\n");
                
            }
        }
        PowerControl_DeInit(); //关闭加速度模块
        WiFi_EnterPowerDownMode(); 
        Wireless_power_down();
        printf("Main Go to Sleep\n");
#if 1
         //进入休眠模式 30s
         Enter_Stop_Mode(); //30S
         To_Exit_Stop(); 
         Enter_Stop_Mode(); //30S
         To_Exit_Stop(); 
         Enter_Stop_Mode(); //30S
         To_Exit_Stop(); 
         Enter_Stop_Mode(); //30S
         To_Exit_Stop(); 
#else
         for(delay_i=0;delay_i<20;delay_i++)
         {
            delay_ms(1000);

         }
#endif
         //auto_upload_adc_cnt++; //30S  update 

   

        }

}



uint8_t App_get_RealtimeData(RealData_TypeDef *realtime_data_p)
{
    uint16_t acceleration_value =0;
    uint8_t re_realtime_data_cnt=0;
    ADS869x_Start_Sample_little(&acceleration_value);
    realtime_data_p = &realtime_data;
    realtime_data.temperature_data[realtime_data_cnt] = temp;
    realtime_data.acceleration_data[realtime_data_cnt++] = acceleration_value;


    if(realtime_data_cnt >= UE_UPDATE_DATA_30S_NUM)
    {
        flag_to_start_wifi_trans =1;
        re_realtime_data_cnt = realtime_data_cnt;
        realtime_data_cnt = 0;
    }
    return re_realtime_data_cnt;
}

uint8_t  App_Send_ReportData(RealData_TypeDef *realtime_data_p ,
                                Node_Instru_Packet *node_instru_packet,
                                uint8_t node_addr, 
                                uint16_t bat_power, 
                                uint8_t threshold_num,
                                uint16_t udp_index)
{
    uint8_t i=0;
    uint8_t re;
    node_instru_packet->header1= NODE_INSTRU_HEAD1;
    node_instru_packet->header2= NODE_INSTRU_HEAD2;
    node_instru_packet->instru = NODE_TO_SERVER_INST_HEART;
    node_instru_packet->node_addr =node_addr;
    //power
    node_instru_packet->data[0] = bat_power&0xff;
    node_instru_packet->data[1] =(bat_power>>8)&0xff;
    //threshold num
    node_instru_packet->data[2] =threshold_num&0xff;
    node_instru_packet->data[3] =(threshold_num>>8)&0xff;
    if(threshold_num>REALTIME_DATA_SIE)
        threshold_num=REALTIME_DATA_SIE;
    
    for(i=0;i<threshold_num;i++)
    {
        //threshold value
        node_instru_packet->data[4+i<<2] =realtime_data_p.acceleration_data[i]&0xff;
        node_instru_packet->data[5+i<<2] =(realtime_data_p.acceleration_data[i]>>8)&0xff;
        node_instru_packet->data[6+i<<2] =realtime_data_p.temperature_data[i]&0xff;
        node_instru_packet->data[7+i<<2] =(realtime_data_p.temperature_data[i]>>8)&0xff;
   }
   node_instru_packet->data[126] =udp_index&0xff;
   node_instru_packet->data[127] =(udp_index>>8)&0xff;
   
   re = WiFi_Send_Report_new(node_instru_packet);
   return re;
      
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
