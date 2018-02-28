/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "../Module/Common/Malloc.h"
#include "ZcProtocol.h"
#include "Http.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
uint8_t ZcProtocol_GetCrc(uint8_t *message, uint16_t length);

/*********************************************************************************************

  * @brief  Э��ת����
  * @param  protocol:  Э��ṹ��ָ��
            
  * @retval ��������ṹ��ָ��
  * @remark 

  ********************************************************************************************/
ArrayStruct* ZcProtocol_ConvertMsg(ZcProtocol* zcProtocol, uint8_t *data, uint16_t dataLen)
{
  /* ȷ�����ݳ��ȣ���������Ӧ�ڴ� */
  if(data == NULL)
  { dataLen = 1; }
  zcProtocol->head.length = ZC_UNDATA_LEN + dataLen;
  ArrayStruct *msg = Array_New(zcProtocol->head.length);     

  /* ���� ͷ �����ݡ���β��˳��ʼ�Ա��Ľ��и�ֵ */
  *(ZcProtocolHead*)(msg->packet) = zcProtocol->head;        // ����Э��ͷ
  
  if(data == NULL)
  { msg->packet[ZC_HEAD_LEN] = 0; }
  else
  { memcpy(msg->packet + ZC_HEAD_LEN, data, dataLen); }          // ����������

  msg->packet[msg->length - 2] = ZcProtocol_GetCrc(msg->packet, msg->length);     // ���CRC
  msg->packet[msg->length - 1] = ZC_END;                //�������ַ�
  
  return msg;
}
/*********************************************************************************************

  * @brief  ׾��Э��תHTTPЭ��
  * @param  zcProtocol:  ׾��Э��ʵ��
            data��������ָ��
            dataLen�������򳤶�
  * @retval �������ݰ��ַ���ָ��
  * @remark �ַ�������Ҫ����

  ********************************************************************************************/
char* ZcProtocol_ConvertHttpString(ZcProtocol* zcProtocol, uint8_t *data, uint16_t dataLen)
{
  ArrayStruct *msg = ZcProtocol_ConvertMsg(zcProtocol, data ,dataLen);      // ����ת���ɱ��ģ�����������
  
  char *msgString = Msg2String(msg->packet, msg->length);       // ��ת��Ϊ�ַ���������Ϊԭʼ��������
  Array_Free(msg);
  
  char *httpMsg = Http_Request(msgString);                    // ��ת����Http����
  Free(msgString);
  
  return httpMsg;
}

/*********************************************************************************************

  * @brief  Э���飬
  * @param  
  * @retval ����Э��ָ��
  * @remark ���Э��ͷ��β�Լ�CRC��

  ********************************************************************************************/
ZcProtocol* ZcProtocol_Check(uint8_t *message, uint16_t length)
{
  if(length < ZC_UNDATA_LEN)
  { return NULL; }
  
  ZcProtocol *protocol = (ZcProtocol*)message;  // ��ȡЭ��ṹָ��
  ZcProtocolEnd *tail = (ZcProtocolEnd*)(message + protocol->head.length - 2);
  
  /* ��β�ж� */
  if(protocol->head.head != ZC_HEAD || tail->end != ZC_END)
  {     return NULL;  }
  
  /* CRC�ж� */
  if(tail->crc != ZcProtocol_GetCrc(message, protocol->head.length))
  { return NULL;  }

  return protocol;
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
  
  crc ^= 0xFF;

  return crc;
}
