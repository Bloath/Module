#ifndef _DATASTORE_H_
#define _DATASTORE_H_

/* Includes ----------------------------------------------------------*/
#include "Module/Module_Conf.h"
/* define ------------------------------------------------------------*/
struct DataStoreApiStruct
{
    int (*CallBack_HalWrite)(uint32_t dstAddress, uint32_t srcAddress, uint16_t length);
    int (*CallBack_HalRead)(uint32_t dstAddress, uint32_t srcAddress, uint16_t length);
};

struct DataStoreUnitStruct
{
    uint32_t storeAddress;
    uint16_t maxLength;
    
    struct DataStoreApiStruct *api;
};

/* typedef -----------------------------------------------------------*/
/* macro -------------------------------------------------------------*/
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/
int DataStore_Write(struct DataStoreUnitStruct *unit, uint32_t dataAddress, uint16_t writeLength);
int DataStore_Restore(struct DataStoreUnitStruct *unit, uint32_t dataAddress, uint16_t readLength);

#endif