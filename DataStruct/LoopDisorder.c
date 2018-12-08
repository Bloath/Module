/* Includes -----------------------------------------------------------*/
#include "LoopDisorder.h"

/* define -------------------------------------------------------------*/
/* macro --------------------------------------------------------------*/
/* typedef ------------------------------------------------------------*/
/* variables ----------------------------------------------------------*/
/* function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  LoopCache_Init
  * @dcpt   循环控制的初始化
  * @param  loopCache：循环控制指针
            data：被控制的数据指针
            size：数据宽度
            maxLen：最大个数
  * @remark 

  ********************************************************************************************/
void LoopCache_Init(LoopCacheStruct *loopCache, void *data, uint8_t size, uint8_t maxLen)
{
    loopCache->data = data;
    loopCache->size = size;
    loopCache->maxLen = maxLen;
    loopCache->counter = 0;
}
/*********************************************************************************************

  * @brief  LoopCache_Init
  * @dcpt   循环控制的初始化
  * @param  loopCache：循环控制指针
            data：被控制的数据指针
            size：数据宽度
            maxLen：最大个数
  * @remark 

  ********************************************************************************************/
void DisorderCache_Init(DisorderCacheStruct *disorderCache, void *data, uint8_t size, uint8_t maxLen)
{
    disorderCache->data = data;
    disorderCache->size = size;
    disorderCache->maxLen = maxLen;
    disorderCache->usedFlag = 0;
}
/*********************************************************************************************

  * @brief  LoopCache_Append
  * @dcpt   向循环队列中插入新的数据
  * @param  loopCache：循环控制指针
            newData新的数据指针
  * @remark

  ********************************************************************************************/
int LoopCache_Append(LoopCacheStruct *loopCache, void *newData)
{
    uint8_t index = loopCache->counter;

    memcpy(CACHE_GET(loopCache, loopCache->counter), newData, loopCache->size);

    loopCache->counter++;

    if (loopCache->counter == loopCache->maxLen)
    {   loopCache->counter = 0;}

    return index;
}
/*********************************************************************************************

  * @brief  DisorderCache_Append
  * @dcpt   在循环队列中找到空闲部分后插入
  * @param  disorderCache：无序缓存控制指针
            newData新的数据指针
  * @remark

  ********************************************************************************************/
int DisorderCache_Append(DisorderCacheStruct *disorderCache, void *newData)
{
    uint8_t i;
    for (i = 0; i < disorderCache->maxLen; i++)
    {
        if ((disorderCache->usedFlag & ((uint32_t)1 << i)) == 0)
        {   break;  }
    }

    if (i == disorderCache->maxLen)
    {   return -1;  }

    memcpy(CACHE_GET(disorderCache, i), newData, disorderCache->size);
    disorderCache->usedFlag |= ((uint32_t)1 << i);
    return i;
}
/*********************************************************************************************

  * @brief  DisorderCache_Remove
  * @dcpt   从无序队列中删除一个块数据
  * @param  disorderCache：无序缓存控制指针
            index：要删除数据块的索引
  * @remark

  ********************************************************************************************/
void DisorderCache_Remove(DisorderCacheStruct *disorderCache, uint8_t index)
{
    memset(CACHE_GET(disorderCache, index), 0, disorderCache->size);
    disorderCache->usedFlag &= ~((uint32_t)1 << index);
}
/*********************************************************************************************

  * @brief  DisorderCache_Get
  * @dcpt   从无序队列中取出一个已缓存的数据块
  * @param  disorderCache：无序缓存控制指针
  * @retval 返回索引，如果队列中没有，则返回-1
  * @remark

  ********************************************************************************************/
int DisorderCache_Get(DisorderCacheStruct *disorderCache)
{
    uint8_t i;

    if (disorderCache->usedFlag == 0)
    {   return -1;  }

    for (i = 0; i < disorderCache->maxLen; i++)
    {
        if ((disorderCache->usedFlag & ((uint32_t)1 << i)) != 0)
        {   return i;   }
    }

    return -1;
}
/*********************************************************************************************

  * @brief  LoopCache_Clear
  * @dcpt   将循环队列清空
  * @param  loopCache：循环控制指针
  * @remark 循环队列清零，计数器重置

  ********************************************************************************************/
void LoopCache_Clear(LoopCacheStruct *loopCache)
{
    memset(loopCache->data, 0, loopCache->size * loopCache->maxLen);
    loopCache->counter = 0;
}
/*********************************************************************************************

  * @brief  DisorderCache_Clear
  * @dcpt   将循环队列清空
  * @param  loopCache：循环控制指针
  * @remark 循环队列清零，计数器重置

  ********************************************************************************************/
void DisorderCache_Clear(DisorderCacheStruct *disorderCache)
{
    memset(disorderCache->data, 0, disorderCache->size * disorderCache->maxLen);
    disorderCache->usedFlag = 0;
}
/*********************************************************************************************

  * @brief  LoopCache_ClearNull
  * @dcpt   清除空块
  * @param  loopCache：循环控制指针
  * @remark 将块中空块挤出，重新顺位

  ********************************************************************************************/
void LoopCache_ClearNull(LoopCacheStruct *loopCache, bool (*nullCondition)(void *, void *), void *param)
{
    uint8_t i, j;

    /* 查找第一个空块 */
    for (i = 0; i < loopCache->maxLen; i++)
    {
        if (nullCondition(CACHE_GET(loopCache, i), param) == true)
        {   break;  }
    }

    /* 从上一次找到的空块往后查找，当找到非空块，则写到空块中，计数器往后推1 */
    for (j = i + 1; j < loopCache->maxLen; j++)
    {
        if (nullCondition(CACHE_GET(loopCache, j), param) == false)
        {
            memcpy(CACHE_GET(loopCache, i), CACHE_GET(loopCache, j), loopCache->size);
            memset(CACHE_GET(loopCache, j), 0, loopCache->size);
            i++;
        }
    }

    loopCache->counter = i;
}
