/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "ZcProtocol.h"

//�ڴ���������Բ�ͬ��������ӵ�ͷ
#include "../Module/Common/Malloc.h"
#include "..//BufferQueue/BufferQueue.h"
#include "../Sys_Conf.h"
#include "Http.h"
#include "ZcProtocol_API.h"    
#include "ZcProtocol_Conf.h"   
#include "ZcProtocol_Handle.h"  //Ӳ����ش���

// ��ͬƽ̨
#ifdef ZC_NET
  #include "../ESP8266/ESP8266_Handle.h"
#endif

#ifdef ZC_24G
  #include "../SPI_Chip/nRF24L01P/nRF24L01P_Handle.h"
#endif


/* private typedef ------------------------------------------------------------*/
/* private define -------------------------------------------------------------*/
/* private macro --------------------------------------------------------------*/
/* private variables ----------------------------------------------------------*/
ZcProtocol zcPrtc;      // ׾��Э��ʵ��
/* private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  Э���ʼ��
  * @param  DeviceType���豸����
            address���豸��ַָ��
  * @retval 
  * @remark �ڳ����ʼ��ʱ����Ҫ��Э��ʵ�����г�ʼ��

  ********************************************************************************************/
void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t* address, uint8_t startId)
{
  zcPrtc.head.head = 0x68;
  zcPrtc.head.control = DeviceType;
  zcPrtc.head.id = startId;                           // 0��Ԥ���������豸����
  zcPrtc.head.timestamp = 0;
  
  memcpy(zcPrtc.head.address, address, 7);      //����7�ֽڵ�ַ���ڲ�Ʒʵ�����ú󣬵�ַ�ǲ���ı�ģ���΢�Źҹ���
}

/*********************************************************************************************

  * @brief  ��ȡ/����ʱ���
  * @param  timeStamp������ʱ����ĸ��£���Ϊ0�����Э��ʵ����ʱ�����Ϊ0�򷵻ص�ǰʱ���
  * @retval ���Ϊ���ã��򷵻�0����������ã��򷵻ص�ǰʱ�������Ƭ���ڲ����£�
  * @remark 

  ********************************************************************************************/
uint32_t ZcProtocol_TimeStamp(uint32_t timeStamp)
{  
  static uint32_t time = 0;    //���ڼ�¼�ϴε�ϵͳʱ��
  
  /* ��������Ϊ0ʱ����Ϊ����ʱ��� */
  /* ������Ϊ0ʱ����Ϊ��ȡʱ��� */
  if(timeStamp == 0)
  { return zcPrtc.head.timestamp + realTime - time; } //��ȡʱ�����������֮ǰ��¼��ʱ����Ļ����ϣ����Ϻ����ܹ���ϵͳʱ�� 
  else if(timeStamp > 1514736000)       // 2018��1��1�գ� 0:0:0
  { 
    zcPrtc.head.timestamp = timeStamp;  //����ʱ���
    time = realTime;                     //��¼��ʱ��ϵͳʱ��
  }
   return 0;
}
/*********************************************************************************************

  * @brief  ׾��Э��=��Э��ʵ��ID����
  * @param  
  * @retval 
  * @remark 0Ԥ���������豸

  ********************************************************************************************/
void ZcProtocol_HeadIdIncrease()
{
  zcPrtc.head.id ++;
  if(zcPrtc.head.id == 0)
  { zcPrtc.head.id = 1; }
}
/*********************************************************************************************

  * @brief  ׾��Э�� ��������
  * @param  source��Դ�����磬24G�� 485��
            cmd: ������
            data:  ����ָ��
            dataLen�����ĳ���
            isUpdateId���Ƿ���Ҫ����ID
            txMode������ģʽ
  * @retval �����������id
  * @remark ͨ�����������Լ����ݣ�����д�����ͻ��嵱��

  ********************************************************************************************/
