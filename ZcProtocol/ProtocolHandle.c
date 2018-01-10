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
void ZcProtocol_ReponseHandle(uint8_t *message, uint16_t length)
{
  ArrayStruct *msg = String2Msg((char*)message);            // 字符串转报文
  
  ZcProtocol *protocol = ZcProtocol_Check(msg->packet, msg->length);
  
  if(protocol != NULL)
  {
    ClearSpecifyBlock(esp8266_TxBlockList, ZcProtocol_SameId, &protocol->head.id);      //清除ID相同的发送报文
  }
  
  Array_Free(msg);
}
/*********************************************************************************************

  * @brief  拙诚协议转字符串，填写到缓冲中
  * @param  zcProtocol:  拙诚协议实例
            txBlock：要填入的发送缓冲名称
            data：数据域指针
            dataLen：数据域长度
  * @retval 报文数组结构体指针
  * @remark 

  ********************************************************************************************/
void ZcProtocol_ConvertHttpMsg(ZcProtocol* zcProtocol, TxBlockTypeDef *txBlock, uint8_t *data, uint16_t dataLen)
{
  ArrayStruct *msg = ZcProtocol_ConvertMsg(zcProtocol, data ,dataLen);      // 首先转换成报文
  
  char *msgString = Msg2String(msg->packet, msg->length);       // 再转换为字符串
  Array_Free(msg);
  
  char *httpMsg = Http_Request(msgString);                    // 再转换成Http包
  free(msgString);
  
  FillTxBlock(txBlock, (uint8_t*)httpMsg, strlen(httpMsg),  TX_FLAG_RT|TX_FLAG_MC);       // 填充到发送缓冲当中
  free(httpMsg);
}
