#ifndef _MALLOC_H_
#define _MALLOC_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"

/* Public typedef ------------------------------------------------------------*/
typedef struct
{
  uint16_t startIndex;       
  uint16_t endIndex;   
}MallocBlockStruct;

typedef enum
{
  Malloc_OutOfMemory = 0,
  Malloc_OutOfBlocks,
  Malloc_MemoryUnreasonable,
}MallocErrorEnum;

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void* Malloc(uint16_t size);
void Free(void* pointer);

#endif