#ifndef _DATASTORE_H_
#define _DATASTORE_H_

/* Includes ----------------------------------------------------------*/
#include "Module/Module_Conf.h"
#include "DataStore_Conf.h"
/* define ------------------------------------------------------------*/
struct DataStoreUnitStruct
{
  void* address;
  uint16_t size;
};    //存储数据单元结构体

struct DataStoreStruct
{
    struct DataStoreUnitStruct *dataStoreList;
    uint16_t count;
    uint16_t dataLength;
    uint32_t startAddress;
    
    int (*CallBack_HalWrite)(uint32_t dstAddress, uint32_t srcAddress, uint16_t length);
    int (*CallBack_HalRead)(uint32_t dstAddress, uint32_t srcAddress, uint16_t length);
};
/* typedef -----------------------------------------------------------*/
/* macro -------------------------------------------------------------*/
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/
int DataStore_Add(struct DataStoreStruct *dataStore, void* address, uint16_t size);
int DataStore_Write(struct DataStoreStruct *dataStore);
int DataStore_Restore(struct DataStoreStruct *dataStore);
#endif