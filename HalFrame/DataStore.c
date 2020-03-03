/* Includes ----------------------------------------------------------*/
#include "DataStore.h"

/* define ------------------------------------------------------------*/
#define DATA_STORE_WRITE(dst, src, length)                      \
    {                                                           \
        writeResult = DATA_STORE_HAL_WRITE(dst,src,length);     \
        if(writeResult != 0)                                    \
        {   return writeResult; }                               \
    }
/* typedef -----------------------------------------------------------*/
/* macro -------------------------------------------------------------*/
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/
/* functions ---------------------------------------------------------*/
/*********************************************************************************************

  * @brief  存储数据列表初始化
  * @param  dataStoreList：存储数据列表
            length：长度
  * @param  
  * @retval 针对列表中的内容，算出来所有数据针对该内容的偏移量，半字对齐（节省空间），不用字节是为了折中效率和空间

  ********************************************************************************************/
void DataStore_Init(struct DataStoreUnitStruct *dataStoreList, uint16_t unitCount)
{
    uint32_t addressTemp = DATA_STORE_START_ADDRESS;
    
    for(int i=0; i<unitCount; i++)
    {
        dataStoreList[i].__storeAddress = addressTemp;            // 付给存储地址
        addressTemp += ((dataStoreList[i].length % DATA_STORE_SIZE) == 0)? dataStoreList[i].length:((dataStoreList[i].length / DATA_STORE_SIZE) + 1) * DATA_STORE_SIZE;
        addressTemp += DATA_STORE_SIZE;                         // 添加CRC校验位
    }
}
/*********************************************************************************************

  * @brief  DataStoreUnit_Write
  * @param  dataStoreUnit：存储数据单元
  * @param  
  * @retval 单独写入一个存储块

  ********************************************************************************************/
int DataStoreUnit_Write(struct DataStoreUnitStruct *dataStoreUnit)
{
    uint8_t offset = dataStoreUnit->length % DATA_STORE_SIZE;
    uint16_t storeLength = (offset == 0)? dataStoreUnit->length: dataStoreUnit->length - offset + DATA_STORE_SIZE;                  // 计算存储长度
    uint32_t temp32u = 0;                   // 建立临时变量，作为偏移来处理
    uint8_t *p = (uint8_t *)&temp32u;       // 临时变量指针
    uint32_t crc = 0;
    int writeResult = 0;
    
    DATA_STORE_WRITE(dataStoreUnit->__storeAddress, dataStoreUnit->address, (offset == 0)? storeLength: storeLength - DATA_STORE_SIZE);
    
    if(offset != 0)
    {   
        for(int i=0; i<offset; i++)
        {   p[i] = *(uint8_t *)(dataStoreUnit->address + dataStoreUnit->length - offset + i);   }                                   // 将剩余的值写入临时变量
        DATA_STORE_WRITE(dataStoreUnit->__storeAddress + storeLength - DATA_STORE_SIZE, (uint32_t)&temp32u, DATA_STORE_SIZE);                   // 写入
    }
    crc = DATA_STORE_CRC(dataStoreUnit->__storeAddress, storeLength);
    DATA_STORE_WRITE(dataStoreUnit->__storeAddress + storeLength, (uint32_t)&crc, DATA_STORE_SIZE);                                             // 写入CRC
    
    return 0;
}
/*********************************************************************************************

  * @brief  DataStoreUnit_Restore
  * @param  dataStoreUnit：存储数据单元
  * @param  
  * @retval 单独恢复一个存储块

  ********************************************************************************************/
int DataStoreUnit_Restore(struct DataStoreUnitStruct *dataStoreUnit)
{
    uint8_t offset = dataStoreUnit->length % DATA_STORE_SIZE;
    uint16_t storeLength = (offset == 0)? dataStoreUnit->length: dataStoreUnit->length - offset + DATA_STORE_SIZE;              // 计算存储长度
    uint32_t crc = DATA_STORE_CRC(dataStoreUnit->__storeAddress, storeLength);
    int i = 0;
    uint8_t *p = (uint8_t *)&crc;
    
    for(i=0; i<DATA_STORE_SIZE; i++)
    {
        if(*(uint8_t *)(dataStoreUnit->__storeAddress + storeLength + i) != p[i])
        {   break;  }
    }
    
    if(i != DATA_STORE_SIZE)
    {   return -1;  }
    
    memcpy((void*)(dataStoreUnit->address), (void*)(dataStoreUnit->__storeAddress), dataStoreUnit->length);
    
    return 0;
}
/*********************************************************************************************

  * @brief  写入全部数据
  * @param  dataStoreList: 存储单元指针
            unitCount：存储单元个数
  * @retval 

  ********************************************************************************************/
int DataStore_WriteAll(struct DataStoreUnitStruct *dataStoreList, uint16_t unitCount)
{
    int result = 0;
    int flag = 0;
    for(int i=0; i<unitCount; i++)
    {   
        result = DataStoreUnit_Write(dataStoreList + i);  
        if(result != 0)
        {   flag |= (1<<i);   }
    }
    
    return flag;
}
/*********************************************************************************************

  * @brief  恢复全部数据
  * @param  dataStoreList: 存储单元指针
            unitCount：存储单元个数
  * @retval 

  ********************************************************************************************/
int DataStore_RestoreAll(struct DataStoreUnitStruct *dataStoreList, uint16_t unitCount)
{
    int result = 0;
    int flag = 0;
    for(int i=0; i<unitCount; i++)
    {   
        result = DataStoreUnit_Restore(dataStoreList + i);  
        if(result != 0)
        {   flag |= (1<<i);   }   
    }
    
    return flag;
}