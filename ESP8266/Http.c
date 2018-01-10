/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "Array.h"
#include "SimpleBuffer.h"
#include "ESP8266.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define HTTP_MAX_LEN 200        //HTTP协议最大长度

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/*********************************************************************************************
 * @brief HTTP q请求头
 * @para： 报文内容
 * @return *httpPacket:返回http请求字符串指针
 * @remark 
  ********************************************************************************************/
char* Http_Request(char* string)
{  
  /* 申请内存 */
  char *httpPacket = (char*)malloc(HTTP_MAX_LEN);    
  memset(httpPacket, 0, HTTP_MAX_LEN);  
  
  /* 拼接HTTP协议 */
  strcat(httpPacket, "GET /?message=");
  strcat(httpPacket, string);
  strcat(httpPacket, " HTTP/1.1\r\nHost:");
  strcat(httpPacket, DOMAIN);
  strcat(httpPacket, "\r\n\r\n");
  
  return httpPacket;
}
/*********************************************************************************************
 * @brief HTTP 从接收数据中拿出数据端
 * @para： packet：数据包字符串
 * @return *字符串
 * @remark 
  ********************************************************************************************/
char* Http_GetResponse(char *packet)
{
  /* 查找HTTP中双换行位置 */
  char * index = strstr(packet, "HTTP");        //先找到HTTP
  index = strstr(index, "\r\n\r\n");           //再找双换行
  
  return (char*)(index + 4);
}