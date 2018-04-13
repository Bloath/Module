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
MemoryManageUnitcStruct mmu;

/* Private function prototypes ------------------------------------------------*/
/*******************************************************************************
* Function Name   : Malloc
* Description     : 自定义申请内存的长度，与malloc不同，是通过全局长数组+多个申请块进行管理
* Parameter       : size，申请长度
* Return          : 申请“内存”空闲指针，没申请到则为NULL
*******************************************************************************/
void* Malloc(uint16_t size)
{
  DISABLE_ALL_INTERRPUTS();
  
  MALLOC_BLOCK_COUNT_SIZE applyBlockCount = ((size % MALLOC_BLOCK_SIZE) == 0)? (size / MALLOC_BLOCK_SIZE): (size / MALLOC_BLOCK_SIZE + 1);
  MALLOC_BLOCK_COUNT_SIZE tempCounter = 0;
  MALLOC_BLOCK_COUNT_SIZE index;

  /* 剩余容量不足 */
  if(mmu.usedBlockQuantity >= MALLOC_BLOCK_COUNT)
  {
    Malloc_ErrorHandle(Malloc_OutOfMemory);
    return NULL;
  }
  
  /* 循环查找是否有合适的区域，连续多个块的值不为0 */
  for(index=0; index<MALLOC_BLOCK_COUNT; index++)
  {
    tempCounter = (mmu.blocks[index] == 0)? (tempCounter + 1) : 0;
    if(tempCounter == applyBlockCount)
    { break; }
  }
  
  /* 根据扫描结果，返回指针 */
  if(index != MALLOC_BLOCK_COUNT)
  {
    mmu.usedBlockQuantity += applyBlockCount;
    for(MALLOC_BLOCK_COUNT_SIZE i=0; i<applyBlockCount; i++)
    { mmu.blocks[index - applyBlockCount + i + 1] = applyBlockCount; }
    
    // 将所占内存全部清零
    memset(mmu.mallocPool + (index - applyBlockCount + 1) * MALLOC_BLOCK_SIZE, 0, applyBlockCount * MALLOC_BLOCK_SIZE);                  
    
    ENABLE_ALL_INTERRPUTS();
    return (void *)(mmu.mallocPool + (index - applyBlockCount + 1) * MALLOC_BLOCK_SIZE);
  }
  else
  {  
    Malloc_ErrorHandle(Malloc_MemoryUnreasonable);
    ENABLE_ALL_INTERRPUTS();
    return NULL;
  }
  
}
/*******************************************************************************
* Function Name   : Free
* Description     : 释放申请的“内存空闲”
* Parameter       : 需要释放的指针
* Return          : 
*******************************************************************************/
void Free(void* pointer)
{
  DISABLE_ALL_INTERRPUTS();
  
  MALLOC_BLOCK_COUNT_SIZE index = ((uint32_t)pointer - (uint32_t)mmu.mallocPool) / MALLOC_BLOCK_SIZE;   // 查找属于哪一块
  MALLOC_BLOCK_COUNT_SIZE len = mmu.blocks[index];              // 查找其所占长度
  mmu.usedBlockQuantity -= len;                                 
  
  for(MALLOC_BLOCK_COUNT_SIZE i=0; i<len; i++)
  { mmu.blocks[index + i] = 0; }                                // 在管理单元中释放该位
  ///memset(pointer, 0, MALLOC_BLOCK_SIZE * len);                  // 将所占内存全部清零
  
  ENABLE_ALL_INTERRPUTS();
}