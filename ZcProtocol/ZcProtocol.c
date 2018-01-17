/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#include "../Common/Array.h"
#include "../Common/Convert.h"
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
  free(msgString);
  
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
  ZcProtocolEnd *tail = (ZcProtocolEnd*)(message + length - 2);
  
  /* ��β�ж� */
  if(protocol->head.head != ZC_HEAD || tail->end != ZC_END)
  {  return NULL;  }
  
  /* CRC�ж� */
  if(tail->crc != ZcProtocol_GetCrc(message, length))
  { return NULL;  }

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
  uint8_t res=1;
 
  char* index = strstr( (char *)message, "message=");
  ArrayStruct* msg = String2Msg(index + 8);
  
  ZcProtocol *protocol = (ZcProtocol*)msg->packet;

  if(protocol->head.id == *((uint8_t *)p))
  { res = 0; }
  
  Array_Free(msg);
  
  return res;
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
