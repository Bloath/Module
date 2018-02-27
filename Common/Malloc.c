/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Sys_Conf.h"
#include "Malloc.h"
#include "Malloc_Conf.h"
#include "Malloc_Handle.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
uint8_t mallocPool[MALLOC_POOL_SIZE];                   //内存申请缓冲池
MallocBlockStruct mallocBlocks[MALLOC_BLOCK_COUNT];     // 内存申请记录快
uint8_t usedBlockCount = 0;                             // 已用的记录快
uint16_t surplusMemory = MALLOC_POOL_SIZE;              // 缓冲池剩余容量

/* Private function prototypes ------------------------------------------------*/
/*******************************************************************************
* Function Name   : Malloc
* Description     : 自定义申请内存的长度，与malloc不同，是通过全局长数组+多个申请块进行管理
* Parameter       : size，申请长度
* Return          : 申请“内存”空闲指针，没申请到则为NULL
*******************************************************************************/
void* Malloc(uint16_t size)
{
  /* 剩余容量不足 */
  if(surplusMemory < size)
  {
    Malloc_ErrorHandle(Malloc_OutOfMemory);
    return NULL;
  }
  if(usedBlockCount == MALLOC_BLOCK_COUNT)
  {
     Malloc_ErrorHandle(Malloc_OutOfBlocks);
     return NULL;
  }
  
  uint16_t index = 0xFFFF;
  
  /* 查找合适区域并填充 */
  if(usedBlockCount == 0)
  { 
    index = 0; 
    for(uint16_t i=0; i<MALLOC_BLOCK_COUNT; i++)
    {
      mallocBlocks[i].startIndex = 0xFFFF;
      mallocBlocks[i].endIndex = 0xFFFF;
    }
  }
  else
  {
    MallocBlockStruct mallocTemp = {0};
    
    /* 冒泡将块进行排序 */
    for(uint16_t i=1; i<MALLOC_BLOCK_COUNT; i++)
    {
      for(uint16_t j=0; j<MALLOC_BLOCK_COUNT - 1; j++)
      {
        if(mallocBlocks[j].startIndex > mallocBlocks[j + 1].startIndex)
        {
          mallocTemp = mallocBlocks[j];
          mallocBlocks[j] = mallocBlocks[j + 1];
          mallocBlocks[j + 1] = mallocTemp;
        }
      }
    }
    
    uint16_t startIndex = 0;
    /* 查找空闲块 */
    for(uint16_t i=0; i<usedBlockCount; i++)
    {
      if(i == 0 && usedBlockCount != 1)
      {
        if((mallocBlocks[i].startIndex - 0) >= size)
        { 
          index = 0; 
          break;
        }
      }
      else if(i == (usedBlockCount - 1))
      {
        startIndex = ((mallocBlocks[i].endIndex % 4) == 3)? mallocBlocks[i].endIndex: mallocBlocks[i].endIndex + 3 - (mallocBlocks[i].endIndex % 4);
        if((MALLOC_POOL_SIZE - 1 - startIndex) >= size)
        { 
          index = startIndex + 1; 
          break;
        }
      }
      else
      {
        startIndex = ((mallocBlocks[i - 1].endIndex % 4) == 3)? mallocBlocks[i - 1].endIndex: mallocBlocks[i - 1].endIndex + 3 - (mallocBlocks[i - 1].endIndex % 4);
        if((mallocBlocks[i].startIndex - startIndex) >= size)
        { 
          index = startIndex + 1; 
          break;
        }
      }
    }
  }
  
  if(index != 0xFFFF)
  {
    mallocBlocks[usedBlockCount].startIndex = index;
    mallocBlocks[usedBlockCount].endIndex = index + size - 1;
    surplusMemory -= size;
    usedBlockCount += 1; 
  }
  else
  {  
    Malloc_ErrorHandle(Malloc_MemoryUnreasonable);
    return NULL;
  }
  
  return mallocPool + index;
}
/*******************************************************************************
* Function Name   : Free
* Description     : 释放申请的“内存空闲”
* Parameter       : 需要释放的指针
* Return          : 
*******************************************************************************/
void Free(void* pointer)
{
  uint16_t startIndex = (uint32_t)pointer - (uint32_t)mallocPool;
  
  //查找长度不为0的缓冲块,且缓冲块偏移量相同的
  for(uint16_t i=0; i<MALLOC_BLOCK_COUNT; i++)
  {
    if(mallocBlocks[i].startIndex == startIndex)
    {
      surplusMemory += mallocBlocks[i].endIndex - mallocBlocks[i].startIndex + 1;     // 剩余大小恢复
      mallocBlocks[i].endIndex = 0xFFFF;       //清空
      mallocBlocks[i].startIndex = 0xFFFF;       
      usedBlockCount -= 1;                // 计数器递减
      break;
    }
  }  
}