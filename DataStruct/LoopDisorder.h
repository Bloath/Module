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
