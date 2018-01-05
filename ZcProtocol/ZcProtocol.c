/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "Base.h"
#include "ZcProtocol.h"
#include "Convert.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
ZcProtocol zcPrtc;

/* Private function prototypes ------------------------------------------------*/
uint8_t ZcProtocol_GetCrc(uint8_t *message, uint16_t length);

/*********************************************************************************************

  * @brief  Э��ת����
  * @param  protocol:  Э��ṹ��ָ��
  * @retval ��������ṹ��ָ��
  * @remark 

  ********************************************************************************************/
ArrayStruct* ZcProtocol_ConvertMsg(ZcProtocol* zcProtocol, uint8_t *message, uint16_t length)
{
  zcProtocol->head.length = ZC_UNDATA_LEN + length;
  
  ArrayStruct *msg = Array_New(zcProtocol->head.length);     // Э��ͷ��ָ��ռ4���ֽڣ���βռ�����ֽ�

  *(ZcProtocolHead*)(msg->packet) = zcProtocol->head;        // ����Э��ͷ
  
  memcpy(msg->packet + ZC_HEAD_LEN, message, length);      // ��������

  msg->packet[msg->length - 2] = ZcProtocol_GetCrc(msg->packet, msg->length);     // ���CRC
  msg->packet[msg->length - 1] = ZC_END;                //�������ַ�
  
  return msg;
}


/*********************************************************************************************

  * @brief  Э���ʼ��
  * @param  
  * @retval 
  * @remark ����һЩ�������趨��ַ

  ********************************************************************************************/
ZcProtocol* ZcProtocol_Check(uint8_t *message, uint16_t length)
{
  if(length < ZC_UNDATA_LEN)
  { return 0; }
  
  ZcProtocol *protocol = (ZcProtocol*)message;  // ��ȡЭ��ṹָ��
  ZcProtocolEnd *tail = (ZcProtocolEnd*)(message + length - 2);
  
  /* ��β�ж� */
  if(protocol->head.head != ZC_HEAD || tail->end != ZC_END)
  {  return 0;  }
  
  /* CRC�ж� */
  if(tail->crc != ZcProtocol_GetCrc(message, length))
  { return 0;  }

  return protocol;
}
/*********************************************************************************************

  * @brief  �ж�Э��ID�Ƿ���ͬ
  * @param  message:  ����ָ��
            length�����ĳ���
  * @retval ��ͬ����0����ͬ����1
  * @remark ���ڷ��ͻ�������

  ********************************************************************************************/
uint8_t ZcProtocol_SameId(uint8_t *message, uint16_t length, void *p)
{
  uint8_t id = 0, position=42;
  id |= ((message[position] > 0x40)? (message[position] - 0x41 + 10): (message[position] - 0x30))<<4;
  id |= (message[position + 1] > 0x40)? (message[position + 1] - 0x41 + 10): (message[position + 1] - 0x30);

  if(id == *((uint8_t *)p))
  { return 0; }
  
  return 1;
}


/*********************************************************************************************

  * @brief  Э���ȡCRC
  * @param  message:  ����ָ��
            length�����ĳ���
  * @retval һ���ֽڵ�crc
  * @remark 

  ********************************************************************************************/
uint8_t ZcProtocol_GetCrc(uint8_t *message, uint16_t length)
{
  uint8_t crc = 0;
  for(uint16_t i=0; i<(length - 2); i++)             //���CRC
  { crc += message[i]; }

  return crc;
}
