/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "ZcProtocol.h"

//在此下面是针对不同处理环境添加的头
#include "../Module/Common/Malloc.h"
#include "../UartDma/SimpleBuffer.h"
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
ZcHandleStruct zcHandle = {0};      //拙诚协议处理

/* private function prototypes ------------------------------------------------*/
void ZcProtocol_NetRxHandle(ZcProtocol *zcProtocol);
void ZcProtocol_24GRxHandle(ZcProtocol *zcProtocol);

/*********************************************************************************************

  * @brief  协议初始化
  * @param  DeviceType：设备类型
            address：设备地址指针
  * @retval 
  * @remark 在程序初始化时，需要将协议实例进行初始化

  ********************************************************************************************/
void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t* address)
{
  zcPrtc.head.head = 0x68;
  zcPrtc.head.control = DeviceType;
  zcPrtc.head.id = 0;
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
  if(timeStamp != 0)
  { 
    zcPrtc.head.timestamp = timeStamp;  //更新时间戳
    time = realTime;                     //记录此时的系统时间
    return 0;                           //返回0
  }
  else
  { return zcPrtc.head.timestamp + realTime - time; } //获取时间戳，则是在之前记录的时间戳的基础上，加上后面跑过的系统时间 
}
/*********************************************************************************************

  * @brief  拙诚协议 发送请求
  * @param  source：源，网络，24G， 485等
            cmd: 命令字
            data:  报文指针
            dataLen：报文长度
            isUpdateId：是否需要更新ID
  * @retval 
  * @remark 通过输入命令以及数据，并填写到发送缓冲当中

  ********************************************************************************************/
void ZcProtocol_Request(ZcSourceEnum source, uint8_t cmd, uint8_t *data, uint16_t dataLen, BoolEnum isUpdateId)
{
  char* httpMsg;
  ArrayStruct *msg;
  
    /* 取一个新ID */
  if(isUpdateId == TRUE)
  { zcPrtc.head.id ++; }  
  
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
                      TX_FLAG_MC|TX_FLAG_RT,
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
                      TX_FLAG_MC|TX_FLAG_RT,
                      zcPrtc.head.id);  
    Array_Free(msg);
#endif
    break;
  }
  
  /* 取一个新ID */
  if(isUpdateId == TRUE)
  { zcPrtc.head.id ++; } 
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
                      TX_FLAG_MC|TX_FLAG_RT,
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
                      0,
                      zcProtocol->head.id);  
    Array_Free(msg);
#endif
    break;
  }
}
/*********************************************************************************************

  * @brief  接收协议报文的处理
  * @param  
  * @retval 报文数组结构体指针
  * @remark 

  ********************************************************************************************/
void ZcProtocol_Handle()
{
  static uint32_t time=0;
  
  /* 根据主状态进行处理 */
  switch(zcHandle.status)
  {
  /* 初始化 */
  case ZcHandleStatus_Init:
    zcHandle.loopInterval = ZC_NET_POLL_INTERVAL;
    zcHandle.status = ZcHandleStatus_PollInterval;
    time = realTime;
    break;
  
  /* 轮训间隔时间 */
  case ZcHandleStatus_PollInterval:
     /* 间隔一段时间 */
    if((time + zcHandle.loopInterval) > realTime)
    { break; }
    else
    { zcHandle.status = ZcHandleStatus_Trans; }
  
  /* 空闲状态，填充查询暂存报文，切换为等待状态 */
  case ZcHandleStatus_Trans:
    ZcProtocol_Request(ZcSource_Net, 00, NULL, 0, FALSE);     //发送暂存报文
    zcHandle.status = ZcHandleStatus_Wait;      //切换等待状态
    break;
    
  /* 等待状态，发送缓冲为手动清除，如果没有回复的话是不会清除的 
     等待状态为死锁，直到接收到确认报文或者查询暂存回复*/
  case ZcHandleStatus_Wait:
    time = realTime;
    break;
  }
}
/*********************************************************************************************

  * @brief  接收协议报文的处理
  * @param  message:  报文指针
            length：报文长度
  * @retval 报文数组结构体指针
  * @remark 

  ********************************************************************************************/
