/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "../Sys_Conf.h"
#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "../Common/Malloc.h"

#include "../BufferQueue/BufferQueue.h"

#include "Http.h"
#include "ZcProtocol.h"
#include "ZcProtocol_API.h"    
#include "ZcProtocol_Conf.h"   
#include "ZcProtocol_Handle.h"  //硬件相关处理


/* private typedef ------------------------------------------------------------*/
/* private define -------------------------------------------------------------*/
/* private macro --------------------------------------------------------------*/
/* private variables ----------------------------------------------------------*/
ZcProtocol zcPrtc;      // 拙诚协议实例
ZcErrorStruct zcError;
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
  * @param  communicate：通讯结构体对象
            cmd: 命令字
            data:  报文指针
            dataLen：报文长度
            isUpdateId：是否需要更新ID
            txMode：发送模式
  * @retval 该请求命令的id
  * @remark 通过输入命令以及数据，并填写到发送缓冲当中

  ********************************************************************************************/
uint8_t ZcProtocol_Request(CommunicateStruct *communicate, uint8_t cmd, uint8_t *data, uint16_t dataLen, BoolEnum isUpdateId, TxModeEnum txMode)
{
  char* httpMsg;
  ArrayStruct *msg;
  uint8_t temp8 = 0;
  
    /* 取一个新ID */
  if(isUpdateId == TRUE)
  { ZcProtocol_HeadIdIncrease(); }
  temp8 = zcPrtc.head.id;
  
  zcPrtc.head.timestamp = timeStamp;      //更新时间戳
  zcPrtc.head.cmd = cmd;
  
  /* 根据不同的源，进行不同的发送处理 */
  switch(communicate->medium)
  {
  // 网络通讯方式，需要添加GET方式的头，并且把HEX转换为BCD
  case COM_NET:
    httpMsg = ZcProtocol_ConvertHttpString(&zcPrtc, data, dataLen);         //转换为HTTP协议，
    TxQueue_AddWithId(communicate->txQueue, 
                      (uint8_t*)httpMsg, 
                      strlen(httpMsg), 
                      txMode,
                      zcPrtc.head.id);  
    Free(httpMsg);
    break;
    
  // 有线、无线通讯方式，不需要对协议进行修正添加等操作 
  case COM_24G:
  case COM_485:
    msg = ZcProtocol_ConvertMsg(&zcPrtc, data ,dataLen);
    TxQueue_AddWithId(communicate->txQueue, 
                      msg->packet, 
                      msg->length, 
                      txMode,
                      zcPrtc.head.id);  
    Array_Free(msg);
    break;
  }
  
  /* 取一个新ID */
  if(isUpdateId == TRUE)
  { ZcProtocol_HeadIdIncrease(); } 
  
  return temp8;
}
/*********************************************************************************************

  * @brief  拙诚协议 回复，在接收到命令后的回复
  * @param  communicate：通讯结构体对象
            zcProtocol：接收包中解析出来的数据结构指针
            data:  报文指针
            dataLen：报文长度
  * @retval 
  * @remark 与请求不同，回复一般是用请求的ID、CMD等，仅仅是数据部分有区别

  ********************************************************************************************/
void ZcProtocol_Response(CommunicateStruct *communicate, ZcProtocol *zcProtocol, uint8_t *data, uint16_t dataLen)
{
  char* httpMsg;
  ArrayStruct *msg;
  
  /* 根据不同的源，进行不同的发送处理 */
  switch(communicate->medium)
  {
  // 网络通讯方式，需要添加GET方式的头，并且把HEX转换为BCD
  case COM_NET:
    httpMsg = ZcProtocol_ConvertHttpString(zcProtocol, data, dataLen);         //转换为HTTP协议，
    TxQueue_AddWithId(communicate->txQueue, 
                      (uint8_t*)httpMsg, 
                      strlen(httpMsg), 
                      TX_MULTI_MC,
                      zcProtocol->head.id);  
    Free(httpMsg);
    break;
    
  // 有线、无线通讯方式，不需要对协议进行修正添加等操作   
  case COM_24G:
  case COM_485:
    msg = ZcProtocol_ConvertMsg(zcProtocol, data ,dataLen);
    TxQueue_AddWithId(communicate->txQueue, 
                      msg->packet, 
                      msg->length, 
                      TX_ONCE_AC,
                      zcProtocol->head.id);  
    Array_Free(msg);
    break;
  }
}

/*********************************************************************************************

  * @brief  错误处理
  * @param  communicate：通讯结构体对象
            txMode：发送模式
  * @retval 
  * @remark 当错误标志位与缓存不同时，则上传错误

  ********************************************************************************************/
void ZcError_Upload(CommunicateStruct *communicate, TxModeEnum txMode)
{
  if(zcError.flag != zcError.flagCache && timeStamp > 152000000)
  {
    /* 向服务器发送当前开关阀时间 */
    uint8_t *data = (uint8_t *)Malloc(8);
    *(uint32_t *)(data + 0) = zcError.flag;
    *(uint32_t *)(data + 4) = timeStamp;
    
    ZcProtocol_Request(communicate, ZC_CMD_ALARM, data, 8, TRUE, txMode);
    Free(data);  

    zcError.flagCache = zcError.flag;
  }
}

