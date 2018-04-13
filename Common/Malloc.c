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
* Description     : �Զ��������ڴ�ĳ��ȣ���malloc��ͬ����ͨ��ȫ�ֳ�����+����������й���
* Parameter       : size�����볤��
* Return          : ���롰�ڴ桱����ָ�룬û���뵽��ΪNULL
*******************************************************************************/
void* Malloc(uint16_t size)
{
  DISABLE_ALL_INTERRPUTS();
  
  MALLOC_BLOCK_COUNT_SIZE applyBlockCount = ((size % MALLOC_BLOCK_SIZE) == 0)? (size / MALLOC_BLOCK_SIZE): (size / MALLOC_BLOCK_SIZE + 1);
  MALLOC_BLOCK_COUNT_SIZE tempCounter = 0;
  MALLOC_BLOCK_COUNT_SIZE index;

  /* ʣ���������� */
  if(mmu.usedBlockQuantity >= MALLOC_BLOCK_COUNT)
  {
    Malloc_ErrorHandle(Malloc_OutOfMemory);
    return NULL;
  }
  
  /* ѭ�������Ƿ��к��ʵ���������������ֵ��Ϊ0 */
  for(index=0; index<MALLOC_BLOCK_COUNT; index++)
  {
    tempCounter = (mmu.blocks[index] == 0)? (tempCounter + 1) : 0;
    if(tempCounter == applyBlockCount)
    { break; }
  }
  
  /* ����ɨ����������ָ�� */
  if(index != MALLOC_BLOCK_COUNT)
  {
    mmu.usedBlockQuantity += applyBlockCount;
    for(MALLOC_BLOCK_COUNT_SIZE i=0; i<applyBlockCount; i++)
    { mmu.blocks[index - applyBlockCount + i + 1] = applyBlockCount; }
    
    // ����ռ�ڴ�ȫ������
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
* Description     : �ͷ�����ġ��ڴ���С�
* Parameter       : ��Ҫ�ͷŵ�ָ��
* Return          : 
*******************************************************************************/
void Free(void* pointer)
{
  DISABLE_ALL_INTERRPUTS();
  
  MALLOC_BLOCK_COUNT_SIZE index = ((uint32_t)pointer - (uint32_t)mmu.mallocPool) / MALLOC_BLOCK_SIZE;   // ����������һ��
  MALLOC_BLOCK_COUNT_SIZE len = mmu.blocks[index];              // ��������ռ����
  mmu.usedBlockQuantity -= len;                                 
  
  for(MALLOC_BLOCK_COUNT_SIZE i=0; i<len; i++)
  { mmu.blocks[index + i] = 0; }                                // �ڹ���Ԫ���ͷŸ�λ
  ///memset(pointer, 0, MALLOC_BLOCK_SIZE * len);                  // ����ռ�ڴ�ȫ������
  
  ENABLE_ALL_INTERRPUTS();
}