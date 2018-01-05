/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "Base.h"
#include "ZcProtocol.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  ������ת�ַ���
  * @param  x��������byte
  * @remark 0x6B ==> "6B"

  ********************************************************************************************/
uint8_t Hex_ConvertString(uint8_t x)
{
  x &= 0x0F;
  
  if(x > 9)
  { return (0x41 + x - 10); }
  else
  { return (0x30 + x); }
}

/*********************************************************************************************

  * @brief  �ַ���ת
  * @param  x��������byte
  * @remark 0x6B ==> "6B"

  ********************************************************************************************/
ArrayStruct* String_ConvertMessage(uint8_t *message, uint16_t length)
{
  ArrayStruct* msg = Array_New(length / 2);
  
  for(uint16_t i=0; i<(length / 2); i++)
  {
    msg->packet[i] = 0;
    msg->packet[i] |= ((message[i * 2] > 0x40)? (message[i * 2] - 0x41 + 10): (message[i * 2] - 0x30))<<4;
    msg->packet[i] |= (message[i * 2 + 1] > 0x40)? (message[i * 2 + 1] - 0x41 + 10): (message[i * 2 + 1] - 0x30);
  }
  
  return msg;
}
/*********************************************************************************************

  * @brief  ����ת�ַ���
  * @param  message������ָ��
            length������
  * @retval ת����ɺ�Ķ���
  * @remark 

  ********************************************************************************************/
ArrayStruct* Message_ConvertString(uint8_t *message, uint16_t length)
{
  ArrayStruct *msg = Array_New(length * 2);
  
  for(uint8_t i=0; i<length; i++)
  {
    msg->packet[i * 2] = Hex_ConvertString(message[i] >> 4);
    msg->packet[i * 2 + 1] = Hex_ConvertString(message[i] & 0x0F);
  }
  
  return msg;
}