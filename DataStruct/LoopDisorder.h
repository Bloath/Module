#ifndef _LOOPDISORDER_H_
#define _LOOPDISORDER_H_

/* Includes -----------------------------------------------------------*/
#include "../Module_Conf.h"

/* define -------------------------------------------------------------*/
/* macro --------------------------------------------------------------*/
#define CACHE_GET(cache, i) ((void *)((uint32_t)cache->__data + cache->__size * i))

/* typedef ------------------------------------------------------------*/
typedef struct
{
    void *__data;
    uint8_t __counter;    
    uint8_t __size;
    uint8_t _maxLen;
} LoopCacheStruct;

typedef struct
{
    void *__data;
    uint8_t __size;
    uint8_t _maxLen;
    uint32_t _usedFlag;    // usedFlag作为是否被占用的标准位，说明该cache最多只能指向32个成员的对象数组
} DisorderCacheStruct;

/* variables ----------------------------------------------------------*/
/* function prototypes ------------------------------------------------*/
void LoopCache_Init(LoopCacheStruct *loopCache, void *data, uint8_t size, uint8_t maxLen);
void DisorderCache_Init(DisorderCacheStruct *disorderCache, void *data, uint8_t size, uint8_t maxLen);

int LoopCache_Append(LoopCacheStruct *loopCache, void *newData);
void LoopCache_ClearNull(LoopCacheStruct *loopCache, bool (*nullCondition)(void *, void *), void *param);
void LoopCache_Clear(LoopCacheStruct *loopCache);

int DisorderCache_Append(DisorderCacheStruct *disorderCache, void *newData);
int DisorderCache_Get(DisorderCacheStruct *disorderCache);
void DisorderCache_Remove(DisorderCacheStruct *disorderCache, uint8_t index);
void DisorderCache_Clear(DisorderCacheStruct *disorderCache);

#endif
