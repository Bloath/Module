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
ZcHandleStruct zcHandle;      //׾��Э�鴦��

/* private function prototypes ------------------------------------------------*/
void ZcProtocol_NetRxHandle(ZcProtocol *zcProtocol);
void ZcProtocol_24GRxHandle(ZcProtocol *zcProtocol);
void ZcProtocol_HeadIdIncrease();

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
  if(timeStamp != 0)
  { 
    zcPrtc.head.timestamp = timeStamp;  //����ʱ���
    time = realTime;                     //��¼��ʱ��ϵͳʱ��
    return 0;                           //����0
  }
  else
  { return zcPrtc.head.timestamp + realTime - time; } //��ȡʱ�����������֮ǰ��¼��ʱ����Ļ����ϣ����Ϻ����ܹ���ϵͳʱ�� 
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
/*********************************************************************************************

  * @brief  ����Э�鱨�ĵĴ���
  * @param  
  * @retval 
  * @remark 

  ********************************************************************************************/
void ZcProtocol_NetTxProcess()
{
  static uint32_t time=0;
  
  /* ������״̬���д��� */
  switch(zcHandle.status)
  {
  /* ��ʼ�� */
  case ZcHandleStatus_Init:
    zcHandle.loopInterval = ZC_NET_POLL_INTERVAL;
    zcHandle.status = ZcHandleStatus_PollInterval;
    time = realTime;
    break;
  
  /* ��ѵ���ʱ�� */
  case ZcHandleStatus_PollInterval:
     /* ���һ��ʱ�� */
    if((time + zcHandle.loopInterval) > realTime)
    { break; }
    else
    { zcHandle.status = ZcHandleStatus_Trans; }
  
  /* ����״̬������ѯ�ݴ汨�ģ��л�Ϊ�ȴ�״̬ */
  case ZcHandleStatus_Trans:
    ZcProtocol_Request(ZcSource_Net, 00, NULL, 0, FALSE, TX_MULTI_MC);     //�����ݴ汨��
    zcHandle.status = ZcHandleStatus_Wait;      //�л��ȴ�״̬
    break;
    
  /* �ȴ�״̬�����ͻ���Ϊ�ֶ���������û�лظ��Ļ��ǲ�������� 
     �ȴ�״̬Ϊ������ֱ�����յ�ȷ�ϱ��Ļ��߲�ѯ�ݴ�ظ�*/
  case ZcHandleStatus_Wait:
    time = realTime;
    break;
  }
}
/*********************************************************************************************

  * @brief  ����Э�鱨�ĵĴ���
  * @param  packet:  ����ָ��
            length�����ĳ���
            source����Դ��NET/2.4G/485
  * @retval ��������ṹ��ָ��
  * @remark 

  ********************************************************************************************/
void ZcProtocol_ReceiveHandle(uint8_t *packet, uint16_t length, ZcSourceEnum source)
{
  ZcProtocol *zcProtocol = ZcProtocol_Check(packet, length);     //�����յĻظ��Ƿ�׼ȷ
  
  if(zcProtocol != NULL)
  {
    ZcProtocol_TimeStamp(zcProtocol->head.timestamp);             //ÿ�ζ�����ʱ���
    
    /* ���ݲ�ͬ����Դ���в�ͬ�Ĵ��� */
    switch(source)
    {
    case ZcSource_Net:
      ZcProtocol_NetRxHandle(zcProtocol);                   // �����ͨѶЭ�飬�ؼ����������豸���𣬷�����������Ӧ
      break;
    case ZcSource_24G:
      ZcProtocol_24GRxHandle(zcProtocol);                   // 2.4GЭ��
      break;
    case ZcSource_485:
      ZcProtocol_24GRxHandle(zcProtocol);                   // 485
      break;
    }
  }
}

//////////////////////////////////////////���Ľ��մ����Ѿ�ͨ��Э����֮��////////////////////////////////////////////////

/*********************************************************************************************

  * @brief  ���ձ��Ĵ���-> ����
  * @param  zcProtocol��Э�鱨��ָ��
  * @retval 
  * @remark 

  ********************************************************************************************/
