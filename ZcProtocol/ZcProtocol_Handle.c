/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "ZcProtocol.h"

//在此下面是针对不同处理环境添加的头
#include "../ESP8266/ESP8266.h"
#include "../UartDma/SimpleBuffer.h"
#include "../Sys_Conf.h"
#include "Http.h"
#include "ZcProtocol_Handle.h"

/* private typedef ------------------------------------------------------------*/
/* private define -------------------------------------------------------------*/
/* private macro --------------------------------------------------------------*/
/* private variables ----------------------------------------------------------*/
ZcProtocol zcPrtc;      // 拙诚协议实例
ZcHandleStruct zcHandle = {0};      //拙诚协议处理
extern TxBlockTypeDef Enthernet_TxBlockList[TX_BLOCK_COUNT];     //网络协议发送缓冲
extern RxBlockTypeDef Enthernet_RxBlockList[RX_BLOCK_COUNT];     //网络协议接收缓冲

/* private function prototypes ------------------------------------------------*/
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
    time = sysTime;                     //记录此时的系统时间
    return 0;                           //返回0
  }
  else
  { return zcPrtc.head.timestamp + (sysTime - time) / 1000; } //获取时间戳，则是在之前记录的时间戳的基础上，加上后面跑过的系统时间
    
}

/*********************************************************************************************

  * @brief  拙诚协议 网络通讯发送部分
  * @param  cmd: 命令字
            data:  报文指针
            dataLen：报文长度
            isUpdateId：是否需要更新ID
  * @retval 
  * @remark 通过输入命令以及数据，并填写到发送缓冲当中

  ********************************************************************************************/
void ZcProtocol_NetTransmit(uint8_t cmd, uint8_t *data, uint16_t dataLen, uint8_t isUpdateId)
{
  zcPrtc.head.timestamp = ZcProtocol_TimeStamp(0);      //更新时间戳
  zcPrtc.head.cmd = cmd;
  
  char* httpMsg = ZcProtocol_ConvertHttpString(&zcPrtc, data, dataLen);         //转换为HTTP协议，
  FillTxBlock(Enthernet_TxBlockList, (uint8_t*)httpMsg, strlen(httpMsg), TX_FLAG_MC|TX_FLAG_RT);        //填写到网络发送缓冲当中
  free(httpMsg);
  
  /* 更新ID */
  if(isUpdateId)
  { zcPrtc.head.id ++; }        
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
  
  /* 间隔一段时间 */
  if((time + zcHandle.loopInterval) > sysTime)
  { return; }
  else
  { time = sysTime; }
  
  /* 根据主状态进行处理 */
  switch(zcHandle.status)
  {
    /* 初始化 */
  case ZcHandleStatus_Init:
    zcHandle.loopInterval = 60000;
    zcHandle.status = ZcHandleStatus_Idle;
    break;
    
    /* 空闲状态，填充查询暂存报文，切换为等待状态 */
  case ZcHandleStatus_Idle:
    ZcProtocol_NetTransmit(00, NULL, 0, 0);     //发送暂存报文
    zcHandle.holdId = zcPrtc.head.id;           //记录当前ID
    zcHandle.status = ZcHandleStatus_Wait;      //切换等待状态
    break;
    
    /* 等待状态，发送缓冲为手动清除，如果没有回复的话是不会清除的 
       等待状态为死锁，直到接收到确认报文或者查询暂存回复*/
  case ZcHandleStatus_Wait:
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
void ZcProtocol_ReceiveHandle(uint8_t *message, uint16_t length)
{
  ZcProtocol *protocol = ZcProtocol_Check(message, length);     //检查接收的回复是否准确
  
  if(protocol != NULL)
  {
    ZcProtocol_TimeStamp(protocol->head.timestamp);             //每次都更新时间戳
    ClearSpecifyBlock(Enthernet_TxBlockList, ZcProtocol_SameId, &protocol->head.id);    //每次服务器回复后都要清除相应报文
    
    switch(protocol->head.cmd)                          
    { 
    /* 查询暂存报文 */
    case ZC_CMD_QUERY_HOLD:
      zcHandle.status = ZcHandleStatus_Idle;  // 切换为空闲状态，继续发送查询暂存报文
      break;
      
    /* 服务器下发确认报文 */
    case ZC_CMD_SERVER_CONFIRM:
      
      /* 当接收到确认报文且与之前的暂存报文ID相同 */
      if(protocol->head.id == zcHandle.holdId)
      { 
        zcHandle.status = ZcHandleStatus_Idle;  // 切换为空闲状态，发送下一次查询暂存报文
        zcPrtc.head.id++;                       // Id递增
      }
      break;
    
    /* 地址域 */
    case ZC_CMD_ADDRESS:
      ZcProtocol_NetTransmit(ZC_CMD_ADDRESS, zcPrtc.head.address, 7, 0);
      break;
    
//    /* 设备相关属性 */
//    case ZC_CMD_DEVICE_ATTR:
//      
//      break;
//      
//    /* 计量相关属性 */
//    case ZC_CMD_MEASURE_ATTR:
//      
//      break;
//    
//    /* 阶梯费用 */
//    case ZC_CMD_LADIR_PRICE:
//      
//      break;
//    
//    /* 充值记录 */
//    case ZC_CMD_RECHARGE:
//      
//      break;
//    
//    /* 用气历史记录 */
//    case ZC_CMD_USE_HISTORY:
//      
//      break;
//      
//    /* 报警信息 */
//    case ZC_CMD_ALARM:
//      
//      break;
//      
//    /* 环境参数 */
//    case ZC_CMD_ENVIROMENT:
//      
//      break;
//      
//    /* 管道状态 */
//    case ZC_CMD_PIPE_STATUS:
//      
//      break;
//      
//    /* 阀门开关记录 */
//    case ZC_CMD_VALVE_RECORD:
//      
//      break;
//      
//    /* 阀门开关操作 */
//    case ZC_CMD_VALVE_OPRT:
//      
//      break;
      
    /* 回复的命令 */
    default:
      ZcProtocol_NetTransmit(ZC_CMD_FAIL, NULL, 0, 0);
      break;
    }
    
    
  }
}

/*********************************************************************************************

  * @brief  拙诚协议=》网络协议处理
  * @param  message：http协议回复数据包内的回复内容部分字符串
            length：报文长度
  * @retval 
  * @remark 

  ********************************************************************************************/
void ZcProtocol_NetReceiveHandle(uint8_t *message, uint16_t length)
{
  ArrayStruct *msg = String2Msg((char*)message);                // 字符串转报文数组
  
  ZcProtocol_ReceiveHandle(msg->packet, msg->length);           //协议的原始报文处理
  
  Array_Free(msg);      // 释放报文数组空闲
}
