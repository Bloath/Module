/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "Array.h"
#include "ZcProtocol.h"
#include "Convert.h"
#include "Http.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
ZcProtocol zcPrtc;

/* Private function prototypes ------------------------------------------------*/
uint8_t ZcProtocol_GetCrc(uint8_t *message, uint16_t length);

/*********************************************************************************************

  * @brief  协议转报文
  * @param  protocol:  协议结构体指针
  * @retval 报文数组结构体指针
  * @remark 

  ********************************************************************************************/
ArrayStruct* ZcProtocol_ConvertMsg(ZcProtocol* zcProtocol, uint8_t *message, uint16_t length)
{
  zcProtocol->head.length = ZC_UNDATA_LEN + length;
  
  ArrayStruct *msg = Array_New(zcProtocol->head.length);     // 协议头中指针占4个字节，结尾占两个字节

  *(ZcProtocolHead*)(msg->packet) = zcProtocol->head;        // 复制协议头
  
  memcpy(msg->packet + ZC_HEAD_LEN, message, length);      // 复制数据

  msg->packet[msg->length - 2] = ZcProtocol_GetCrc(msg->packet, msg->length);     // 填充CRC
  msg->packet[msg->length - 1] = ZC_END;                //填充结束字符
  
  return msg;
}
/*********************************************************************************************

  * @brief  拙诚协议转HTTP协议
  * @param  zcProtocol:  拙诚协议实例
            data：数据域指针
            dataLen：数据域长度
  * @retval 返回数据包字符串指针
  * @remark 字符串最终要返回

  ********************************************************************************************/
char* ZcProtocol_ConvertHttpString(ZcProtocol* zcProtocol, uint8_t *data, uint16_t dataLen)
{
  ArrayStruct *msg = ZcProtocol_ConvertMsg(zcProtocol, data ,dataLen);      // 首先转换成报文，二进制数据
  
  char *msgString = Msg2String(msg->packet, msg->length);       // 再转换为字符串，长度为原始报文两倍
  Array_Free(msg);
  
  char *httpMsg = Http_Request(msgString);                    // 再转换成Http包，
  free(msgString);
  
  return httpMsg;
}

/*********************************************************************************************

  * @brief  协议初始化
  * @param  
  * @retval 
  * @remark 设置一些参数，设定地址

  ********************************************************************************************/
ZcProtocol* ZcProtocol_Check(uint8_t *message, uint16_t length)
{
  if(length < ZC_UNDATA_LEN)
  { return NULL; }
  
  ZcProtocol *protocol = (ZcProtocol*)message;  // 获取协议结构指针
  ZcProtocolEnd *tail = (ZcProtocolEnd*)(message + length - 2);
  
  /* 首尾判断 */
  if(protocol->head.head != ZC_HEAD || tail->end != ZC_END)
  {  return NULL;  }
  
  /* CRC判断 */
  if(tail->crc != ZcProtocol_GetCrc(message, length))
  { return NULL;  }

  return protocol;
}
/*********************************************************************************************

  * @brief  判断协议ID是否相同
  * @param  message:  报文指针
            length：报文长度
  * @retval 相同返回0，不同返回1
  * @remark 用于发送缓冲的清除

  ********************************************************************************************/
uint8_t ZcProtocol_SameId(uint8_t *message, uint16_t length, void *p)
{
  uint8_t id = 0, position=42;
  id |= ((message[position] > 0x40)? (message[position] - 0x41 + 10): (message[position] - 0x30))<<4;
  id |= (message[position + 1] > 0x40)? (message[position + 1] - 0x41 + 10): (message[position + 1] - 0x30);

  if(id == *((uint8_t *)p))
  { return 0; }
  
  return 1;
}


/*********************************************************************************************

  * @brief  协议获取CRC
  * @param  message:  报文指针
            length：报文长度
  * @retval 一个字节的crc
  * @remark 

  ********************************************************************************************/
uint8_t ZcProtocol_GetCrc(uint8_t *message, uint16_t length)
{
  uint8_t crc = 0;
  for(uint16_t i=0; i<(length - 2); i++)             //填充CRC
  { crc += message[i]; }

  return crc;
}
