/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "../Module/Common/Malloc.h"
#include "ZcProtocol_Conf.h"
#include "Http.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*********************************************************************************************

  * @brief  字符串拼接
  * @param  src,源字符串
            str：后面拼接的字符串
  * @retval 
  * @remark 在sys_conf中，有切换是否使用标准库的宏定义

  ********************************************************************************************/
void Str_Concat(char *src, char *str)
{
#ifdef CUSTOM_STANDARD_FUNCTION
  StrCat(src, str);
#else
  strcat(src, str);
#endif
}

/*********************************************************************************************
 * @brief HTTP q请求头
 * @para： 报文内容
 * @return *httpPacket:返回http请求字符串指针
 * @remark 
  ********************************************************************************************/
char* Http_Request(char* string)
{  
  /* 申请内存 */
  char *httpPacket = (char*)Malloc(HTTP_MAX_LEN);    
  memset(httpPacket, 0, HTTP_MAX_LEN);  
  
  /* 拼接HTTP协议 */
  Str_Concat(httpPacket, "GET ");
  Str_Concat(httpPacket, PATH);
  Str_Concat(httpPacket, "?message=");
  Str_Concat(httpPacket, string);
  Str_Concat(httpPacket, " HTTP/1.1\r\nHost:");
#ifdef DOMAIN
  Str_Concat(httpPacket, DOMAIN);
#else
  Str_Concat(httpPacket, IP);
  Str_Concat(httpPacket, ":");
  Str_Concat(httpPacket, PORT);
#endif
  Str_Concat(httpPacket, "\r\n\r\n");
  
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
  
  return (index == NULL)? (char*)index:(char*)(index + 4);
}