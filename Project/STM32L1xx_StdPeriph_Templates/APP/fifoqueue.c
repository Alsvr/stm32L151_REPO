//////////////////////////////////////////////////////////
// 文件：FIFOQUEUE.C
//////////////////////////////////////////////////////////
#include "fifoqueue.h"
#include "stdio.h"
//Queue Init
static FifoQueue static_fifo;
//typedef struct WarningSt
//{ 
//    uint32_t ADCValue;
//}WarningSt;

void QueueInit(struct FifoQueue *Queue)
{
    Queue->front = Queue->rear=0;//初始化时队列头队列首相连
    Queue->count = 0;           //队列计数为0
}
    
FifoQueue* GetFifo_Piot()
{
    return &static_fifo;
}
// Queue In
uint8_t QueueIn(struct FifoQueue *Queue,ElemType sdat) //数据进入队列
{
    if((Queue->front == Queue->rear) && (Queue->count == QueueSize))
    {                    // full //判断如果队列满了
        return QueueFull;    //返回队列满的标志
    }else
    {                    // in
        Queue->dat[Queue->rear] = sdat;
        Queue->rear = (Queue->rear + 1) % QueueSize;
        Queue->count = Queue->count + 1;
        return QueueOperateOk;
    }
}
// Queue Out
uint8_t QueueOut(struct FifoQueue *Queue,ElemType *sdat)
{
    if((Queue->front == Queue->rear) && (Queue->count == 0))
    {                    // empty
        return QueueEmpty;
    }
    else
    {                    // out
        *sdat = Queue->dat[Queue->front];
        Queue->front = (Queue->front + 1) % QueueSize;
        Queue->count = Queue->count - 1;
        return QueueOperateOk;
    }
}
//#define MAX_ADC_EVENT_CNT 3  //adc累计发送失败最大次数
//static WarningSt MyWarning={0x12345678};
//static uint8_t GlobalEvent=0x00;
//static uint8_t ADC_OK_EVENT_CNT=0;
//uint8_t GetEvent(void)
//{
//    if(GlobalEvent&ADC_OK_EVENT)
//    {
//        
//        ADC_OK_EVENT_CNT++;
//        if(ADC_OK_EVENT_CNT>MAX_ADC_EVENT_CNT)
//        {
//            ClearEvent(ADC_OK_EVENT);
//            ADC_OK_EVENT_CNT=0;
//        }
//        return ADC_OK_EVENT;
//    }
//    else if(GlobalEvent&WARNING_EVENT)
//    {
//        return WARNING_EVENT;
//    }
//    else if(GlobalEvent&CC2530_STARTUP_EVENT)
//    {
//        return CC2530_STARTUP_EVENT;
//    }
//    else
//        return 0;
//}

//uint8 SetEvent(uint8 evnet)
//{
//    if(evnet&ADC_OK_EVENT)
//    {
//        GlobalEvent|=ADC_OK_EVENT;
//        return 1;
//    }
//    else if(evnet&WARNING_EVENT)
//    {
//        GlobalEvent|=WARNING_EVENT;
//        return 1;
//    }
//    else if(evnet&CC2530_STARTUP_EVENT)
//    {
//        GlobalEvent|=CC2530_STARTUP_EVENT;
//        return 1;
//    }
//    else
//        return 0;
//}

//void ClearEvent(uint8 event)
//{
//    GlobalEvent&=~event; 
//    
//}
//uint8 GetWarning(uint32* Value)
//{
//    if(WARNING_EVENT&GlobalEvent)
//    {
//        *Value=MyWarning.ADCValue;
//        return 1;
//    }
//    return 0;
//}
//uint8 GetADCEvent()
//{
//    if(ADC_OK_EVENT&GlobalEvent)
//    {
//       
//        return 1;
//    }
//    return 0;
//}
//uint8 ClearADCEvent()
//{
//    if(ADC_OK_EVENT&GlobalEvent)
//    {
//        GlobalEvent&=~ADC_OK_EVENT;
//        return 1;
//    }
//    return 0;
//}

