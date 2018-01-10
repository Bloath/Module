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

  * @brief  �ַ���ת����
  * @param  string���ַ���
  * @remark ת����ɵı�������

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

  * @brief  ����ת�ַ���
  * @param  message������ָ��
            length������
  * @retval ת����ɵ��ַ���
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
  
  string[length * 2] = 0;  //������
  
  return string;
}
/*********************************************************************************************

  * @brief  uintת�ַ���
  * @param  number ����
  * @retval ת����ɵ��ַ���
  * @remark 

  ********************************************************************************************/
char* Uint2String(uint32_t number)
{
  uint8_t len = log10((double)number) + 1;
  uint32_t temp = 0;
  
  /* �����ڴ�׼�������ַ��� */ 
  char *numString = (char*)malloc(len + 1);     // �����Ӧ�ڴ�
  memset(numString, 0, len + 1);        // ȫ������
  
  /* �ַ�����ֵ */
  temp = number;        // ����number
  for(uint8_t i=0; i<len; i++)
  {
    numString[i] = Hex2Char(temp / (int)pow(10, len - i - 1));  // ���λ��ʼת��
    temp = temp % (int)pow(10, len - i - 1);                    // ��¼ʣ��ֵ
  }
  
  return numString;
}