#ifndef _MALLOC_H_
#define _MALLOC_H_

/* Includes ------------------------------------------------------------------*/
#include "../Module.h"

#include "Malloc_Conf.h"

/* Public typedef ------------------------------------------------------------*/

typedef enum
{
    Malloc_OutOfMemory = 0,
    Malloc_MemoryUnreasonable,
} MallocErrorEnum;

typedef struct
{
    uint32_t __mallocPool[MALLOC_POOL_SIZE];                    // 采用32位，其目的是为了对齐
    MALLOC_BLOCK_COUNT_SIZE __blocks[MALLOC_BLOCK_COUNT];       // 32位为一块，每一块
    MALLOC_BLOCK_COUNT_SIZE _usedBlockQuantity;
} MemoryManageUnitcStruct;

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern MemoryManageUnitcStruct mmu;

/* Public function prototypes ------------------------------------------------*/
void *Malloc(uint16_t size);
void Free(void *pointer);

#endif
