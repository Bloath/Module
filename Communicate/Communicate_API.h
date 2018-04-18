#ifndef _COMMUNICATE_API_H_
#define _COMMUNICATE_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Module/Sys_Conf.h"
#include "../BufferQueue/BufferQueue.h"
/* Public typedef -----------------------------------------------------------*/
typedef struct
{
  /* 默认项参数 */
  uint32_t time;                // 记录时间
  ProcessEnum process;          // 流程
  
  /* 手动设置项 */
  uint8_t tag;                  // 标记
  BoolEnum isFullDuplex;        // 是否全双工
  BoolEnum isHDMaster;          // 半双工-主机 
  TxQueueStruct *txQueue;       // 发送缓冲
  RxQueueStruct *rxQueue;       // 接收缓冲
  uint16_t loopInterval;        // 轮询时间（半双工有效）
  uint32_t *refTime;            // 参考时间，为系统时间(sysTime)还是真实时间(realTime) ==（半双工有效）
  
  void (*CallBack_TxFunc)(uint8_t*, uint16_t);                  // 硬件发送函数
}CommunicateStruct;

/* Public define ------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void Communicate_Handle(CommunicateStruct *communicate);

#endif 