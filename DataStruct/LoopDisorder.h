#ifndef _LOOPDISORDER_H_
#define _LOOPDISORDER_H_

/* Includes -----------------------------------------------------------*/
#include "Module/Module_Conf.h"

/* define -------------------------------------------------------------*/
/* macro --------------------------------------------------------------*/
#define CACHE_INIT_STATIC(record)   {.data=record, .unitSize=sizeof(record[0]), .unitCount=sizeof(record) / sizeof(record[0])}
#define CACHE_INIT_DYNAMIC(loop, record)    {(loop)->data=record; (loop)->unitSize=sizeof(record[0]); (loop)->unitCount=sizeof(record) / (loop)->unitSize; (loop)->__counter = 0;}
#define CACHE_GET(cache, i) ((void *)((uint32_t)((cache)->data) + (cache)->unitSize * i))
#define CACHE_FOREACH(cache, cacheBlock)    for(cacheBlock=(cache)->data; (uint32_t)cacheBlock < (uint32_t)((cache)->data) + (cache)->unitSize * (cache)->unitCount; cacheBlock += (cache)->unitSize)
#define LOOPCACHE_COUNT(cache)  (((cache)->_isFull == true)? (cache)->unitCount : (cache)->_currentIndex)
#define DisorderCache_IsIndexAvaliable(cache, i)    (((cache)->__usedFlag & ((uint32_t)1L << i)) != 0)
/* typedef ------------------------------------------------------------*/
struct LoopCacheStruct
{
    void *data;                     // 数据指针
    uint8_t unitSize;               // 数据宽度
    uint16_t unitCount;              // 数据个数
    
    bool _isCatchUp;                // 是否被赶超
    bool _isFull;                   // 是否已满
    uint16_t _currentIndex;          // 当前数据索引
    uint16_t _lastIndex;             // 上次数据索引，与当前索引配合可查看新入库数据
};

struct DisorderCacheStruct
{
    void *data;
    uint8_t unitSize;
    uint8_t unitCount;
    uint32_t __usedFlag;    // usedFlag作为是否被占用的标准位，说明该cache最多只能指向32个成员的对象数组
};

/* variables ----------------------------------------------------------*/
/* function prototypes ------------------------------------------------*/

/*****************************循环缓存*****************************/
void LoopCache_Init(struct LoopCacheStruct *loopCache, void *data, uint8_t unitSize, uint8_t unitCount);                      // 初始化
int LoopCache_Append(struct LoopCacheStruct *loopCache, void *newData);                                                // 添加新成员
void LoopCache_ClearNull(struct LoopCacheStruct *loopCache, bool (*nullCondition)(void *, void *), void *param);       // 通过函数指针清除空成员，并将非空成员前推
void LoopCache_Clear(struct LoopCacheStruct *loopCache);          
int LoopCache_Handle(struct LoopCacheStruct *loopCache, void (*DataHandle)(void *data, void *param), void *param);
void LoopCache_HandleAll(struct LoopCacheStruct *loopCache, void (*DataHandle)(void *data, void *param), void *param);
int LoopCache_FindSimilar(struct LoopCacheStruct *loopCache, bool (*IsSimilar)(void *current, void *contrast, void *param), void *param, void **result);

/*****************************无序缓存*****************************/
void DisorderCache_Init(struct DisorderCacheStruct *disorderCache, void *data, uint8_t unitSize, uint8_t unitCount);          // 初始化
int DisorderCache_Append(struct DisorderCacheStruct *disorderCache, void *newData);                                    // 添加新成员
int DisorderCache_Get(struct DisorderCacheStruct *disorderCache);                                                      // 获取第一个已占用的
void DisorderCache_Remove(struct DisorderCacheStruct *disorderCache, uint8_t index);                                   // 根据索引将成员从缓存中删除
void DisorderCache_Clear(struct DisorderCacheStruct *disorderCache);                                                   // 清空

#endif