void ZcProtocol_ReceiveHandle(uint8_t *message, uint16_t length, ZcSourceEnum source)
{
  ZcProtocol *zcProtocol = ZcProtocol_Check(message, length);     //检查接收的回复是否准确
  
  if(zcProtocol != NULL)
  {
    ZcProtocol_TimeStamp(zcProtocol->head.timestamp);             //每次都更新时间戳
    
    /* 根据不同的来源进行不同的处理 */
    switch(source)
    {
    case ZcSource_Net:
      ZcProtocol_NetRxHandle(zcProtocol);                   // 网络版通讯协议，关键点在于由设备发起，服务器单纯响应
      break;
    case ZcSource_24G:
      ZcProtocol_24GRxHandle(zcProtocol);                   // 2.4G协议
      break;
    }
  }
}
/*********************************************************************************************

  * @brief  接收协议报文的处理
  * @param  message:  报文指针
            length：报文长度
  * @retval 报文数组结构体指针
  * @remark 

  ********************************************************************************************/
void ZcProtocol_NetRxHandle(ZcProtocol *zcProtocol)
{
#ifdef ZC_NET
  TxQueue_FreeById(&Enthernet_TxQueue, zcProtocol->head.id);    //每次服务器回复后都要清除相应发送报文

    /* 先查看是否有操作的报文
     回复0：处理成功，不需要后续处理
     回复1：非处理命令，进入通讯类报文处理
     回复2：则处理失败，直接发送失败恢复命令 */
  uint8_t operationRes = ZcProtocol_NetOperationCmdHandle(zcProtocol);        //先查看是否有操作的报文

  if(operationRes == 1)
  {
    switch(zcProtocol->head.cmd)                          
    { 
    /* 查询暂存报文 */
    case ZC_CMD_QUERY_HOLD:
      zcHandle.status = ZcHandleStatus_Init;  // 切换为初始化状态，根据预定轮训时间进行轮训
      break;
      
    /* 服务器下发确认报文 */
    case ZC_CMD_SERVER_CONFIRM:
      zcPrtc.head.id++;                       // Id递增
           
      /* 当控制欲的SFD为1时，直接发送查询暂存报文,并切换为等待状态*/
      if((zcProtocol->head.control & (1<<7)) != 0)
      { zcHandle.status = ZcHandleStatus_Trans; }       // 直接切换为发送状态   
      else
      { zcHandle.status = ZcHandleStatus_Init; }        // 切换为初始化状态，根据预定轮训时间进行轮训
      break;
      
    /* 回复的命令 */
    default:
      ZcProtocol_Request(ZcSource_Net, ZC_CMD_FAIL, NULL, 0, FALSE);
      break;
    }
  }
  else if(operationRes == 2)
  { ZcProtocol_Request(ZcSource_Net, ZC_CMD_FAIL, NULL, 0, FALSE); }        //操作类指令失败，发送失败回复
#endif
}
/*********************************************************************************************

  * @brief  接收协议报文的处理
  * @param  message:  报文指针
            length：报文长度
  * @retval 报文数组结构体指针
  * @remark 

  ********************************************************************************************/
void ZcProtocol_24GRxHandle(ZcProtocol *zcProtocol)
{
#ifdef ZC_24G
  
  TxQueue_FreeById(&nRF24L01_TxQueue, zcProtocol->head.id);    //每次服务器回复后都要清除相应发送报文
  ZcProtocol_24GOperationCmdHandle(zcProtocol);
  
#endif
}
/*********************************************************************************************

  * @brief  拙诚协议=》网络协议处理
  * @param  message：http协议回复数据包内的回复内容部分字符串
            length：报文长度
  * @retval 
  * @remark 

  ********************************************************************************************/
void ZcProtocol_NetPacketHandle(uint8_t *message, uint16_t length)
{
  ArrayStruct *msg = String2Msg(strstr((const char*)message, "68"), 0);      // 字符串转报文数组
  
  ZcProtocol_ReceiveHandle(msg->packet, msg->length, ZcSource_Net);  //协议的原始报文处理
  
  Array_Free(msg);      // 释放报文数组空闲
}
