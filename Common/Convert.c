/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "Array.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  二进制转字符
  * @param  x：二进制byte
  * @remark 单字节转换

  ********************************************************************************************/
char Hex2Char(uint8_t x)
{
  x &= 0x0F;
  return (x > 9)? (0x41 + x - 10): (0x30 + x);
}
/*********************************************************************************************

  * @brief  字符转二进制
  * @param  x：字符
  * @remark 单字节转换

  ********************************************************************************************/
uint8_t Char2Hex(char x)
{
  return (x > 0x40)? (x - 0x41 + 10): (x - 0x30);
}

/*********************************************************************************************

  * @brief  字符串转报文
  * @param  string：字符串
  * @remark 转换完成的报文数组

  ********************************************************************************************/
ArrayStruct* String2Msg(char* string)
{
  uint16_t length = strlen(string);
  ArrayStruct* msg = Array_New(length / 2);
  
  for(uint16_t i=0; i<(length / 2); i++)
  {
    msg->packet[i] = 0;
    msg->packet[i] |= Char2Hex(string[i * 2]) << 4;
    msg->packet[i] |= Char2Hex(string[i * 2 + 1]);
  }
  
  return msg;
}
/*********************************************************************************************

  * @brief  报文转字符串
  * @param  message：报文指针
            length：长度
  * @retval 转换完成的字符串
  * @remark 

  ********************************************************************************************/
char* Msg2String(uint8_t *message, uint16_t length)
{
  char *string = (char*)malloc(length * 2 + 1);
  
  for(uint8_t i=0; i<length; i++)
  {
    string[i * 2] = Hex2Char(message[i] >> 4);
    string[i * 2 + 1] = Hex2Char(message[i] & 0x0F);
  }
  
  string[length * 2] = 0;  //结束符
  
  return string;
}
/*********************************************************************************************

  * @brief  uint转字符串
  * @param  number 数字
  * @retval 转换完成的字符串
  * @remark 

  ********************************************************************************************/
char* Uint2String(uint32_t number)
{
  uint8_t len = log10((double)number) + 1;
  uint32_t temp = 0;
  
  /* 申请内存准备放置字符串 */ 
  char *numString = (char*)malloc(len + 1);     // 申请对应内存
  memset(numString, 0, len + 1);        // 全部置零
  
  /* 字符串赋值 */
  temp = number;        // 缓存number
  for(uint8_t i=0; i<len; i++)
  {
    numString[i] = Hex2Char(temp / (int)pow(10, len - i - 1));  // 最高位开始转换
    temp = temp % (int)pow(10, len - i - 1);                    // 记录剩余值
  }
  
  return numString;
}