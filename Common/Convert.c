/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "../Common/Malloc.h"
#include "Array.h"
#include "Convert.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  ������ת�ַ�
  * @param  x��������byte
  * @remark ���ֽ�ת��

  ********************************************************************************************/
char Hex2Char(uint8_t x)
{
  x &= 0x0F;
  return (x > 9)? (0x41 + x - 10): (0x30 + x);
}
/*********************************************************************************************

  * @brief  �ַ�ת������
  * @param  x���ַ�
  * @remark ���ֽ�ת��

  ********************************************************************************************/
uint8_t Char2Hex(char x)
{
  return (x > 0x40)? (x - 0x41 + 10): (x - 0x30);
}
/*********************************************************************************************

  * @brief  10��N�η�����
  * @param  x���ַ�
  * @remark ���ֽ�ת��

  ********************************************************************************************/
uint32_t Math_10nthPower(uint8_t nth)
{
  return (uint32_t)(pow(10, nth) + 0.2);
}
/*********************************************************************************************

  * @brief  �ַ���ת����
  * @param  string���ַ���
  * @remark ת����ɵı�������

  ********************************************************************************************/
ArrayStruct* String2Msg(char* string, uint16_t specifyLen)
{
  uint16_t length = (specifyLen == 0)? strlen(string): specifyLen;
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

  * @brief  ����ת�ַ���
  * @param  message������ָ��
            length������
  * @retval ת����ɵ��ַ���
  * @remark 

  ********************************************************************************************/
char* Msg2String(uint8_t *message, uint16_t length)
{
  char *string = (char*)Malloc(length * 2 + 1);
  
  for(uint8_t i=0; i<length; i++)
  {
    string[i * 2] = Hex2Char(message[i] >> 4);
    string[i * 2 + 1] = Hex2Char(message[i] & 0x0F);
  }
  
  string[length * 2] = 0;  //������
  
  return string;
}
/*********************************************************************************************

  * @brief  �ַ�������ת��Ϊ�޷�������
  * @param  numString�������ַ���
  * @retval ������ɵ��޷�����
  * @remark 

  ********************************************************************************************/
uint32_t NumberString2Uint(const char* numString)
{
  uint32_t temp32u = 0;
  uint8_t len = strlen(numString);
  
  for(uint8_t i=0; i<len; i++)
  { temp32u += (*(uint8_t *)(numString + i) - 0x30) * Math_10nthPower(len - 1 - i); }
  
  return temp32u;
}
/*********************************************************************************************

  * @brief  uintת�ַ���
  * @param  number ����
  * @retval ת����ɵ��ַ���
  * @remark 

  ********************************************************************************************/
char* Uint2String(uint32_t number)
{
  uint8_t len = (uint8_t)log10((double)number) + 1;
  uint32_t temp = 0;
  
  /* �����ڴ�׼�������ַ��� */ 
  char *numString = (char*)Malloc(len + 1);     // �����Ӧ�ڴ�
  memset(numString, 0, len + 1);        // ȫ������
  
  /* �ַ�����ֵ */
  temp = number;        // ����number
  for(uint8_t i=0; i<len; i++)
  {
    numString[i] = Hex2Char(temp / (int)Math_10nthPower(len - 1 - i));  // ���λ��ʼת��
    temp = temp % (int)Math_10nthPower(len - 1 - i);                    // ��¼ʣ��ֵ
  }
  
  return numString;
}
/*********************************************************************************************

  * @brief  ����ת��Ϊ���� 4321 => {4,3,2,1}
  * @param  number����Ҫת��������
            isPositiveSequence���Ƿ�Ϊ����4321 => {4,3,2,1} ���� {1,2,3,4}
  * @retval ��������ת��Ϊ����
  * @remark 

  ********************************************************************************************/
ArrayStruct* Number2Array(uint32_t number, BoolEnum isPositiveSequence)
{
  uint8_t powIndex = (uint8_t)log10(number);
  uint32_t temp32 = number;
  uint32_t remain = 0;
  
  /* �����ڴ沢��� */
  ArrayStruct* numArray = Array_New(powIndex + 1);     // �����Ӧ�ڴ�
  
  for(int8_t i=powIndex; i>=0; i--)
  {
    if(isPositiveSequence == TRUE)
    { numArray->packet[powIndex - i] = temp32 / Math_10nthPower(i); }
    else
    { numArray->packet[i] = temp32 / Math_10nthPower(i); }
    
    remain = Math_10nthPower(i);      // pow���ظ��������п���99.999ת��Ϊ99�����0.1��֤
    temp32 %= remain;
  }
  
  return numArray;
}
/*********************************************************************************************

  * @brief  �ֽ�˳����ת��
  * @param  dst��Ŀ��ָ��
  * @param  src��Դָ��
  * @param  len������
  * @retval 
  * @remark 

  ********************************************************************************************/
void EndianExchange(uint8_t* dst, uint8_t* src, uint8_t len)
{
  for(uint8_t i=0; i<len; i++)
  { dst[i] = src[len - i - 1]; }
}
/*********************************************************************************************

  * @brief  ʱ���ת����
  * @param  timeStamp��ʱ���
  * @param  calendar������ָ��
            timeZone��ʱ��
  * @retval 
  * @remark 

  ********************************************************************************************/
void TimeStamp2Calendar(uint32_t timeStamp, CalendarStruct *calendar, uint8_t timeZone)
{
  timeStamp += timeZone * 3600L;
  uint32_t daySec = 3600L * 24L;
  uint32_t days = timeStamp / daySec;
  uint32_t secs = timeStamp % daySec;
  uint16_t years4List[4] = {365, 365, 366, 365};
  uint8_t monthList[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  uint8_t i=0;
  
  uint16_t remainDays = days % 1461;            // 4��֮�ڵ�
  
  calendar->year = 1970 + (days / 1461) * 4;
  
  /************** ��ݼ��� **************/
  for(i=0; i<4; i++)
  {
    if(remainDays <= years4List[i])
    {
      calendar->year += i;              // ������
      if(i == 2)
      { monthList[1] += 1; }               // ���¶��¼�һ��
      break;
    }
    remainDays -= years4List[i];        
  }
  
  /************** ���ռ��� **************/
  for(i=0; i<12; i++)
  {
    if(remainDays <= monthList[i])
    {
      calendar->month = i + 1;
      calendar->day = remainDays + 1;
      break;
    }
    remainDays -= monthList[i];
  }
  
  calendar->hour = secs / 3600;
  calendar->min = (secs % 3600) / 60;
  calendar->sec = secs % 60;
}
/*********************************************************************************************

  * @brief  ����תʱ���
  * @param  calendar������ָ��
            timeZone��ʱ��
  * @retval 
  * @remark 

  ********************************************************************************************/
uint32_t Calendar2TimeStamp(CalendarStruct *calendar, uint8_t timeZone)
{
  uint32_t temp32u = 0;
  uint8_t i=0;
  uint16_t years4List[4] = {365, 365, 366, 365};
  uint8_t monthList[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  uint8_t temp8u = (calendar->year - 1970L) / 4;
  temp32u += temp8u * 1461L;
  temp8u = (calendar->year - 1970L) % 4;
  for(i=0; i<temp8u; i++)
  { temp32u += years4List[i]; }
  
  if(temp8u == 2)
  { monthList[1] += 1; }
  
  for(i=0; i<(calendar->month - 1); i++)
  { temp32u += monthList[i];}
  
  temp32u += (calendar->day - 1);
  
  temp32u *= 24L * 3600L;
  
  temp32u += calendar->hour * 3600L;
  temp32u += calendar->min * 60L;
  temp32u += calendar->sec;
  
  temp32u -= timeZone * 3600L;
  
  return temp32u;
}