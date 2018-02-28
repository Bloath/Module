/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "../Module/Common/Malloc.h"
#include "ZcProtocol.h"
#include "Http.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
uint8_t ZcProtocol_GetCrc(uint8_t *message, uint16_t length);

/*********************************************************************************************

  * @brief  协议转报文
  * @param  protocol:  协议结构体指针
            
  * @retval 报文数组结构体指针
  * @remark 

  ********************************************************************************************/
ArrayStruct* ZcProtocol_ConvertMsg(ZcProtocol* zcProtocol, uint8_t *data, uint16_t dataLen)
{
  /* 确定数据长度，并申请相应内存 */
  if(data == NULL)
  { dataLen = 1; }
  zcProtocol->head.length = ZC_UNDATA_LEN + dataLen;
  ArrayStruct *msg = Array_New(zcProtocol->head.length);     

  /* 按照 头 、数据、结尾的顺序开始对报文进行赋值 */
  *(ZcProtocolHead*)(msg->packet) = zcProtocol->head;        // 复制协议头
  
  if(data == NULL)
  { msg->packet[ZC_HEAD_LEN] = 0; }
  else
  { memcpy(msg->packet + ZC_HEAD_LEN, data, dataLen); }          // 复制数据域

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
  Free(msgString);
  
  return httpMsg;
}

/*********************************************************************************************

  * @brief  协议检查，
  * @param  
  * @retval 返回协议指针
  * @remark 检查协议头、尾以及CRC等

  ********************************************************************************************/
ZcProtocol* ZcProtocol_Check(uint8_t *message, uint16_t length)
{
  if(length < ZC_UNDATA_LEN)
  { return NULL; }
  
  ZcProtocol *protocol = (ZcProtocol*)message;  // 获取协议结构指针
  ZcProtocolEnd *tail = (ZcProtocolEnd*)(message + protocol->head.length - 2);
  
  /* 首尾判断 */
  if(protocol->head.head != ZC_HEAD || tail->end != ZC_END)
  {     return NULL;  }
  
  /* CRC判断 */
  if(tail->crc != ZcProtocol_GetCrc(message, protocol->head.length))
  { return NULL;  }

  return protocol;
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
  
  crc ^= 0xFF;

  return crc;
}
