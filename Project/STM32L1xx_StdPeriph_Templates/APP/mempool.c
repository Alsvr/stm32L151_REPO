#include "mempool.h"
#include "string.h"
#include <stdlib.h>
#include <stdint.h>
#include "ADS869x.h"
static uint8_t mempool[MEM_CNT][ADC_PACKET_SIZE];  //8*256×Ö½Ú=2K
static uint32_t memflag=0;
void* memalloc(){  
  uint8_t i;
  for(i=0;i<MEM_CNT;i++){
    if((memflag&(0x01<<i))==0){
      memflag|=(0x01<<i);
      return (void*)mempool[i];
    }
  }
  return NULL;
}

void memfree(void* addr){
  uint8_t i;
  for(i=0;i<MEM_CNT;i++){
    if((uint8_t*)addr==(uint8_t*)mempool[i]){
      memflag&=~(0x01<<i);
      memset(addr,0,ADC_PACKET_SIZE);
    }
  }
  
}