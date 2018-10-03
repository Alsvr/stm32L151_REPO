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


#define AUTO_UPLOAD_ADC_MAX_CNT 1 
#define REALTIME_DATA_SIE       32 




#define UE_DETECTION_NORMAL_MODE     0x00
#define UE_DETECTION_EMERGENCY_MODE  0x01
#define UE_DETECTION_PROTECT_MODE    0x02







//emergency  0

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


static uint8_t normal_detection_state= 0;
static uint8_t ue_emergency_state_cnt= 0;

static uint8_t auto_upload_adc_cnt= 0;
static uint16_t acceleration_threshlod_g = 0;  
static uint16_t temperature_threshold_g = 0;  
static uint8_t  first_boot=0;

static uint16_t power_rate;
static uint16_t udp_index=0;

//记录未收到服务器反馈的总次数
static uint32_t ser_nack_count=0;
//记录收到服务器反馈的总次数
static uint32_t ser_ack_count=0;


//节点工作的持续时间
static uint32_t cpu_active_time_30S = 0;

//uint8_t Rx[1024];
short temp;
uint32_t ads8699_reg=0;
Node_Instru_Packet node_instru_packet;


static RealData_TypeDef realtime_data_g;  

static uint8_t          realtime_data_cnt=0;

//static uint8_t          flag_to_start_wifi_trans=0;

static uint8_t  ue_update_date_cnt_30s =0;

//extern uint8_t MDK;
//extern uint8_t NbrOfDataToRead;
//extern __IO uint8_t TxCounter; 
//extern __IO uint16_t RxCounter; 
/* Private function prototypes -----------------------------------------------*/
void NVIC_Config(void);
//void RTC_Config(void); 
uint8_t  App_Send_ReportData(RealData_TypeDef *realtime_data_p ,
                                Node_Report_Packet *node_instru_packet,
                                uint8_t node_addr, 
                                uint16_t bat_power, 
                                uint8_t threshold_num,
                                uint16_t udp_index);
uint8_t App_get_RealtimeData(RealData_TypeDef *realtime_data_p);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
  


