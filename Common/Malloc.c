/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Module.h"
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
void *Malloc(uint16_t size)
{
    DISABLE_ALL_INTERRPUTS();
    
    uint8_t *pool = (uint8_t *)mmu.mallocPool;          // mmu.mallocPool为32位数组（为了对齐），下面的操作都是使用字节数组

    /* 通过size的大小转换为申请块的个数，例如块宽度为32字节，申请size为33，则申请2块，31则申请1块 */
    MALLOC_BLOCK_COUNT_SIZE applyBlockCount = ((size % MALLOC_BLOCK_SIZE) == 0) ? (size / MALLOC_BLOCK_SIZE) : (size / MALLOC_BLOCK_SIZE + 1); 
    MALLOC_BLOCK_COUNT_SIZE tempCounter = 0;
    MALLOC_BLOCK_COUNT_SIZE index;

    /* 剩余容量不足 */
    if (mmu.usedBlockQuantity >= MALLOC_BLOCK_COUNT)
    {
        Malloc_ErrorHandle(Malloc_OutOfMemory);
        return NULL;
    }

    /* 循环查找是否有合适的区域，连续多个块的值不为0 */
    for (index = 0; index < MALLOC_BLOCK_COUNT; index++)
    {
        tempCounter = (mmu.blocks[index] == 0) ? (tempCounter + 1) : 0;
        if (tempCounter == applyBlockCount)
        {   break;  }
    }

    /* 根据扫描结果，返回指针 */
    if (index != MALLOC_BLOCK_COUNT)
    {
        mmu.usedBlockQuantity += applyBlockCount;                                   // 使用块个数累加
        
        for (MALLOC_BLOCK_COUNT_SIZE i = 0; i < applyBlockCount; i++)
        {   mmu.blocks[index - applyBlockCount + i + 1] = applyBlockCount;  }       // 通过循环，将块管理中对应块的地址填充（标记为可用，内容为申请块个数）

        memset(pool + (index - applyBlockCount + 1) * MALLOC_BLOCK_SIZE, 0, applyBlockCount * MALLOC_BLOCK_SIZE);   // 将所占内存全部清零

        ENABLE_ALL_INTERRPUTS();
        return (void *)(pool + (index - applyBlockCount + 1) * MALLOC_BLOCK_SIZE);
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
void Free(void *pointer)
{
    DISABLE_ALL_INTERRPUTS();

    MALLOC_BLOCK_COUNT_SIZE index = ((uint32_t)pointer - (uint32_t)mmu.mallocPool) / MALLOC_BLOCK_SIZE; // 查找属于哪一块
    MALLOC_BLOCK_COUNT_SIZE len = mmu.blocks[index];                                                    // 查找其所占长度
    mmu.usedBlockQuantity -= len;

    for (MALLOC_BLOCK_COUNT_SIZE i = 0; i < len; i++)
    {   mmu.blocks[index + i] = 0;  } // 在管理单元中释放该位

    ENABLE_ALL_INTERRPUTS();
}
