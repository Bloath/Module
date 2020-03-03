#ifndef _MALLOC_H_
#define _MALLOC_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"
#include "Malloc_Conf.h"

/* Public typedef ------------------------------------------------------------*/

enum MallocErrorEnum
{
    Malloc_OutOfMemory = 0,
    Malloc_MemoryUnreasonable,
};

struct MemoryManageUnitcStruct
{
    uint32_t __mallocPool[MALLOC_POOL_SIZE];                    // 采用32位，其目的是为了对齐
    MALLOC_BLOCK_COUNT_SIZE __blocks[MALLOC_BLOCK_COUNT];       // 32位为一块，每一块
    MALLOC_BLOCK_COUNT_SIZE _usedBlockQuantity;
    int (*CallBack_Error)(enum MallocErrorEnum mallocError);
};

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern struct MemoryManageUnitcStruct mmu;

/* Public function prototypes ------------------------------------------------*/
void *Malloc(size_t size);
void *Realloc(void *pointer, size_t size);
void Free(void *pointer);

#endif