void TaskHandler(Server_Instru_Packet *server_instru_packet)
{
    uint16_t adc_len=0;
    uint16_t adc_speed=0;

    uint16_t adc_len_save=0;
    uint16_t adc_speed_save=0;

    printf("ser adc_valid: %d\n",server_instru_packet->adc_valid);
    printf("ser adc_set_valid: %d\n",server_instru_packet->adc_config_valid);
    printf("ser Con_sample valid: 0x%d\n",server_instru_packet->continue_sample_valid);
    printf("ser power sample valid: 0x%d\n",server_instru_packet->power_sample_valid);
    printf("ser thres set valid: 0x%d\n",server_instru_packet->thres_hold_valid[0]);
    printf("ser node: 0x%d\n",server_instru_packet->node_addr);
    //设置adc命令有效                
    if((server_instru_packet->adc_config_valid)&SERVER_TO_NODE_CMD_SET_ADC)
    {
        //adc_len = server_instru_packet->adc_len[0] + (server_instru_packet->adc_len[1]<<8);
        adc_len = SERVER_TO_NODE_CMD_ADC_LEN_4S;
        adc_speed = server_instru_packet->adc_speed[0] + (server_instru_packet->adc_speed[1]<<8);
        if(adc_len == SERVER_TO_NODE_CMD_ADC_LEN_4S)
        {
            if(adc_speed == SERVER_TO_NODE_CMD_ADC_SPEED_1K )
            {
                adc_len_save = 64;
                adc_speed_save=1000;

            }
            else if(adc_speed == SERVER_TO_NODE_CMD_ADC_SPEED_2K )
            {
                adc_len_save = 128;
                adc_speed_save=2000;

            }
            else if(adc_speed == SERVER_TO_NODE_CMD_ADC_SPEED_4K )
            {
                adc_len_save = 256;
                adc_speed_save=4000;

            }
            else
            {
                adc_len_save = 64;
                adc_speed_save=1000;
             }
       }
        else if(adc_len == SERVER_TO_NODE_CMD_ADC_LEN_8S)
        {
            if(adc_speed == SERVER_TO_NODE_CMD_ADC_SPEED_1K )
            {
                adc_len_save = 128;
                adc_speed_save=1000;

            }
            else if(adc_speed == SERVER_TO_NODE_CMD_ADC_SPEED_2K )
            {
                adc_len_save = 256;
                adc_speed_save=2000;

            }
            else
            {
                adc_len_save = 64;
                adc_speed_save=1000;
             }

       }
        else if(adc_len == SERVER_TO_NODE_CMD_ADC_LEN_16S)
        {
            if(adc_speed == SERVER_TO_NODE_CMD_ADC_SPEED_1K )
            {
                adc_len_save = 256;
                adc_speed_save=1000;

            }
            else
            {
                adc_len_save = 64;
                adc_speed_save=1000;
             }
        }
        else
        {
            adc_len_save = 64;
            adc_speed_save=1000;

        }
        printf("get adc len is %d,sp is %d\n",adc_len_save,adc_speed_save);
        DataStoreSetADC(adc_len_save,adc_speed_save);   //传入的是256个16位数据
    }    
    //采集上次数据命令
    if((server_instru_packet->continue_sample_valid)&SERVER_TO_NODE_CMD_CON_SAMP)
    {
        printf("get continue sample cmd\n");
    }
    //设置报警阈值命令
    if((server_instru_packet->thres_hold_valid[0]) & SERVER_TO_NODE_CMD_SET_THRESHOLD)
    {
        //get adc_threshold 
        acceleration_threshlod_g = server_instru_packet->thres_hold_accelebration[0] +(server_instru_packet->thres_hold_accelebration[1]<<8);  
        //get temp threshold
        temperature_threshold_g = server_instru_packet->thres_hold_temperature[0]+(server_instru_packet->thres_hold_temperature[1]<<8);  
        printf("Threshold set temp is%d，acc is%d\n",temperature_threshold_g,acceleration_threshlod_g);
        Set_Node_Temperature_threshold(temperature_threshold_g);
        Set_Node_accelebration_threshold(acceleration_threshlod_g);
        SetGlobalData();
    }
    //得到采集adc命令
    if(((server_instru_packet->adc_valid) & SERVER_TO_NODE_CMD_START_ADC) ||
        auto_upload_adc_cnt >= AUTO_UPLOAD_ADC_MAX_CNT)
    {
        ADS869x_Start_Sample(); //开始采集ADC
        WireLess_Send_ADC_data();//开始发送ADC数据
        auto_upload_adc_cnt=0;
    }
    
}



uint8_t ConnetTheWifiServer()
{
#if 1
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
    delay_ms(1500);
    //WiFi_Exit_CMD_mode();
    //联网结束   
    return ret;
#else 
    

    return Wireless_Get_link_status();
#endif
}
void App_Variable_Init(GlobalData_Para* globaldata_p)
{
    realtime_data_cnt = 0;
//    flag_to_start_wifi_trans =0;
    ue_update_date_cnt_30s = UE_UPDATE_DATA_30S_NUM ;
    normal_detection_state = UE_DETECTION_NORMAL_MODE;

    temperature_threshold_g = globaldata_p->temperature_threshold;
    acceleration_threshlod_g = globaldata_p->ADC_threshold;
    ser_ack_count =0;
    ser_nack_count =0;
}
int main(void)
{
    /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32l1xx_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32l1xx.c file
     */  
    uint8_t first_init_wifi=1;
    uint8_t wifi_connect_flag =0;
    uint8_t realtime_num=0;
    Node_Report_Packet node_report_packet;
    RealData_TypeDef *realtime_data_p=&realtime_data_g;
    GlobalData_Para *globaldata_p;

    /* Check if the system has resumed from WWDG reset */
    if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)
    { 
        /* WWDGRST flag set */
        /* Clear reset flags */
        RCC_ClearFlag();
    }
    delay_init(32);
    
    PowerControl_Init();
    To_Exit_Stop();
    //void globaldata_p;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Led_Init();
    ADXL362_DeInit();
    Led_Open();
    Uart_Log_Configuration();
    printf("Node Power On good luck!\r\n");

    FM25VXX_Init();
    globaldata_p=GetGlobalData();
    App_Variable_Init(globaldata_p);
    printf("Please enter AT Config AT+CONFIG\r\n");
    delay_ms(2000);
    Led_Close();
    //调试ADC
#ifdef ADC_DEBUG
    ADS869x_Start_Sample_Debug();
    while(1)
    {

    }
