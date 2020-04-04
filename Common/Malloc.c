/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "Malloc.h"
#include "Malloc_Conf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
DATA_PREFIX struct MemoryManageUnitcStruct mmu;

/* Private function prototypes ------------------------------------------------*/
/*******************************************************************************

* Function Name   : Malloc
* Description     : 自定义申请内存的长度，与malloc不同，是通过全局长数组+多个申请块进行管理
* Parameter       : size，申请长度
* Return          : 申请“内存”空闲指针，没申请到则为NULL

*******************************************************************************/
void *Malloc(size_t size)
{
    int i = 0;
  
    DISABLE_ALL_INTERRPUTS();
    
    uint8_t *pool = (uint8_t *)mmu.__mallocPool;          // mmu.mallocPool为32位数组（为了对齐），下面的操作都是使用字节数组

    /* 通过size的大小转换为申请块的个数，例如块宽度为32字节，申请size为33，则申请2块，31则申请1块 */
    MALLOC_BLOCK_COUNT_SIZE applyBlockCount = ((size % MALLOC_BLOCK_SIZE) == 0) ? (size / MALLOC_BLOCK_SIZE) : (size / MALLOC_BLOCK_SIZE + 1); 
    MALLOC_BLOCK_COUNT_SIZE tempCounter = 0;
    MALLOC_BLOCK_COUNT_SIZE index;

    /* 剩余容量不足 */
    if (mmu._usedBlockQuantity >= MALLOC_BLOCK_COUNT)
    {
        if(mmu.CallBack_Error != NULL)
        {   mmu.CallBack_Error(Malloc_OutOfMemory); }
        return NULL;
    }

    /* 循环查找是否有合适的区域，连续多个块的值不为0 */
    for (index = 0; index < MALLOC_BLOCK_COUNT; index++)
    {
        tempCounter = (mmu.__blocks[index] == 0) ? (tempCounter + 1) : 0;
        if (tempCounter == applyBlockCount)
        {   break;  }
    }

    /* 根据扫描结果，返回指针 */
    if (index != MALLOC_BLOCK_COUNT)
    {
        mmu._usedBlockQuantity += applyBlockCount;                                   // 使用块个数累加
        
        for (i = 0; i < applyBlockCount; i++)
        {   mmu.__blocks[index - applyBlockCount + i + 1] = applyBlockCount;  }       // 通过循环，将块管理中对应块的地址填充（标记为可用，内容为申请块个数）

        ENABLE_ALL_INTERRPUTS();
        return (void *)(pool + (index - applyBlockCount + 1) * MALLOC_BLOCK_SIZE);
    }
    else
    {
        if(mmu.CallBack_Error != NULL)
        {   mmu.CallBack_Error(Malloc_MemoryUnreasonable);  }
        ENABLE_ALL_INTERRPUTS();
        return NULL;
    }
}
/*******************************************************************************

* Function Name   : Malloc
* Description     : 自定义申请内存的长度，与malloc不同，是通过全局长数组+多个申请块进行管理
* Parameter       : pointer：旧的指针
                    size，申请长度
* Return          : 这个算法暂时没想好，预留后面更新

*******************************************************************************/
void *Realloc(void *pointer, size_t size)
{
    Free(pointer);
    return Malloc(size);
}
/*******************************************************************************
* Function Name   : Free
* Description     : 释放申请的“内存空闲”
* Parameter       : 需要释放的指针
* Return          : 
*******************************************************************************/
void Free(void *pointer)
{
    DISABLE_ALL_INTERRPUTS();

    MALLOC_BLOCK_COUNT_SIZE index = ((uint32_t)pointer - (uint32_t)mmu.__mallocPool) / MALLOC_BLOCK_SIZE; // 查找属于哪一块
    MALLOC_BLOCK_COUNT_SIZE len = mmu.__blocks[index];                                                    // 查找其所占长度
    mmu._usedBlockQuantity -= len;

    //memset(pointer, 0, len * MALLOC_BLOCK_SIZE);
    
    for (MALLOC_BLOCK_COUNT_SIZE i = 0; i < len; i++)
    {   mmu.__blocks[index + i] = 0;  } // 在管理单元中释放该位

    ENABLE_ALL_INTERRPUTS();
}
