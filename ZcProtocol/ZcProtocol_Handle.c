/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "ZcProtocol.h"
#include "Array.h"
#include "Convert.h"

//在此下面是针对不同处理环境添加的头
#include "ESP8266.h"
#include "Http.h"
#include "SimpleBuffer.h"

/* private typedef ------------------------------------------------------------*/
/* private define -------------------------------------------------------------*/
/* private macro --------------------------------------------------------------*/
/* private variables ----------------------------------------------------------*/
/* private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  协议初始化
  * @param  
  * @retval 
  * @remark 设置一些参数，设定地址

  ********************************************************************************************/
void ZcProtocol_Init()
{
  zcPrtc.head.head = 0x68;
  zcPrtc.head.control = 0xB0;
  zcPrtc.head.info = 0x0140;
  zcPrtc.head.afn = 0x03;
  zcPrtc.head.id = 0x65;
  
  uint8_t address[7] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11};
  memcpy(zcPrtc.head.address, address, 7);
  
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
  ZcProtocol *protocol = ZcProtocol_Check(message, length);
  
  if(protocol != NULL)
  {
    ClearSpecifyBlock(esp8266_TxBlockList, ZcProtocol_SameId, &protocol->head.id);      //清除ID相同的发送报文
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
