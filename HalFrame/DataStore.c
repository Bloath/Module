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
#define SIZE_ALIGN(size)   (((size % 4) == 0)? size:((size / 4) + 1) * 4)
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/
/* functions ---------------------------------------------------------*/

/*********************************************************************************************

  写入全部数据

  ********************************************************************************************/
int DataStore_Write(struct DataStoreUnitStruct *unit, uint32_t dataAddress, uint16_t writeLength)
{
    uint8_t *tempArray = NULL;
    uint32_t crc = 0, totalLength = SIZE_ALIGN(unit->maxLength) + 4;
    
    /* 1. 检查读写回调函数是否为空 */
    if(unit->api->CallBack_HalWrite == NULL)
    {   return ERR_IS_NULL;  }
    
    /* 2. 申请内存准备存放数据 */
    tempArray = (uint8_t *)Malloc(totalLength);
    if(tempArray == NULL)
    {   return ERR_ALLOC_FAILED;  }
    memset(tempArray, 0, SIZE_ALIGN(totalLength));
    
    /* 3. 将需要存储的数据填充到临时数组中 */
    memcpy(tempArray, 
           (void*)dataAddress, 
           (writeLength != 0)? writeLength : unit->maxLength);   
        
    /* 4. 计算CRC并填充到数据中 */
    crc = Crc32(tempArray, unit->maxLength, 0xAABBCCDD);
    memcpy(tempArray + totalLength - 4, &crc, 4);
    
    /* 5. 写入存储设备中 */
    if(unit->api->CallBack_HalWrite(unit->storeAddress, (uint32_t)tempArray, totalLength) != 0)
    {   return ERR_HAL_FAULT;  }
    
    Free(tempArray);
    
    return ERR_SUCCESS;
}
/*********************************************************************************************

  恢复全部数据

  ********************************************************************************************/
int DataStore_Restore(struct DataStoreUnitStruct *unit, uint32_t dataAddress, uint16_t readLength)
{
    uint8_t *tempArray = NULL;
    uint32_t crc = 0, totalLength = SIZE_ALIGN(unit->maxLength) + 4;
    
    if(dataAddress != unit->storeAddress)
    {
        /* 1. 检查读写回调函数是否为空 */
        if(unit->api->CallBack_HalRead == NULL)
        {   return ERR_IS_NULL;  }
        
        /* 2. 申请内存准备存放数据 */
        tempArray = (uint8_t *)Malloc(totalLength);
        if(tempArray == NULL)
        {   return ERR_ALLOC_FAILED;  }
        memset(tempArray, 0, totalLength);
        
        /* 3.将数据读取出来并计算crc */
        if(unit->api->CallBack_HalRead((uint32_t)tempArray, unit->storeAddress, totalLength) != 0)
        {   return ERR_HAL_FAULT;  }
    }
    else
    {   tempArray = (void *)dataAddress;    }
    
    /* 4. 校验crc是否正确 */
    crc = Crc32(tempArray, unit->maxLength, 0xAABBCCDD);
    if(crc != *(uint32_t *)(tempArray + totalLength - 4))
    {   return ERR_VERIFICATION;  }
    
    if(dataAddress != unit->storeAddress)
    {
        /* 5. 恢复数据 */
        memcpy((void *)dataAddress,
               tempArray,
               unit->maxLength);
        
        /* 释放内存 */
        Free(tempArray);
    }
    
    return ERR_SUCCESS;
}