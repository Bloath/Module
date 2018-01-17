/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "ZcProtocol.h"

//�ڴ���������Բ�ͬ��������ӵ�ͷ
#include "../ESP8266/ESP8266.h"
#include "../UartDma/SimpleBuffer.h"
#include "../Sys_Conf.h"
#include "Http.h"
#include "ZcProtocol_Handle.h"

/* private typedef ------------------------------------------------------------*/
/* private define -------------------------------------------------------------*/
/* private macro --------------------------------------------------------------*/
/* private variables ----------------------------------------------------------*/
ZcProtocol zcPrtc;      // ׾��Э��ʵ��
ZcHandleStruct zcHandle = {0};      //׾��Э�鴦��
extern TxBlockTypeDef Enthernet_TxBlockList[TX_BLOCK_COUNT];     //����Э�鷢�ͻ���
extern RxBlockTypeDef Enthernet_RxBlockList[RX_BLOCK_COUNT];     //����Э����ջ���

/* private function prototypes ------------------------------------------------*/
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
    time = sysTime;                     //��¼��ʱ��ϵͳʱ��
    return 0;                           //����0
  }
  else
  { return zcPrtc.head.timestamp + (sysTime - time) / 1000; } //��ȡʱ�����������֮ǰ��¼��ʱ����Ļ����ϣ����Ϻ����ܹ���ϵͳʱ��
    
}

/*********************************************************************************************

  * @brief  ׾��Э�� ����ͨѶ���Ͳ���
  * @param  cmd: ������
            data:  ����ָ��
            dataLen�����ĳ���
            isUpdateId���Ƿ���Ҫ����ID
  * @retval 
  * @remark ͨ�����������Լ����ݣ�����д�����ͻ��嵱��

  ********************************************************************************************/
void ZcProtocol_NetTransmit(uint8_t cmd, uint8_t *data, uint16_t dataLen, uint8_t isUpdateId)
{
  zcPrtc.head.timestamp = ZcProtocol_TimeStamp(0);      //����ʱ���
  zcPrtc.head.cmd = cmd;
  
  char* httpMsg = ZcProtocol_ConvertHttpString(&zcPrtc, data, dataLen);         //ת��ΪHTTPЭ�飬
  FillTxBlock(Enthernet_TxBlockList, (uint8_t*)httpMsg, strlen(httpMsg), TX_FLAG_MC|TX_FLAG_RT);        //��д�����緢�ͻ��嵱��
  free(httpMsg);
  
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
  if((time + zcHandle.loopInterval) > sysTime)
  { return; }
  else
  { time = sysTime; }
  
  /* ������״̬���д��� */
  switch(zcHandle.status)
  {
    /* ��ʼ�� */
  case ZcHandleStatus_Init:
    zcHandle.loopInterval = 60000;
    zcHandle.status = ZcHandleStatus_Idle;
    break;
    
    /* ����״̬������ѯ�ݴ汨�ģ��л�Ϊ�ȴ�״̬ */
  case ZcHandleStatus_Idle:
    ZcProtocol_NetTransmit(00, NULL, 0, 0);     //�����ݴ汨��
    zcHandle.holdId = zcPrtc.head.id;           //��¼��ǰID
    zcHandle.status = ZcHandleStatus_Wait;      //�л��ȴ�״̬
    break;
    
    /* �ȴ�״̬�����ͻ���Ϊ�ֶ���������û�лظ��Ļ��ǲ�������� 
       �ȴ�״̬Ϊ������ֱ�����յ�ȷ�ϱ��Ļ��߲�ѯ�ݴ�ظ�*/
  case ZcHandleStatus_Wait:
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
void ZcProtocol_ReceiveHandle(uint8_t *message, uint16_t length)
{
  ZcProtocol *protocol = ZcProtocol_Check(message, length);     //�����յĻظ��Ƿ�׼ȷ
  
  if(protocol != NULL)
  {
    ZcProtocol_TimeStamp(protocol->head.timestamp);             //ÿ�ζ�����ʱ���
    ClearSpecifyBlock(Enthernet_TxBlockList, ZcProtocol_SameId, &protocol->head.id);    //ÿ�η������ظ���Ҫ�����Ӧ����
    
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
      break;
    
    /* ��ַ�� */
    case ZC_CMD_ADDRESS:
      ZcProtocol_NetTransmit(ZC_CMD_ADDRESS, zcPrtc.head.address, 7, 0);
      break;
    
//    /* �豸������� */
//    case ZC_CMD_DEVICE_ATTR:
//      
//      break;
//      
//    /* ����������� */
//    case ZC_CMD_MEASURE_ATTR:
//      
//      break;
//    
//    /* ���ݷ��� */
//    case ZC_CMD_LADIR_PRICE:
//      
//      break;
//    
//    /* ��ֵ��¼ */
//    case ZC_CMD_RECHARGE:
//      
//      break;
//    
//    /* ������ʷ��¼ */
//    case ZC_CMD_USE_HISTORY:
//      
//      break;
//      
//    /* ������Ϣ */
//    case ZC_CMD_ALARM:
//      
//      break;
//      
//    /* �������� */
//    case ZC_CMD_ENVIROMENT:
//      
//      break;
//      
//    /* �ܵ�״̬ */
//    case ZC_CMD_PIPE_STATUS:
//      
//      break;
//      
//    /* ���ſ��ؼ�¼ */
//    case ZC_CMD_VALVE_RECORD:
//      
//      break;
//      
//    /* ���ſ��ز��� */
//    case ZC_CMD_VALVE_OPRT:
//      
//      break;
      
    /* �ظ������� */
    default:
      ZcProtocol_NetTransmit(ZC_CMD_FAIL, NULL, 0, 0);
      break;
    }
    
    
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
