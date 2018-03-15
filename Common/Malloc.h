#ifndef _MALLOC_H_
#define _MALLOC_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"

#include "Malloc_Conf.h"

/* Public typedef ------------------------------------------------------------*/

typedef enum
{
  Malloc_OutOfMemory = 0,
  Malloc_MemoryUnreasonable,
}MallocErrorEnum;

typedef struct
{
  uint8_t mallocPool[MALLOC_POOL_SIZE];    
  MALLOC_BLOCK_COUNT_SIZE blocks[MALLOC_BLOCK_COUNT];
  MALLOC_BLOCK_COUNT_SIZE usedBlockQuantity;
  
}MemoryManageUnitcStruct;

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern MemoryManageUnitcStruct mmu;

/* Public function prototypes ------------------------------------------------*/
void* Malloc(uint16_t size);
void Free(void* pointer);

#endif