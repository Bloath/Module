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
            unitSize：数据宽度
            unitCount：最大个数
  * @remark 

  ********************************************************************************************/
void LoopCache_Init(struct LoopCacheStruct *loopCache, void *data, uint8_t unitSize, uint8_t unitCount)
{
    loopCache->data = data;
    loopCache->unitSize = unitSize;
    loopCache->unitCount = unitCount;
    loopCache->_currentIndex = 0;
}
/*********************************************************************************************

  * @brief  LoopCache_Init
  * @dcpt   循环控制的初始化
  * @param  loopCache：循环控制指针
            data：被控制的数据指针
            unitSize：数据宽度
            unitCount：最大个数
  * @remark 

  ********************************************************************************************/
void DisorderCache_Init(struct DisorderCacheStruct *disorderCache, void *data, uint8_t unitSize, uint8_t unitCount)
{
    disorderCache->data = data;
    disorderCache->unitSize = unitSize;
    disorderCache->unitCount = unitCount;
    disorderCache->__usedFlag = 0;
}
/*********************************************************************************************

  * @brief  LoopCache_Append
  * @dcpt   向循环队列中插入新的数据
  * @param  loopCache：循环控制指针
            newData新的数据指针
  * @remark

  ********************************************************************************************/
int LoopCache_Append(struct LoopCacheStruct *loopCache, void *newData)
{
    uint8_t index = loopCache->_currentIndex;

    memcpy(CACHE_GET(loopCache, loopCache->_currentIndex), newData, loopCache->unitSize);
    loopCache->_currentIndex++;

    if (loopCache->_currentIndex == loopCache->unitCount)
    {    
        loopCache->_currentIndex = 0; 
        loopCache->_isFull = true;
    }

    // 如果两个索引相当了，那last就必须被current拖动前进
    if(loopCache->_currentIndex == loopCache->_lastIndex)
    {   loopCache->_isCatchUp = true;  }
    if(loopCache->_isCatchUp == true)
    {   loopCache->_lastIndex = loopCache->_currentIndex;   }
    
    return index;
}
/*********************************************************************************************

  * @brief  LoopCache_Handle
  * @dcpt   向循环队列中插入新的数据
  * @param  loopCache：循环控制指针
            DataHandle：数据处理函数
  * @remark 返回总数量

  ********************************************************************************************/
int LoopCache_Handle(struct LoopCacheStruct *loopCache, void (*DataHandle)(void *data, void *param), void *param)
{
    int i=0; 
    
    int count = 0;
    
    if(loopCache->_currentIndex > loopCache->_lastIndex)
    {
        for(i=loopCache->_lastIndex; i<loopCache->_currentIndex; i++)
        {   
            (*DataHandle)(CACHE_GET(loopCache, i), param);  
            count++;
        }
    }
    else if(loopCache->_currentIndex == loopCache->_lastIndex && loopCache->_isCatchUp == false)
    {   return 0;   }
    else
    {
        for(i=loopCache->_lastIndex; i<loopCache->unitCount; i++)
        {   
            (*DataHandle)(CACHE_GET(loopCache, i), param);  
            count++;
        }
        
        for(i=0; i<loopCache->_currentIndex; i++)
        {   
            (*DataHandle)(CACHE_GET(loopCache, i), param);  
            count++;
        }
        loopCache->_isCatchUp = false;
    }

    
    loopCache->_lastIndex = loopCache->_currentIndex;
    return count;
}
/*********************************************************************************************

  * @brief  LoopCache_HandleAll
  * @dcpt   处理loop中所有的数据
  * @param  loopCache：循环控制指针
            DataHandle：数据处理函数
  * @remark 返回总数量

  ********************************************************************************************/
void LoopCache_HandleAll(struct LoopCacheStruct *loopCache, void (*DataHandle)(void *data, void *param), void *param)
{
    int i=0; 
    
    int count = 0;
    
    for(i=loopCache->_currentIndex; i<loopCache->unitCount; i++)
    {   
        (*DataHandle)(CACHE_GET(loopCache, i), param);  
        count++;
    }
    
    for(i=0; i<loopCache->_currentIndex; i++)
    {   
        (*DataHandle)(CACHE_GET(loopCache, i), param);  
        count++;
    }
}
/*********************************************************************************************

  * @brief  DisorderCache_Append
  * @dcpt   在循环队列中找到空闲部分后插入
  * @param  disorderCache：无序缓存控制指针
            newData新的数据指针
  * @remark

  ********************************************************************************************/
