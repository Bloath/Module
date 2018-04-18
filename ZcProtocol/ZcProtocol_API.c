/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "ZcProtocol.h"

//在此下面是针对不同处理环境添加的头
#include "../Module/Common/Malloc.h"
#include "..//BufferQueue/BufferQueue.h"
#include "../Sys_Conf.h"
#include "Http.h"
#include "ZcProtocol_API.h"    
#include "ZcProtocol_Conf.h"   
#include "ZcProtocol_Handle.h"  //硬件相关处理

// 不同平台
#ifdef ZC_NET
  #include "../ESP8266/ESP8266_Handle.h"
#endif

#ifdef ZC_24G
  #include "../SPI_Chip/nRF24L01P/nRF24L01P_Handle.h"
#endif


/* private typedef ------------------------------------------------------------*/
/* private define -------------------------------------------------------------*/
/* private macro --------------------------------------------------------------*/
/* private variables ----------------------------------------------------------*/
ZcProtocol zcPrtc;      // 拙诚协议实例
/* private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  协议初始化
  * @param  DeviceType：设备类型
            address：设备地址指针
  * @retval 
  * @remark 在程序初始化时，需要将协议实例进行初始化

  ********************************************************************************************/
void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t* address, uint8_t startId)
{
  zcPrtc.head.head = 0x68;
  zcPrtc.head.control = DeviceType;
  zcPrtc.head.id = startId;                           // 0是预留给无线设备请求
  zcPrtc.head.timestamp = 0;
  
  memcpy(zcPrtc.head.address, address, 7);      //复制7字节地址，在产品实际运用后，地址是不会改变的（跟微信挂钩）
}

/*********************************************************************************************

  * @brief  获取/更新时间戳
  * @param  timeStamp：用于时间戳的更新，不为0则更新协议实例的时间戳，为0则返回当前时间戳
  * @retval 如果为设置，则返回0，如果非设置，则返回当前时间戳（单片机内部更新）
  * @remark 

  ********************************************************************************************/
uint32_t ZcProtocol_TimeStamp(uint32_t timeStamp)
{  
  static uint32_t time = 0;    //用于记录上次的系统时间
  
  /* 当参数不为0时，则为更新时间戳 */
  /* 当参数为0时，则为获取时间戳 */
  if(timeStamp == 0)
  { return zcPrtc.head.timestamp + realTime - time; } //获取时间戳，则是在之前记录的时间戳的基础上，加上后面跑过的系统时间 
  else if(timeStamp > 1514736000)       // 2018年1月1日， 0:0:0
  { 
    zcPrtc.head.timestamp = timeStamp;  //更新时间戳
    time = realTime;                     //记录此时的系统时间
  }
   return 0;
}
/*********************************************************************************************

  * @brief  拙诚协议=》协议实例ID自增
  * @param  
  * @retval 
  * @remark 0预留给其他设备

  ********************************************************************************************/
void ZcProtocol_HeadIdIncrease()
{
  zcPrtc.head.id ++;
  if(zcPrtc.head.id == 0)
  { zcPrtc.head.id = 1; }
}
/*********************************************************************************************

  * @brief  拙诚协议 发送请求
  * @param  source：源，网络，24G， 485等
            cmd: 命令字
            data:  报文指针
            dataLen：报文长度
            isUpdateId：是否需要更新ID
            txMode：发送模式
  * @retval 该请求命令的id
  * @remark 通过输入命令以及数据，并填写到发送缓冲当中

  ********************************************************************************************/
uint8_t ZcProtocol_Request(ZcSourceEnum source, uint8_t cmd, uint8_t *data, uint16_t dataLen, BoolEnum isUpdateId, TxModeEnum txMode)
{
  char* httpMsg;
  ArrayStruct *msg;
  uint8_t temp8 = 0;
  
    /* 取一个新ID */
  if(isUpdateId == TRUE)
  { ZcProtocol_HeadIdIncrease(); }
  temp8 = zcPrtc.head.id;
  
  zcPrtc.head.timestamp = ZcProtocol_TimeStamp(0);      //更新时间戳
  zcPrtc.head.cmd = cmd;
  
  /* 根据不同的源，进行不同的发送处理 */
  switch(source)
  {
  case ZcSource_Net:
#ifdef ZC_NET
    httpMsg = ZcProtocol_ConvertHttpString(&zcPrtc, data, dataLen);         //转换为HTTP协议，
    TxQueue_AddWithId(&Enthernet_TxQueue, 
                      (uint8_t*)httpMsg, 
                      strlen(httpMsg), 
                      txMode,
                      zcPrtc.head.id);  
    Free(httpMsg);
#endif
    break;
  case ZcSource_24G:
#ifdef ZC_24G
    msg = ZcProtocol_ConvertMsg(&zcPrtc, data ,dataLen);
    TxQueue_AddWithId(&nRF24L01_TxQueue, 
                      msg->packet, 
                      msg->length, 
                      txMode,
                      zcPrtc.head.id);  
    Array_Free(msg);
#endif
    break;
    
  case ZcSource_485:
#ifdef ZC_485
    msg = ZcProtocol_ConvertMsg(&zcPrtc, data ,dataLen);
    TxQueue_AddWithId(&ZC_485_TXQUEUE, 
                      msg->packet, 
                      msg->length, 
                      txMode,
                      zcPrtc.head.id);  
    Array_Free(msg);
#endif
    break;
  }
  
  /* 取一个新ID */
  if(isUpdateId == TRUE)
  { ZcProtocol_HeadIdIncrease(); } 
  
  return temp8;
}
/*********************************************************************************************

  * @brief  拙诚协议 回复，在接收到命令后的回复
  * @param  source：源，网络，24G， 485等
            zcProtocol：接收包中解析出来的数据结构指针
            data:  报文指针
            dataLen：报文长度
  * @retval 
  * @remark 与请求不同，回复一般是用请求的ID、CMD等，仅仅是数据部分有区别

  ********************************************************************************************/
void ZcProtocol_Response(ZcSourceEnum source, ZcProtocol *zcProtocol, uint8_t *data, uint16_t dataLen)
{
  char* httpMsg;
  ArrayStruct *msg;
  
  /* 根据不同的源，进行不同的发送处理 */
  switch(source)
  {
  case ZcSource_Net:
#ifdef ZC_NET
    httpMsg = ZcProtocol_ConvertHttpString(zcProtocol, data, dataLen);         //转换为HTTP协议，
    TxQueue_AddWithId(&Enthernet_TxQueue, 
                      (uint8_t*)httpMsg, 
                      strlen(httpMsg), 
                      TX_MULTI_MC,
                      zcProtocol->head.id);  
    Free(httpMsg);
#endif
    break;
  case ZcSource_24G:
#ifdef ZC_24G
    msg = ZcProtocol_ConvertMsg(zcProtocol, data ,dataLen);
    TxQueue_AddWithId(&nRF24L01_TxQueue, 
                      msg->packet, 
                      msg->length, 
                      TX_ONCE_AC,
                      zcProtocol->head.id);  
    Array_Free(msg);
#endif
    break;
    
  case ZcSource_485:
#ifdef ZC_485
    msg = ZcProtocol_ConvertMsg(zcProtocol, data ,dataLen);
    TxQueue_AddWithId(&ZC_485_TXQUEUE, 
                      msg->packet, 
                      msg->length, 
                      TX_ONCE_AC,
                      zcProtocol->head.id);  
    Array_Free(msg);
#endif
    break;
  }
}

