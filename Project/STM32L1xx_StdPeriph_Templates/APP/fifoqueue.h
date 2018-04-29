#ifndef __FIFOQUEUE_H
#define __FIFOQUEUE_H
#include "mempool.h"
#include "string.h"
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_rcc.h"
#include <stdlib.h>


#define ElemType       uint8_t*
#define QueueSize      20 //fifo队列的大小
#define QueueFull      0  //fifo满置0
#define QueueEmpty     1  //FIFO空置1
#define QueueOperateOk 2  //队列操作完成 赋值为2


#define ADC_OK_EVENT         BIT0
#define WARNING_EVENT        BIT1
#define CC2530_STARTUP_EVENT BIT2
typedef struct FifoQueue
{
    uint16_t front;       //队列头
    uint16_t rear;        //队列尾
    uint16_t count;       //队列计数
    ElemType dat[QueueSize];
}FifoQueue;
//Queue Initalize
 void QueueInit(struct FifoQueue *Queue);
// Queue In
 uint8_t QueueIn(struct FifoQueue *Queue,ElemType sdat);
// Queue Out
 uint8_t QueueOut(struct FifoQueue *Queue,ElemType *sdat);
// uint8 GetWarning(uint32* Value);
uint8_t SetEvent(uint8_t evnet);
void ClearEvent(uint8_t event);
uint8_t GetEvent(void);
// uint8 ClearADCEvent();
FifoQueue* GetFifo_Piot(void);
#endif
 