int DisorderCache_Append(struct DisorderCacheStruct *disorderCache, void *newData)
{
    uint8_t i;
    for (i = 0; i < disorderCache->unitCount; i++)
    {
        if ((disorderCache->__usedFlag & ((uint32_t)1 << i)) == 0)
        {   break;  }
    }

    if (i == disorderCache->unitCount)
    {   return -1;  }

    memcpy(CACHE_GET(disorderCache, i), newData, disorderCache->unitSize);
    disorderCache->__usedFlag |= ((uint32_t)1 << i);
    return i;
}
/*********************************************************************************************

  * @brief  DisorderCache_Remove
  * @dcpt   从无序队列中删除一个块数据
  * @param  disorderCache：无序缓存控制指针
            index：要删除数据块的索引
  * @remark

  ********************************************************************************************/
void DisorderCache_Remove(struct DisorderCacheStruct *disorderCache, uint8_t index)
{
    memset(CACHE_GET(disorderCache, index), 0, disorderCache->unitSize);
    disorderCache->__usedFlag &= ~((uint32_t)1 << index);
}
/*********************************************************************************************

  * @brief  DisorderCache_Get
  * @dcpt   从无序队列中取出一个已缓存的数据块
  * @param  disorderCache：无序缓存控制指针
  * @retval 返回索引，如果队列中没有，则返回-1
  * @remark

  ********************************************************************************************/
int DisorderCache_Get(struct DisorderCacheStruct *disorderCache)
{
    uint8_t i;

    if (disorderCache->__usedFlag == 0)
    {   return -1;  }

    for (i = 0; i < disorderCache->unitCount; i++)
    {
        if ((disorderCache->__usedFlag & ((uint32_t)1 << i)) != 0)
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
void LoopCache_Clear(struct LoopCacheStruct *loopCache)
{
    memset(loopCache->data, 0, loopCache->unitSize * loopCache->unitCount);
    memset(&loopCache->_isCatchUp, 0, sizeof(struct LoopCacheStruct) - ((uint32_t)(&loopCache->_isCatchUp) - (uint32_t)(loopCache)));
}
/*********************************************************************************************

  * @brief  DisorderCache_Clear
  * @dcpt   将循环队列清空
  * @param  loopCache：循环控制指针
  * @remark 循环队列清零，计数器重置

  ********************************************************************************************/
void DisorderCache_Clear(struct DisorderCacheStruct *disorderCache)
{
    memset(disorderCache->data, 0, disorderCache->unitSize * disorderCache->unitCount);
    memset(&disorderCache->__usedFlag, 0, sizeof(struct DisorderCacheStruct) - ((uint32_t)(&disorderCache->__usedFlag) - (uint32_t)(disorderCache)));
}
/*********************************************************************************************

  * @brief  LoopCache_ClearNull
  * @dcpt   清除空块
  * @param  loopCache：循环控制指针
  * @remark 将块中空块挤出，重新顺位

  ********************************************************************************************/
void LoopCache_ClearNull(struct LoopCacheStruct *loopCache, bool (*nullCondition)(void *, void *), void *param)
{
    uint8_t i, j;

    /* 查找第一个空块 */
    for (i = 0; i < loopCache->unitCount; i++)
    {
        if (nullCondition(CACHE_GET(loopCache, i), param) == true)
        {   break;  }
    }

    /* 从上一次找到的空块往后查找，当找到非空块，则写到空块中，计数器往后推1 */
    for (j = i + 1; j < loopCache->unitCount; j++)
    {
        if (nullCondition(CACHE_GET(loopCache, j), param) == false)
        {
            memcpy(CACHE_GET(loopCache, i), CACHE_GET(loopCache, j), loopCache->unitSize);
            memset(CACHE_GET(loopCache, j), 0, loopCache->unitSize);
            i++;
        }
    }

    loopCache->_currentIndex = i;
}
