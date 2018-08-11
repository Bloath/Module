#ifndef _COMMUNICATE_API_H_
#define _COMMUNICATE_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "../BufferQueue/BufferQueue.h"
/* Public typedef -----------------------------------------------------------*/
typedef enum
{
  COM_NET = 0,
  COM_485,
  COM_24G,
  COM_NB
}CommunicateMediumEnum;

typedef struct
{
  /* 默认项参数 */
  uint32_t time;                // 记录时间
  ProcessEnum process;          // 流程
  
  /* 手动设置项 */
  CommunicateMediumEnum medium; // 通讯介质
  BoolEnum isFullDuplex;        // 是否全双工
  BoolEnum isHDMaster;          // 半双工-主机 
  TxQueueStruct *txQueue;       // 发送缓冲，业务层 在进行业务处理后，将需要处理的业务直接填充到该缓冲中(不断循环)
  RxQueueStruct *rxQueue;       // 接收缓冲，业务层 RxQueue_Handle(communicate->rxQueue, Communicate_RxHandle, (void*)communicate); 
  uint16_t loopInterval;        // 轮询时间（半双工有效）
  uint32_t *refTime;            // 参考时间，为系统时间(sysTime)还是真实时间(realTime) ==（半双工有效）
  
  BoolEnum (*CallBack_TxFunc)(uint8_t*, uint16_t);                  // 业务层发送处理函数
}CommunicateStruct;

/* Public define ------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void Communicate_Handle(CommunicateStruct *communicate);

#endif 