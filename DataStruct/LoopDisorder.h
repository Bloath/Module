#ifndef _LOOPDISORDER_H_
#define _LOOPDISORDER_H_

/* Includes -----------------------------------------------------------*/
#include "../Module_Conf.h"

/* define -------------------------------------------------------------*/
/* macro --------------------------------------------------------------*/
#define CACHE_INIT_STATIC(record)   {.data=record, .size=sizeof(record[0]), .maxLen=sizeof(record) / sizeof(record[0])}
#define CACHE_INIT_DYNAMIC(loopPtr, record)    {loop->data=record; loop->size=sizeof(record[0]); loop->maxLen=sizeof(record); }
#define CACHE_GET(cache, i) ((void *)((uint32_t)((cache)->data) + (cache)->size * i))
#define CACHE_FOREACH(cache, cacheBlock)    for(cacheBlock=(cache)->data; (uint32_t)cacheBlock < (uint32_t)((cache)->data) + (cache)->size * (cache)->maxLen; cacheBlock += (cache)->size)

#define DisorderCache_IsIndexAvaliable(cache, i)    (((cache)->__usedFlag & ((uint32_t)1L << i)) != 0)
/* typedef ------------------------------------------------------------*/
typedef struct
{
    void *data;
    uint8_t __counter;    
    uint8_t size;
    uint8_t maxLen;
} LoopCacheStruct;

typedef struct
{
    void *data;
    uint8_t size;
    uint8_t maxLen;
    uint32_t __usedFlag;    // usedFlag作为是否被占用的标准位，说明该cache最多只能指向32个成员的对象数组
} DisorderCacheStruct;

/* variables ----------------------------------------------------------*/
/* function prototypes ------------------------------------------------*/

/*****************************循环缓存*****************************/
void LoopCache_Init(LoopCacheStruct *loopCache, void *data, uint8_t size, uint8_t maxLen);                      // 初始化
int LoopCache_Append(LoopCacheStruct *loopCache, void *newData);                                                // 添加新成员
void LoopCache_ClearNull(LoopCacheStruct *loopCache, bool (*nullCondition)(void *, void *), void *param);       // 通过函数指针清除空成员，并将非空成员前推
void LoopCache_Clear(LoopCacheStruct *loopCache);                                                               // 清空

/*****************************无序缓存*****************************/
void DisorderCache_Init(DisorderCacheStruct *disorderCache, void *data, uint8_t size, uint8_t maxLen);          // 初始化
int DisorderCache_Append(DisorderCacheStruct *disorderCache, void *newData);                                    // 添加新成员
int DisorderCache_Get(DisorderCacheStruct *disorderCache);                                                      // 获取第一个已占用的
void DisorderCache_Remove(DisorderCacheStruct *disorderCache, uint8_t index);                                   // 根据索引将成员从缓存中删除
void DisorderCache_Clear(DisorderCacheStruct *disorderCache);                                                   // 清空

#endif
