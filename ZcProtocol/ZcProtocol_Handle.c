/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "ZcProtocol.h"
#include "Array.h"
#include "Convert.h"

//�ڴ���������Բ�ͬ��������ӵ�ͷ
#include "ESP8266.h"
#include "Http.h"
#include "SimpleBuffer.h"

/* private typedef ------------------------------------------------------------*/
/* private define -------------------------------------------------------------*/
/* private macro --------------------------------------------------------------*/
/* private variables ----------------------------------------------------------*/
/* private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  Э���ʼ��
  * @param  
  * @retval 
  * @remark ����һЩ�������趨��ַ

  ********************************************************************************************/
void ZcProtocol_Init()
{
  zcPrtc.head.head = 0x68;
  zcPrtc.head.control = 0xB0;
  zcPrtc.head.info = 0x0140;
  zcPrtc.head.afn = 0x03;
  zcPrtc.head.id = 0x65;
  
  uint8_t address[7] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11};
  memcpy(zcPrtc.head.address, address, 7);
  
}
/*********************************************************************************************

  * @brief  ����Э�鱨�ĵĴ���
  * @param  message:  ����ָ��
            length�����ĳ���
  * @retval ��������ṹ��ָ��
  * @remark 

  ********************************************************************************************/
void ZcProtocol_ReceiveHandle(uint8_t *message, uint16_t length)
{
  ZcProtocol *protocol = ZcProtocol_Check(message, length);
  
  if(protocol != NULL)
  {
    ClearSpecifyBlock(esp8266_TxBlockList, ZcProtocol_SameId, &protocol->head.id);      //���ID��ͬ�ķ��ͱ���
  }
}


/*********************************************************************************************

  * @brief  ׾��Э��=������Э�鴦��
  * @param  message��httpЭ��ظ����ݰ��ڵĻظ����ݲ����ַ���
            length�����ĳ���
  * @retval 
  * @remark 

  ********************************************************************************************/
void ZcProtocol_NetReceiveHandle(uint8_t *message, uint16_t length)
{
  ArrayStruct *msg = String2Msg((char*)message);                // �ַ���ת��������
  
  ZcProtocol_ReceiveHandle(msg->packet, msg->length);           //Э���ԭʼ���Ĵ���
  
  Array_Free(msg);      // �ͷű����������
}
