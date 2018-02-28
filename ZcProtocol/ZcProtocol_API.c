/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "ZcProtocol.h"

//�ڴ���������Բ�ͬ��������ӵ�ͷ
#include "../Module/Common/Malloc.h"
#include "../UartDma/SimpleBuffer.h"
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
ZcHandleStruct zcHandle = {0};      //׾��Э�鴦��

/* private function prototypes ------------------------------------------------*/
void ZcProtocol_NetRxHandle(ZcProtocol *protocol);

/*********************************************************************************************

  * @brief  Э���ʼ��
  * @param  DeviceType���豸����
            address���豸��ַָ��
  * @retval 
  * @remark �ڳ����ʼ��ʱ����Ҫ��Э��ʵ�����г�ʼ��

  ********************************************************************************************/
void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t* address)
{
  zcPrtc.head.head = 0x68;
  zcPrtc.head.control = DeviceType;
  zcPrtc.head.id = 0;
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

  * @brief  ׾��Э�� ����ͨѶ���Ͳ���
  * @param  source��Դ�����磬24G�� 485��
            cmd: ������
            data:  ����ָ��
            dataLen�����ĳ���
            isUpdateId���Ƿ���Ҫ����ID
  * @retval 
  * @remark ͨ�����������Լ����ݣ�����д�����ͻ��嵱��

  ********************************************************************************************/
void ZcProtocol_Transmit(ZcSourceEnum source, uint8_t cmd, uint8_t *data, uint16_t dataLen, uint8_t isUpdateId)
{
  char* httpMsg = NULL;
  
    /* ȡһ����ID */
  if(isUpdateId)
  { zcPrtc.head.id ++; }  
  
  zcPrtc.head.timestamp = ZcProtocol_TimeStamp(0);      //����ʱ���
  zcPrtc.head.cmd = cmd;
  
  /* ���ݲ�ͬ��Դ�����в�ͬ�ķ��ʹ��� */
  switch(source)
  {
  case ZcSource_Net:
#ifdef ZC_NET
    httpMsg = ZcProtocol_ConvertHttpString(&zcPrtc, data, dataLen);         //ת��ΪHTTPЭ�飬
    FillTxBlockWithId(Enthernet_TxBlockList, 
                      (uint8_t*)httpMsg, 
                      strlen(httpMsg), 
                      TX_FLAG_MC|TX_FLAG_RT,
                      zcPrtc.head.id);  
    Free(httpMsg);
#endif
    break;
  case ZcSource_24G:
#ifdef ZC_24G
    FillTxBlockWithId(nRF24L01_TxBlockList, 
                      data, 
                      dataLen, 
                      TX_FLAG_MC|TX_FLAG_RT,
                      zcPrtc.head.id);  
#endif
    break;
  }
      
  
  
    /* ����ID */
  if(isUpdateId)
  { zcPrtc.head.id ++; } 
}
/*********************************************************************************************

  * @brief  ����Э�鱨�ĵĴ���
  * @param  
  * @retval ��������ṹ��ָ��
  * @remark 

  ********************************************************************************************/
void ZcProtocol_Handle()
{
  static uint32_t time=0;
  
  /* ���һ��ʱ�� */
  if((time + zcHandle.loopInterval) > realTime)
  { return; }
  else
  { time = realTime; }
  
  /* ������״̬���д��� */
  switch(zcHandle.status)
  {
    /* ��ʼ�� */
  case ZcHandleStatus_Init:
    zcHandle.loopInterval = 60;
    zcHandle.status = ZcHandleStatus_Idle;
    break;
    
    /* ����״̬������ѯ�ݴ汨�ģ��л�Ϊ�ȴ�״̬ */
  case ZcHandleStatus_Idle:
    ZcProtocol_Transmit(ZcSource_Net, 00, NULL, 0, 0);     //�����ݴ汨��
    zcHandle.holdId = zcPrtc.head.id;           //��¼��ǰID
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
  * @param  message:  ����ָ��
            length�����ĳ���
  * @retval ��������ṹ��ָ��
  * @remark 

  ********************************************************************************************/
void ZcProtocol_ReceiveHandle(uint8_t *message, uint16_t length, ZcSourceEnum source)
{
  ZcProtocol *protocol = ZcProtocol_Check(message, length);     //�����յĻظ��Ƿ�׼ȷ
  
  if(protocol != NULL)
  {
    ZcProtocol_TimeStamp(protocol->head.timestamp);             //ÿ�ζ�����ʱ���
    
    
    /* ���ݲ�ͬ����Դ���в�ͬ�Ĵ��� */
    switch(source)
    {
    case ZcSource_Net:
      ZcProtocol_NetRxHandle(protocol);                   // �����ͨѶЭ�飬�ؼ����������豸���𣬷�����������Ӧ
      break;
    }
  }
}
/*********************************************************************************************

  * @brief  ����Э�鱨�ĵĴ���
  * @param  message:  ����ָ��
            length�����ĳ���
  * @retval ��������ṹ��ָ��
  * @remark 

  ********************************************************************************************/
void ZcProtocol_NetRxHandle(ZcProtocol *protocol)
{
#ifdef ZC_NET
  FreeTxBlockById(Enthernet_TxBlockList, protocol->head.id);    //ÿ�η������ظ���Ҫ�����Ӧ���ͱ���
#endif
    /* �Ȳ鿴�Ƿ��в����ı���
     �ظ�0������ɹ�������Ҫ��������
     �ظ�1���Ǵ����������ͨѶ�౨�Ĵ���
     �ظ�2������ʧ�ܣ�ֱ�ӷ���ʧ�ָܻ����� */
  uint8_t operationRes = ZcProtocol_NetOperationCmdHandle(protocol);        //�Ȳ鿴�Ƿ��в����ı���

  if(operationRes == 1)
  {
    switch(protocol->head.cmd)                          
    { 
    /* ��ѯ�ݴ汨�� */
    case ZC_CMD_QUERY_HOLD:
      zcHandle.status = ZcHandleStatus_Idle;  // �л�Ϊ����״̬���������Ͳ�ѯ�ݴ汨��
      break;
      
    /* �������·�ȷ�ϱ��� */
    case ZC_CMD_SERVER_CONFIRM:
      /* �����յ�ȷ�ϱ�������֮ǰ���ݴ汨��ID��ͬ */
      if(protocol->head.id == zcHandle.holdId)
      { 
        zcHandle.status = ZcHandleStatus_Idle;  // �л�Ϊ����״̬��������һ�β�ѯ�ݴ汨��
        zcPrtc.head.id++;                       // Id����
      }
           
      /* ����������SFDΪ1ʱ��ֱ�ӷ��Ͳ�ѯ�ݴ汨��,���л�Ϊ�ȴ�״̬*/
      if((protocol->head.control & (1<<7)) != 0)
      { 
        zcHandle.holdId = zcPrtc.head.id;
        ZcProtocol_Transmit(ZcSource_Net, 00, NULL, 0, 0);
        zcHandle.status = ZcHandleStatus_Wait; 
      }   
      break;
      
    /* �ظ������� */
    default:
      ZcProtocol_Transmit(ZcSource_Net, ZC_CMD_FAIL, NULL, 0, 0);
      break;
    }
  }
  else if(operationRes == 2)
  { ZcProtocol_Transmit(ZcSource_Net, ZC_CMD_FAIL, NULL, 0, 0); }        //������ָ��ʧ�ܣ�����ʧ�ܻظ�
}
/*********************************************************************************************

  * @brief  ׾��Э��=������Э�鴦��
  * @param  message��httpЭ��ظ����ݰ��ڵĻظ����ݲ����ַ���
            length�����ĳ���
  * @retval 
  * @remark 

  ********************************************************************************************/
void ZcProtocol_NetPacketHandle(uint8_t *message, uint16_t length)
{
  ArrayStruct *msg = String2Msg(strstr(message, "68"), 0);      // �ַ���ת��������
  
  ZcProtocol_ReceiveHandle(msg->packet, msg->length, ZcSource_Net);  //Э���ԭʼ���Ĵ���
  
  Array_Free(msg);      // �ͷű����������
}
