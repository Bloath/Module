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
  
  /* ���ݷ�Χȷ��ѭ������������ϵͳѹ�� */
  if(tempUint32 < pow(10, 3))
  { digit = 3; }
  else if(tempUint32 < pow(10, 6))
  { digit = 6; }
  else
  { digit = 10; }
  
  /* ѭ�����ҵ���һλ������ѭ������ */
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

  * @brief  �ַ����ֽ�Ϊ����
  * @param  string���ַ���ָ��
  * @param  stringLength���ַ�������
  * @return ת���õĸ�����
  * @remark 

  ********************************************************************************************/
float StringConvertToNumber(uint8_t *string, uint16_t stringLength)
{
  uint16_t i=0, point;
  float value = 0;
  
  /* �ҵ�С����λ�� */
  for(i=0; i<stringLength; i++)
  {
    if(string[i] == '.')
    { break; }
  }
  
  point = i;
  
  /* �������ַֽ� */
  for(i=0; i<point; i++)
  {
    value += (string[i] - 0x30) * pow(10,(point - i - 1));
  }
  
  /* С�����ַֽ� */ 
  for(i=(point + 1); i< stringLength; i++)
  { value += (string[i] - 0x30) * pow(0.1, (i - point)); }
  
  return value;
}
/*********************************************************************************************

  * @brief  �ҵ�Ŀ������
  * @param  string:  Դ�ַ���������ֱ��д���ַ��������硰,��
  * @param  message:  Ŀ������ 
  * @param  len:     Ŀ���ַ���ƫ����
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

  * @brief  �ж�Ŀ���ַ����Ƿ���Դ�ַ�����ͬ
  * @param  src:  Դ�ַ���������ֱ��д���ַ��������硰down��
  * @param  des:  Ŀ������ 
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

  * @brief  �鿴Ŀ���ַ������Ƿ����Դ�ַ���
  * @param  src:  Դ�ַ���������ֱ��д���ַ��������硰down��
  * @param  des          Ŀ������ 
  * @param  desLength:    Ŀ���ַ�������
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

  * @brief  ͨ���ָ��������ַ����ֽ�ɶ���ַ������γ��б�
  * @param  splitSting:  �ָ�������д���
  * @param  desString:  Ŀ������ 
  * @param  desLen��Ŀ�����鳤��
  * @retval StringListStruct    �ַ����б�ṹ��ָ��
  * @remark �˺���������ͷ����Ҫͨ��LinkList_WholeFree�����ͷ�

  ********************************************************************************************/
//LinkListBlock* StringSplit(uint8_t *splitSting, uint8_t *des, uint16_t length)
//{
//  uint16_t splitStingLen = strlen((char const *)splitSting);
//  LinkListBlock* head = LinkList_New();    //��������ͷ
//  
//  // ����������ָ�������ֱ�ӷ��ؿ�����
//  if(StringInclude(splitSting, des, length))
//  { return head; }
//  
//  uint16_t i,start = 0, end = 0, counter = 0;
//  
//  /* ѭ������Ŀ�����飬�������ս����ӵ������� */
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
//  /* �����һ���ָ�������Ĳ�����ȡ���� */
//  counter = LinkList_Append(head, des + start, length - start);
//  
//  return head;
//}
