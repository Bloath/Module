#ifndef _SIMPLEBUFFER_HANDLE_H_
#define _SIMPLEBUFFER_HANDLE_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "SimpleBuffer.h"

/* Public typedef ------------------------------------------------------------*/

typedef enum
{
  BlockFull = 0,
  
}BlockErrorEnum;

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void TxBlock_ErrorHandle(TxQueueStruct *rxQueue, BlockErrorEnum error);
void RxBlock_ErrorHandle(RxQueueStruct *rxQueue, BlockErrorEnum error);
void BufferOverFlow();

#endif