#endif
    RTC_Config(UE_SLEEP_TIME_S);
    //立即触发采样
    auto_upload_adc_cnt = AUTO_UPLOAD_ADC_MAX_CNT;

    //ADC_Config();
    while(1)
    {
        /* Check if the system has resumed from WWDG reset */
        if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)
        { 
            /* WWDGRST flag set */
            /* Clear reset flags */
            RCC_ClearFlag();
        }
        printf("**Node %d wake up ack %d,nack %d **\n",globaldata_p->node_num,ser_ack_count,ser_nack_count);
        PowerControl_Init();
        if(!DS18B20_ReadTempStep1())
            printf("temp step1 fail!\n");
        
        Led_Init();
        Led_Open();
        
        delay_ms(600);
        realtime_num = App_get_RealtimeData(realtime_data_p);
      
        Led_Close();
        //达到一定的数目  要发送数据给服务器 但是开机第一次要上传
        if(!first_boot)
        {
            realtime_num=1;
            first_boot=1;
        }
        if(realtime_num)  //满足上传信息的要求 会开启Wifi
        {
            Init_CC3200(first_init_wifi,115200);  //初始化Wifi，如果是第一次使用Wifi
            first_init_wifi=0;
            //try to connect the wifi server 
            wifi_connect_flag=ConnetTheWifiServer(); //连接到路由器
            udp_index++;

            if(wifi_connect_flag)
            {
                //read power
                power_rate=ADC_Config();
                printf("power is %d\n",power_rate);
                //发送查询包到服务器
                if(App_Send_ReportData(realtime_data_p,
                                        &node_report_packet,
                                        Get_Node_NUM(),
                                        power_rate,
                                        realtime_num,
                                        udp_index))
                {
                    
                    //根据收到的命令执行相应的命令
                    TaskHandler((Server_Instru_Packet *)&node_report_packet);
                    ser_ack_count++;
                }
                else
                {
                    printf("Don't get the Sever response!\n");
                    ser_nack_count++;

                }
            }
            else
            {
                printf("WIFI AP is miss\n");

                
            }
            WiFi_EnterPowerDownMode(); 
            Wireless_power_down();
        }
        PowerControl_DeInit(); //关闭加速度模块

        printf("Main Go to Sleep\n");
#if 1
         //进入休眠模式 30s
         Enter_Stop_Mode(); //30S
         To_Exit_Stop();
         cpu_active_time_30S++;
         if(cpu_active_time_30S>=UE_REST_SELF_PERIOD30S)
         {
               __disable_irq();
               NVIC_SystemReset();

         }
          
#else
         for(delay_i=0;delay_i<20;delay_i++)
         {
            delay_ms(1000);

         }
#endif
        }

}



