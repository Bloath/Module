#ifndef _DATASTORE_H_
#define _DATASTORE_H_

/* Includes ----------------------------------------------------------*/
#include "Module/Module_Conf.h"
#include "DataStore_Conf.h"
/* define ------------------------------------------------------------*/
struct DataStoreUnitStruct
{
  uint32_t address;
  uint32_t length;
  uint32_t __storeAddress;
};    //存储数据单元结构体

/* typedef -----------------------------------------------------------*/
/* macro -------------------------------------------------------------*/
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/
void DataStore_Init(struct DataStoreUnitStruct *dataStoreList, uint16_t unitCount);
int DataStoreUnit_Write(struct DataStoreUnitStruct *dataStoreUnit);
int DataStoreUnit_Restore(struct DataStoreUnitStruct *dataStoreUnit);
int DataStore_WriteAll(struct DataStoreUnitStruct *dataStoreList, uint16_t unitCount);
int DataStore_RestoreAll(struct DataStoreUnitStruct *dataStoreList, uint16_t unitCount);
#endif