uint8_t ZcProtocol_Request(ZcSourceEnum source, uint8_t cmd, uint8_t *data, uint16_t dataLen, BoolEnum isUpdateId, TxModeEnum txMode)
{
  char* httpMsg;
  ArrayStruct *msg;
  uint8_t temp8 = 0;
  
    /* ȡһ����ID */
  if(isUpdateId == TRUE)
  { ZcProtocol_HeadIdIncrease(); }
  temp8 = zcPrtc.head.id;
  
  zcPrtc.head.timestamp = ZcProtocol_TimeStamp(0);      //����ʱ���
  zcPrtc.head.cmd = cmd;
  
  /* ���ݲ�ͬ��Դ�����в�ͬ�ķ��ʹ��� */
  switch(source)
  {
  case ZcSource_Net:
#ifdef ZC_NET
    httpMsg = ZcProtocol_ConvertHttpString(&zcPrtc, data, dataLen);         //ת��ΪHTTPЭ�飬
    TxQueue_AddWithId(&Enthernet_TxQueue, 
                      (uint8_t*)httpMsg, 
                      strlen(httpMsg), 
                      txMode,
                      zcPrtc.head.id);  
    Free(httpMsg);
#endif
    break;
  case ZcSource_24G:
#ifdef ZC_24G
    msg = ZcProtocol_ConvertMsg(&zcPrtc, data ,dataLen);
    TxQueue_AddWithId(&nRF24L01_TxQueue, 
                      msg->packet, 
                      msg->length, 
                      txMode,
                      zcPrtc.head.id);  
    Array_Free(msg);
#endif
    break;
    
  case ZcSource_485:
#ifdef ZC_485
    msg = ZcProtocol_ConvertMsg(&zcPrtc, data ,dataLen);
    TxQueue_AddWithId(&ZC_485_TXQUEUE, 
                      msg->packet, 
                      msg->length, 
                      txMode,
                      zcPrtc.head.id);  
    Array_Free(msg);
#endif
    break;
  }
  
  /* ȡһ����ID */
  if(isUpdateId == TRUE)
  { ZcProtocol_HeadIdIncrease(); } 
  
  return temp8;
}
/*********************************************************************************************

  * @brief  ׾��Э�� �ظ����ڽ��յ������Ļظ�
  * @param  source��Դ�����磬24G�� 485��
            zcProtocol�����հ��н������������ݽṹָ��
            data:  ����ָ��
            dataLen�����ĳ���
  * @retval 
  * @remark ������ͬ���ظ�һ�����������ID��CMD�ȣ����������ݲ���������

  ********************************************************************************************/
void ZcProtocol_Response(ZcSourceEnum source, ZcProtocol *zcProtocol, uint8_t *data, uint16_t dataLen)
{
  char* httpMsg;
  ArrayStruct *msg;
  
  /* ���ݲ�ͬ��Դ�����в�ͬ�ķ��ʹ��� */
  switch(source)
  {
  case ZcSource_Net:
#ifdef ZC_NET
    httpMsg = ZcProtocol_ConvertHttpString(zcProtocol, data, dataLen);         //ת��ΪHTTPЭ�飬
    TxQueue_AddWithId(&Enthernet_TxQueue, 
                      (uint8_t*)httpMsg, 
                      strlen(httpMsg), 
                      TX_MULTI_MC,
                      zcProtocol->head.id);  
    Free(httpMsg);
#endif
    break;
  case ZcSource_24G:
#ifdef ZC_24G
    msg = ZcProtocol_ConvertMsg(zcProtocol, data ,dataLen);
    TxQueue_AddWithId(&nRF24L01_TxQueue, 
                      msg->packet, 
                      msg->length, 
                      TX_ONCE_AC,
                      zcProtocol->head.id);  
    Array_Free(msg);
#endif
    break;
    
  case ZcSource_485:
#ifdef ZC_485
    msg = ZcProtocol_ConvertMsg(zcProtocol, data ,dataLen);
    TxQueue_AddWithId(&ZC_485_TXQUEUE, 
                      msg->packet, 
                      msg->length, 
                      TX_ONCE_AC,
                      zcProtocol->head.id);  
    Array_Free(msg);
#endif
    break;
  }
}