void ZcProtocol_NetRxHandle(ZcProtocol *zcProtocol)
{
#ifdef ZC_NET
  TxQueue_FreeById(&Enthernet_TxQueue, zcProtocol->head.id);    //ÿ�η������ظ���Ҫ�����Ӧ���ͱ���

    /* �Ȳ鿴�Ƿ��в����ı���
     �ظ�0������ɹ�������Ҫ��������
     �ظ�1���Ǵ����������ͨѶ�౨�Ĵ���
     �ظ�2������ʧ�ܣ�ֱ�ӷ���ʧ�ָܻ����� */
  uint8_t operationRes = ZcProtocol_NetOperationCmdHandle(zcProtocol);        //�Ȳ鿴�Ƿ��в����ı���

  if(operationRes == 1)
  {
    switch(zcProtocol->head.cmd)                          
    { 
    /* ��ѯ�ݴ汨�� */
    case ZC_CMD_QUERY_HOLD:
      zcHandle.status = ZcHandleStatus_Init;  // �л�Ϊ��ʼ��״̬������Ԥ����ѵʱ�������ѵ
      break;
      
    /* �������·�ȷ�ϱ��� */
    case ZC_CMD_SERVER_CONFIRM:
      ZcProtocol_HeadIdIncrease();                       // Id����
           
      /* ����������SFDΪ1ʱ��ֱ�ӷ��Ͳ�ѯ�ݴ汨��,���л�Ϊ�ȴ�״̬*/
      if((zcProtocol->head.control & (1<<7)) != 0)
      { zcHandle.status = ZcHandleStatus_Trans; }       // ֱ���л�Ϊ����״̬   
      else
      { zcHandle.status = ZcHandleStatus_Init; }        // �л�Ϊ��ʼ��״̬������Ԥ����ѵʱ�������ѵ
      break;
      
    /* �ظ������� */
    default:
      ZcProtocol_Request(ZcSource_Net, ZC_CMD_FAIL, NULL, 0, FALSE, TX_MULTI_MC);
      break;
    }
  }
  else if(operationRes == 2)
  { ZcProtocol_Request(ZcSource_Net, ZC_CMD_FAIL, NULL, 0, FALSE, TX_MULTI_MC); }        //������ָ��ʧ�ܣ�����ʧ�ܻظ�
#endif
}
/*********************************************************************************************

  * @brief  ���ձ��Ĵ���-> 2.4G
  * @param  zcProtocol��Э�鱨��ָ��
  * @retval 
  * @remark 

  ********************************************************************************************/
void ZcProtocol_24GRxHandle(ZcProtocol *zcProtocol)
{
#ifdef ZC_24G
  
  TxQueue_FreeById(&nRF24L01_TxQueue, zcProtocol->head.id);    
  ZcProtocol_24GOperationCmdHandle(zcProtocol);
  
#endif
}
/*********************************************************************************************

  * @brief  ���ձ��Ĵ���-> 485
  * @param  zcProtocol��Э�鱨��ָ��
  * @retval 
  * @remark 

  ********************************************************************************************/
void ZcProtocol_485RxHandle(ZcProtocol *zcProtocol)
{
#ifdef ZC_485
  
  TxQueue_FreeById(&ZC_485_TXQUEUE, zcProtocol->head.id);    
  ZcProtocol_485OperationCmdHandle(zcProtocol);
  
#endif
}


////////////////////////////////////��������/////////////////////////////////////////////////

/*********************************************************************************************

  * @brief  ׾��Э��=������Э�鴦��
  * @param  packet��httpЭ��ظ����ݰ��ڵĻظ����ݲ����ַ���
            length�����ĳ���
  * @retval 
  * @remark 

  ********************************************************************************************/
void ZcProtocol_NetPacketHandle(uint8_t *packet, uint16_t length)
{
  ArrayStruct *msg = String2Msg(strstr((const char*)packet, "68"), 0);      // �ַ���ת��������
  
  ZcProtocol_ReceiveHandle(msg->packet, msg->length, ZcSource_Net);  //Э���ԭʼ���Ĵ���
  
  Array_Free(msg);      // �ͷű����������
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
