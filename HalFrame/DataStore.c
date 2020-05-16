/* Includes ----------------------------------------------------------*/
#include "DataStore.h"
#include "Module/Module.h"

/* define ------------------------------------------------------------*/
#define DATA_STORE_WRITE(dst, src, length)                      \
    {                                                           \
        writeResult = DATA_STORE_HAL_WRITE(dst,src,length);     \
        if(writeResult != 0)                                    \
        {   return writeResult; }                               \
    }
/* typedef -----------------------------------------------------------*/
/* macro -------------------------------------------------------------*/
#define SAVE_SIZE(size)   (((size % 4) == 0)? size:((size / 4) + 1) * 4)
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/
/* functions ---------------------------------------------------------*/
/*********************************************************************************************

  * @brief  存储列表添加
  * @param  dataStore：存储数据
            address: 要存储的对象的地址
            length：要存储的对象的长度
  * @param  
  * @retval 添加到队列中

  ********************************************************************************************/
int DataStore_Add(struct DataStoreStruct *dataStore, void* address, uint16_t size)
{
    if(dataStore->dataStoreList == NULL)
    {   return -1;  }
    
    dataStore->dataStoreList[dataStore->count].address = address;
    dataStore->dataStoreList[dataStore->count].size = size;
    dataStore->dataLength += SAVE_SIZE(size);
    dataStore->count += 1;
    
    return 0;
}
/*********************************************************************************************

  写入全部数据

  ********************************************************************************************/
int DataStore_Write(struct DataStoreStruct *dataStore)
{
    uint8_t *tempArray = NULL;
    uint32_t crc = 0, tempAddress;
    int result = 0;
    
    /* 1. 检查读写回调函数是否为空 */
    if(dataStore->CallBack_HalWrite == NULL)
    {   return -1;  }
    
    /* 2. 申请内存准备存放数据 */
    tempArray = (uint8_t *)Malloc(dataStore->dataLength + 4);
    if(tempArray == NULL)
    {   return -2;  }
    memset(tempArray, 0, dataStore->dataLength + 4);
    
    /* 3. 将需要存储的数据填充到临时数组中 */
    tempAddress = (uint32_t)tempArray;
    for(int i=0; i<dataStore->count; i++)
    {   
        memcpy((void*)tempAddress, 
               dataStore->dataStoreList[i].address, 
               dataStore->dataStoreList[i].size);   
        tempAddress += SAVE_SIZE(dataStore->dataStoreList[i].size);
    }
    
    /* 4. 计算CRC并填充到数据中 */
    crc = Crc32(tempArray, dataStore->dataLength, 0xFFFFFFFF);
    memcpy(tempArray + dataStore->dataLength, &crc, 4);
    
    /* 5. 写入存储设备中 */
    result = dataStore->CallBack_HalWrite((uint32_t)(dataStore->startAddress), (uint32_t)tempArray, dataStore->dataLength + 4);
    
    Free(tempArray);
    
    return result;
}
/*********************************************************************************************

  恢复全部数据

  ********************************************************************************************/
int DataStore_Restore(struct DataStoreStruct *dataStore)
{
    uint8_t *tempArray = NULL;
    uint32_t crc = 0, tempAddress;
    int result = 0;
    
    /* 1. 检查读写回调函数是否为空 */
    if(dataStore->CallBack_HalRead == NULL)
    {   return -1;  }
    
    /* 2. 申请内存准备存放数据 */
    tempArray = (uint8_t *)Malloc(dataStore->dataLength + 4);
    if(tempArray == NULL)
    {   return -2;  }
    memset(tempArray, 0, dataStore->dataLength + 4);
    
    /* 3.将数据读取出来并计算crc */
    dataStore->CallBack_HalRead((uint32_t)tempArray, dataStore->startAddress, dataStore->dataLength +  4);
    crc = Crc32(tempArray, dataStore->dataLength, 0xFFFFFFFF);
    
    /* 4. 校验crc是否正确 */
    if(crc != *(uint32_t *)(tempArray + dataStore->dataLength))
    {   return -2;  }
    
    /* 5. 恢复数据 */
    tempAddress = (uint32_t)(tempArray);
    for(int i=0; i<dataStore->count; i++)
    {
        memcpy(dataStore->dataStoreList[i].address,
               (void*)tempAddress,
               dataStore->dataStoreList[i].size);
        tempAddress += SAVE_SIZE(dataStore->dataStoreList[i].size);
    }
    
    /* 释放内存 */
    Free(tempArray);
    
    return 0;
}