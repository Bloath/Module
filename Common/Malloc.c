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
uint8_t mallocPool[MALLOC_POOL_SIZE];                   //�ڴ����뻺���
MallocBlockStruct mallocBlocks[MALLOC_BLOCK_COUNT];     // �ڴ������¼��
uint8_t usedBlockCount = 0;                             // ���õļ�¼��
uint16_t surplusMemory = MALLOC_POOL_SIZE;              // �����ʣ������

/* Private function prototypes ------------------------------------------------*/
/*******************************************************************************
* Function Name   : Malloc
* Description     : �Զ��������ڴ�ĳ��ȣ���malloc��ͬ����ͨ��ȫ�ֳ�����+����������й���
* Parameter       : size�����볤��
* Return          : ���롰�ڴ桱����ָ�룬û���뵽��ΪNULL
*******************************************************************************/
void* Malloc(uint16_t size)
{
  /* ʣ���������� */
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
  
  /* ���Һ���������� */
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
    
    /* ð�ݽ���������� */
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
    /* ���ҿ��п� */
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
* Description     : �ͷ�����ġ��ڴ���С�
* Parameter       : ��Ҫ�ͷŵ�ָ��
* Return          : 
*******************************************************************************/
void Free(void* pointer)
{
  uint16_t startIndex = (uint32_t)pointer - (uint32_t)mallocPool;
  
  //���ҳ��Ȳ�Ϊ0�Ļ����,�һ����ƫ������ͬ��
  for(uint16_t i=0; i<MALLOC_BLOCK_COUNT; i++)
  {
    if(mallocBlocks[i].startIndex == startIndex)
    {
      surplusMemory += mallocBlocks[i].endIndex - mallocBlocks[i].startIndex + 1;     // ʣ���С�ָ�
      mallocBlocks[i].endIndex = 0xFFFF;       //���
      mallocBlocks[i].startIndex = 0xFFFF;       
      usedBlockCount -= 1;                // �������ݼ�
      break;
    }
  }  
}