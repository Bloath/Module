/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "math.h"
#include "StringHandle.h"
#include "string.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

IntSringTypeDef* IntConvertToString(uint32_t number)
{
  uint8_t digit = 0, ifGetFirstNumber = 0, counter = 0, temp;
  uint32_t tempUint32 = number;
  IntSringTypeDef *IntString = (IntSringTypeDef*)malloc(sizeof(IntSringTypeDef));
  
  /* 根据范围确定循环次数，减少系统压力 */
  if(tempUint32 < pow(10, 3))
  { digit = 3; }
  else if(tempUint32 < pow(10, 6))
  { digit = 6; }
  else
  { digit = 10; }
  
  /* 循环，找到第一位数，并循环计数 */
  for(int16_t i=digit; i>=0; i--)
  {
    temp = (uint8_t)(tempUint32 / pow(10, i));
    tempUint32 %= (uint32_t)pow(10, i);
    
    if(temp != 0 && ifGetFirstNumber == 0)
    { ifGetFirstNumber = 1; }
    
    if(ifGetFirstNumber)
    { IntString->string[counter++] = temp + 0x30; }
  }
  
  IntString->length = counter;

  
  return IntString;
}

/*********************************************************************************************

  * @brief  字符串分解为数字
  * @param  string：字符串指针
  * @param  stringLength：字符串长度
  * @return 转换好的浮点型
  * @remark 

  ********************************************************************************************/
float StringConvertToNumber(uint8_t *string, uint16_t stringLength)
{
  uint16_t i=0, point;
  float value = 0;
  
  /* 找到小数点位置 */
  for(i=0; i<stringLength; i++)
  {
    if(string[i] == '.')
    { break; }
  }
  
  point = i;
  
  /* 整数部分分解 */
  for(i=0; i<point; i++)
  {
    value += (string[i] - 0x30) * pow(10,(point - i - 1));
  }
  
  /* 小数部分分解 */ 
  for(i=(point + 1); i< stringLength; i++)
  { value += (string[i] - 0x30) * pow(0.1, (i - point)); }
  
  return value;
}
/*********************************************************************************************

  * @brief  找到目标索引
  * @param  string:  源字符串，可以直接写入字符串，例如“,”
  * @param  message:  目标数组 
  * @param  len:     目标字符串偏移量
  * @remark 

  ********************************************************************************************/
uint16_t StringFindIndex(uint8_t *string, uint8_t *message, uint16_t len)
{
  uint16_t i = 0, str_len = strlen((char const *)string);
  
  for(i=0; i<len-str_len; i++)
  {
    if(!StringSameJudge(string, message + i))
    { break; }
  }
  
  return (i != (len-str_len))?i:0xFFFF;
}

/*********************************************************************************************

  * @brief  判断目标字符串是否与源字符串相同
  * @param  src:  源字符串，可以直接写入字符串，例如“down”
  * @param  des:  目标数组 
  * @remark 

  ********************************************************************************************/
uint8_t StringSameJudge(uint8_t *src, uint8_t *des)
{
  uint16_t i = 0, length = strlen((char const *)src);
  
  for(i=0; i<length; i++)
  {
    if(src[i] != des[i])
    { break; }
  }
  if(i != length)
  { return 1; }
  else
  { return 0; }
}
/*********************************************************************************************

  * @brief  查看目标字符串中是否包含源字符串
  * @param  src:  源字符串，可以直接写入字符串，例如“down”
  * @param  des          目标数组 
  * @param  desLength:    目标字符串长度
  * @remark 

  ********************************************************************************************/
uint8_t StringInclude(uint8_t *src, uint8_t *des, uint16_t desLength)
{
  uint16_t i = 0, srcLengh = strlen((char const *)src);

  
  if(desLength  >= srcLengh)
  {
    for(i = 0 ;i<(desLength - srcLengh + 1); i++)
    {
      if(!StringSameJudge(src, des + i))
      { break; }
    }
    
    if(i == (desLength - srcLengh + 1))
    { return 1; }
    else
    { return 0; }
  }
  else
  { return 2; } 
  
}

/*********************************************************************************************

  * @brief  通过分隔符，将字符串分解成多个字符串并形成列表
  * @param  splitSting:  分隔符，可写多个
  * @param  desString:  目标数组 
  * @param  desLen：目标数组长度
  * @retval StringListStruct    字符串列表结构体指针
  * @remark 此函数的链表头，需要通过LinkList_WholeFree整体释放

  ********************************************************************************************/
//LinkListBlock* StringSplit(uint8_t *splitSting, uint8_t *des, uint16_t length)
//{
//  uint16_t splitStingLen = strlen((char const *)splitSting);
//  LinkListBlock* head = LinkList_New();    //创建链表头
//  
//  // 如果不包含分隔符，则直接返回空数组
//  if(StringInclude(splitSting, des, length))
//  { return head; }
//  
//  uint16_t i,start = 0, end = 0, counter = 0;
//  
//  /* 循环查找目标数组，并将最终结果添加到链表内 */
//  for(i=0; i<(length-splitStingLen); i++)
//  {
//      if(!StringSameJudge(splitSting, des, i))
//      {
//        end = i;
//        counter = LinkList_Append(head, des + start, end -start);
//        start = end + splitStingLen;
//      }
//  }
//  
//  /* 把最后一个分隔符后面的部分提取出来 */
//  counter = LinkList_Append(head, des + start, length - start);
//  
//  return head;
//}
