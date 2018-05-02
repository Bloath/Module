/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"

#include "../Module/Sys_Conf.h"
#include "../Module/BufferQueue/BufferQueue.h"
#include "../Module/Common/Array.h"
#include "Communicate_API.h"
#include "Communicate_Handle.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
void Communicate_RxHandle(uint8_t *message, uint16_t len, void *param);

/*********************************************************************************************

  * @brief  发送块错误处理
  * @param  communicate：半双工实例指针
  * @retval 无
  * @remark 

  ********************************************************************************************/
void Comunicate_HDMaster_Polling(CommunicateStruct *communicate)
{
  /* 根据主状态进行处理 */
  switch(communicate->process)
  {
  /* 初始化 */
  case Process_Init:
    communicate->process = Process_Idle;
    communicate->time = *communicate->refTime;
    break;
  
  /* 轮询间隔时间 */
  case Process_Idle:
     /* 间隔一段时间 */
    if((communicate->time + communicate->loopInterval) > *communicate->refTime)
    { break; }
    else
    { communicate->process = Process_Start; }
  
  /* 空闲状态，填充查询暂存报文，切换为等待状态 */
  case Process_Start:
    CallBack_HD_FillPollingPackt(communicate);     // 空闲数据包填充
    communicate->process = Process_Wait;        // 切换等待状态
    break;
    
  /* 等待状态，发送缓冲为手动清除，如果没有回复的话是不会清除的，否则会出现阻塞 
     等待状态为死锁，直到接收到确认报文或者查询暂存回复*/
  case Process_Wait:
     communicate->time = *communicate->refTime;
    break;
  }
}

/*********************************************************************************************

  * @brief  半双工通讯 主处理
  * @param  communicate：半双工处理
  * @retval 无
  * @remark 

  ********************************************************************************************/
void Communicate_Handle(CommunicateStruct *communicate)
{
  /* 半双工 + 主机模式，发送 */
  if(communicate->isFullDuplex == FALSE && communicate->isHDMaster == TRUE)
  { Comunicate_HDMaster_Polling(communicate);}
  
  /* 发送队列处理 */
  if(communicate->CallBack_TxFunc != NULL)
  { TxQueue_Handle(communicate->txQueue, communicate->CallBack_TxFunc); }
  
  RxQueue_Handle(communicate->rxQueue, Communicate_RxHandle, (void*)communicate); 
}

/*********************************************************************************************

  * @brief  半双工通讯，主机接收处理
  * @param  message:  发送块指针
            len：报文长度
            param：参数
  * @retval 
  * @remark 

  ********************************************************************************************/
void Communicate_RxHandle(uint8_t *message, uint16_t len, void *param)
{
  CommunicateStruct *communicate = (CommunicateStruct *)param;      // 指针获取
  
  ArrayStruct *msg = CallBack_MsgConvert(message, len, communicate);
  if(msg != NULL)
  { 
    message = msg->packet;
    len = msg->length;
    Array_Free(msg);
  }
  
  /* 报文检查 */
  if(CallBack_MessageInspect(message, len, communicate) == FALSE)
  { return; }
  
  /* 清除对应发送队列 */
  CallBack_ClearTxQueue(message, len, communicate);
  communicate->process = Process_Idle;
  
  if(communicate->isFullDuplex == FALSE)
  {
    /* 半双工情况下
       半双工-主模式：重新发送暂存
       半双工-从模式：则要填充数据 */
    if(CallBack_HD_isPooling(message, len, communicate) == TRUE )
    { 
      if(communicate->isHDMaster == FALSE)
      { CallBack_HDSlave_PollHandle(message, len, communicate); }
      
      return;
    }
  }
  
  CallBack_UnpollMsgHandle(message, len, communicate);     // 非轮询报文处理
}