uint8_t App_get_RealtimeData(RealData_TypeDef *realtime_data_p)
{
    uint16_t acurrent_cceleration_value =0;
    uint16_t current_temp_value =0;
    uint8_t re_realtime_data_cnt=0;
    uint8_t threshold_alarm=0;
    //读取实时加速度
    ADS869x_Start_Sample_little(&acurrent_cceleration_value);
    //读取实时温度
    current_temp_value =DS18B20_ReadTempStep2();

    printf("detection Mode is %d，distance to report is%d\n",
        normal_detection_state,
        ue_update_date_cnt_30s-realtime_data_cnt);
    
    //将本次的实时数据存入数组
    realtime_data_p->temperature_data[realtime_data_cnt] = current_temp_value;
    realtime_data_p->acceleration_data[realtime_data_cnt] = acurrent_cceleration_value;

   //打印当前的温度值和加速度实时值和阈值
    printf("current temp is %f ^C,limit is %f\n",
        realtime_data_p->temperature_data[realtime_data_cnt]* 0.0625,
        temperature_threshold_g*0.0625);
    printf("current acceleration is %d, limit is%d\n",
        realtime_data_p->acceleration_data[realtime_data_cnt],
        acceleration_threshlod_g);

    if(normal_detection_state == UE_DETECTION_NORMAL_MODE)
    {

        //正常模式下判断是否超过限制
        if(current_temp_value>temperature_threshold_g ||
        acurrent_cceleration_value >acceleration_threshlod_g)
        {
            threshold_alarm = 1; //超标警报
            //打印报警类别
            if(current_temp_value>temperature_threshold_g)
                printf("Node into emergency mode,temperature Alarm %d\n",current_temp_value);
            if(acurrent_cceleration_value>acceleration_threshlod_g)
                printf("Node into emergency mode ,Accelebration Alarm %d\n",acurrent_cceleration_value);
            //进入紧急模式 5min 更新一次 且上传ADC数据
            ue_update_date_cnt_30s = UE_UPDATE_DATA_30S_EMERGENCY_NUM;  //上报间隔次数
            normal_detection_state = UE_DETECTION_EMERGENCY_MODE;  //更新模式
            ue_emergency_state_cnt = 0;  //emergency mode计数复位
            //立即触发采样 
            auto_upload_adc_cnt = AUTO_UPLOAD_ADC_MAX_CNT;
            realtime_data_cnt=0;
        }
    }
    else if(normal_detection_state == UE_DETECTION_EMERGENCY_MODE)
    {
        ue_emergency_state_cnt++;

        if(ue_emergency_state_cnt > (UE_UPDATE_DATA_PERIOD_EMERGENCY_NUM))
        {
            printf("Node into protect mode %d\n",current_temp_value);
            
            normal_detection_state = UE_DETECTION_PROTECT_MODE;   //更新模式
            ue_update_date_cnt_30s = UE_UPDATE_DATA_30S_NUM;   //上报间隔次数
            ue_emergency_state_cnt = 0;
            realtime_data_cnt=0;
        }
    }
    else if(normal_detection_state == UE_DETECTION_PROTECT_MODE)
    {
        ue_emergency_state_cnt++;
        if(ue_emergency_state_cnt > UE_UPDATE_DATA_PERIOD_PRETECION_NUM)
        {
            ue_emergency_state_cnt = 0;
            normal_detection_state = UE_DETECTION_NORMAL_MODE;   //更新模式
            ue_update_date_cnt_30s = UE_UPDATE_DATA_30S_NUM;  //上报间隔次数
            realtime_data_cnt=0;
        }
    }
    
    realtime_data_cnt++;
    
    if((realtime_data_cnt >= ue_update_date_cnt_30s) ||
       (threshold_alarm))
    {
        re_realtime_data_cnt = realtime_data_cnt;
        realtime_data_cnt = 0;
        return re_realtime_data_cnt;
    }
    return 0;
    
}

uint8_t  App_Send_ReportData(RealData_TypeDef *realtime_data_p ,
                                Node_Report_Packet *node_report_packet,
                                uint8_t node_addr, 
                                uint16_t bat_power, 
                                uint8_t threshold_num,
                                uint16_t udp_index)
{
    uint8_t i=0;
    uint8_t re;
    node_report_packet->header1= NODE_INSTRU_HEAD1;
    node_report_packet->header2= NODE_INSTRU_HEAD2;
    node_report_packet->instru = NODE_TO_SERVER_INST_HEART;
    node_report_packet->node_addr =node_addr;
    //power
    node_report_packet->power[0] = bat_power&0xff;
    node_report_packet->power[1] =(bat_power>>8)&0xff;
    //填充实时数据的个数
    if(threshold_num>REALTIME_DATA_SIE)
        threshold_num=REALTIME_DATA_SIE;
    
    node_report_packet->temp_adc_num[0] =threshold_num&0xff;
    node_report_packet->temp_adc_num[1] =(threshold_num>>8)&0xff;
    
    for(i=0;i<threshold_num;i++)
    {
        printf("temp is %d, adc is %d\n",
        realtime_data_p->temperature_data[i],
        realtime_data_p->acceleration_data[i]);
        
        node_report_packet->temperature_data[(i<<1)] =realtime_data_p->temperature_data[i]&0xff;
        node_report_packet->temperature_data[1+(i<<1)] =(realtime_data_p->temperature_data[i]>>8)&0xff;
        
        //threshold value
        node_report_packet->acceleration_data[(i<<1)] =realtime_data_p->acceleration_data[i]&0xff;
        node_report_packet->acceleration_data[1 +(i<<1)] =(realtime_data_p->acceleration_data[i]>>8)&0xff;
        

   }
   node_report_packet->udp_index[0] =udp_index&0xff;
   node_report_packet->udp_index[1] =(udp_index>>8)&0xff;
   
   re = WiFi_Send_Report_new(node_report_packet);
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
