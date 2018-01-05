/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "ZcProtocol.h"
#include "StringHandle.h"
#include "Base.h"
#include "Convert.h"

//�ڴ���������Բ�ͬ��������ӵ�ͷ
#include "ESP8266.h"
#include "Http.h"

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
void ZcProtocol_ReponseHandle(uint8_t *message, uint16_t length)
{
  ArrayStruct *msg = String_ConvertMessage(message, length);            // �ַ���ת����
  
  ZcProtocol *protocol = ZcProtocol_Check(msg->packet, msg->length);
  
  if(protocol != 0)
  {
    ClearSpecifyBlock(esp8266_TxBlockList, ZcProtocol_SameId, &protocol->head.id);      //���ID��ͬ�ķ��ͱ���
  }
  
  Array_Free(msg);
}
/*********************************************************************************************

  * @brief  ����Э�鱨�ĵĴ���
  * @param  message:  ����ָ��
            length�����ĳ���
  * @retval ��������ṹ��ָ��
  * @remark 

  ********************************************************************************************/
void ZcProtocol_ConvertHttpMsg(ZcProtocol* zcProtocol, TxBlockTypeDef *txBlock, uint8_t *message, uint16_t length)
{
  ArrayStruct *msg = ZcProtocol_ConvertMsg(zcProtocol, message ,length);      // ����ת���ɱ���
  ArrayStruct *strMsg = Message_ConvertString(msg->packet, msg->length);       // ��ת��Ϊ�ַ���
  Array_Free(msg);
  
  ArrayStruct *httpMsg = Http_Request(strMsg->packet, strMsg->length);                    // ��ת����Http��
  Array_Free(strMsg);
  FillTxBlock(txBlock, httpMsg->packet, httpMsg->length,  TX_FLAG_RT|TX_FLAG_MC);       // ��䵽���ͻ��嵱��
  Array_Free(msg);
}
