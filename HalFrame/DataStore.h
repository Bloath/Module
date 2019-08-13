#ifndef _DATASTORE_H_
#define _DATASTORE_H_

/* Includes ----------------------------------------------------------*/
#include "../Module_Conf.h"
#include "DataStore_Conf.h"
/* define ------------------------------------------------------------*/
typedef struct
{
  uint32_t address;
  uint32_t length;
  uint32_t __storeAddress;
}DataStoreUnitStruct;    //存储数据单元结构体

/* typedef -----------------------------------------------------------*/
/* macro -------------------------------------------------------------*/
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/
void DataStore_Init(DataStoreUnitStruct *dataStoreList, uint16_t unitCount);
int DataStoreUnit_Write(DataStoreUnitStruct *dataStoreUnit);
int DataStoreUnit_Restore(DataStoreUnitStruct *dataStoreUnit);
int DataStore_WriteAll(DataStoreUnitStruct *dataStoreList, uint16_t unitCount);
int DataStore_RestoreAll(DataStoreUnitStruct *dataStoreList, uint16_t unitCount);
#endif