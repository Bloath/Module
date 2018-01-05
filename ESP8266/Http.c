/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "Base.h"
#include "StringHandle.h"
#include "SimpleBuffer.h"
#include "ESP8266.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
char *httpHeader[4] = { "GET /?message=", " HTTP/1.1\r\nHost:", DOMAIN, "\r\n\r\n"};

/* Private function prototypes -----------------------------------------------*/
/*********************************************************************************************
 * @brief HTTP q请求头
 * @para： packet：数据包
           length：数据包长度
 * @return *httpMessage:返回http请求头
 * @remark 
  ********************************************************************************************/
ArrayStruct* Http_Request(uint8_t *packet, uint16_t length)
{  
  uint16_t counter = 0;
  
  for(uint16_t i=0; i<4; i++)
  { counter += strlen(httpHeader[i]); }
  
  ArrayStruct* httpMessage = Array_New(counter + length);      // 
  uint8_t *p = httpMessage->packet;
  
  memcpy(p, httpHeader[0], strlen(httpHeader[0]));
  p += strlen(httpHeader[0]);
  
  memcpy(p, packet, length);
  p += length;
  
  for(uint16_t i=0; i<3; i++)
  {
    memcpy(p, httpHeader[1 + i], strlen(httpHeader[1 + i]));
    p += strlen(httpHeader[1 + i]);
  }
    
  return httpMessage;
}
/*********************************************************************************************
 * @brief HTTP 从接收数据中拿出数据端
 * @para： packet：数据包
           length：数据包长度
 * @return *array
 * @remark 
  ********************************************************************************************/
ArrayStruct* Http_GetMessage(uint8_t *packet, uint16_t len)
{
  uint16_t index = StringFindIndex("\r\n\r\n", packet, len);
  
  ArrayStruct* array = NULL;
  
  if(index != 0xFFFF)
  { 
    array = Array_New(len - index - 4); 
    CopyPacket(packet + index + 4, array->packet, array->length);
  }
 
  return array;
}