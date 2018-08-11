/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "../Sys_Conf.h"
#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "../Common/Malloc.h"

#include "../BufferQueue/BufferQueue.h"

#include "Http.h"
#include "ZcProtocol.h"
#include "ZcProtocol_API.h"    
#include "ZcProtocol_Conf.h"   
#include "ZcProtocol_Handle.h"  //Ӳ����ش���


/* private typedef ------------------------------------------------------------*/
/* private define -------------------------------------------------------------*/
/* private macro --------------------------------------------------------------*/
/* private variables ----------------------------------------------------------*/
ZcProtocol zcPrtc;      // ׾��Э��ʵ��
ZcErrorStruct zcError;
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
  * @param  communicate��ͨѶ�ṹ�����
            cmd: ������
            data:  ����ָ��
            dataLen�����ĳ���
            isUpdateId���Ƿ���Ҫ����ID
            txMode������ģʽ
  * @retval �����������id
  * @remark ͨ�����������Լ����ݣ�����д�����ͻ��嵱��

  ********************************************************************************************/
uint8_t ZcProtocol_Request(CommunicateStruct *communicate, uint8_t cmd, uint8_t *data, uint16_t dataLen, BoolEnum isUpdateId, TxModeEnum txMode)
{
  char* httpMsg;
  ArrayStruct *msg;
  uint8_t temp8 = 0;
  
    /* ȡһ����ID */
  if(isUpdateId == TRUE)
  { ZcProtocol_HeadIdIncrease(); }
  temp8 = zcPrtc.head.id;
  
  zcPrtc.head.timestamp = timeStamp;      //����ʱ���
  zcPrtc.head.cmd = cmd;
  
  /* ���ݲ�ͬ��Դ�����в�ͬ�ķ��ʹ��� */
  switch(communicate->medium)
  {
  // ����ͨѶ��ʽ����Ҫ���GET��ʽ��ͷ�����Ұ�HEXת��ΪBCD
  case COM_NET:
    httpMsg = ZcProtocol_ConvertHttpString(&zcPrtc, data, dataLen);         //ת��ΪHTTPЭ�飬
    TxQueue_AddWithId(communicate->txQueue, 
                      (uint8_t*)httpMsg, 
                      strlen(httpMsg), 
                      txMode,
                      zcPrtc.head.id);  
    Free(httpMsg);
    break;
    
  // ���ߡ�����ͨѶ��ʽ������Ҫ��Э�����������ӵȲ��� 
  case COM_24G:
  case COM_485:
    msg = ZcProtocol_ConvertMsg(&zcPrtc, data ,dataLen);
    TxQueue_AddWithId(communicate->txQueue, 
                      msg->packet, 
                      msg->length, 
                      txMode,
                      zcPrtc.head.id);  
    Array_Free(msg);
    break;
  }
  
  /* ȡһ����ID */
  if(isUpdateId == TRUE)
  { ZcProtocol_HeadIdIncrease(); } 
  
  return temp8;
}
/*********************************************************************************************

  * @brief  ׾��Э�� �ظ����ڽ��յ������Ļظ�
  * @param  communicate��ͨѶ�ṹ�����
            zcProtocol�����հ��н������������ݽṹָ��
            data:  ����ָ��
            dataLen�����ĳ���
  * @retval 
  * @remark ������ͬ���ظ�һ�����������ID��CMD�ȣ����������ݲ���������

  ********************************************************************************************/
void ZcProtocol_Response(CommunicateStruct *communicate, ZcProtocol *zcProtocol, uint8_t *data, uint16_t dataLen)
{
  char* httpMsg;
  ArrayStruct *msg;
  
  /* ���ݲ�ͬ��Դ�����в�ͬ�ķ��ʹ��� */
  switch(communicate->medium)
  {
  // ����ͨѶ��ʽ����Ҫ���GET��ʽ��ͷ�����Ұ�HEXת��ΪBCD
  case COM_NET:
    httpMsg = ZcProtocol_ConvertHttpString(zcProtocol, data, dataLen);         //ת��ΪHTTPЭ�飬
    TxQueue_AddWithId(communicate->txQueue, 
                      (uint8_t*)httpMsg, 
                      strlen(httpMsg), 
                      TX_MULTI_MC,
                      zcProtocol->head.id);  
    Free(httpMsg);
    break;
    
  // ���ߡ�����ͨѶ��ʽ������Ҫ��Э�����������ӵȲ���   
  case COM_24G:
  case COM_485:
    msg = ZcProtocol_ConvertMsg(zcProtocol, data ,dataLen);
    TxQueue_AddWithId(communicate->txQueue, 
                      msg->packet, 
                      msg->length, 
                      TX_ONCE_AC,
                      zcProtocol->head.id);  
    Array_Free(msg);
    break;
  }
}

/*********************************************************************************************

  * @brief  ������
  * @param  communicate��ͨѶ�ṹ�����
            txMode������ģʽ
  * @retval 
  * @remark �������־λ�뻺�治ͬʱ�����ϴ�����

  ********************************************************************************************/
void ZcError_Upload(CommunicateStruct *communicate, TxModeEnum txMode)
{
  if(zcError.flag != zcError.flagCache && timeStamp > 152000000)
  {
    /* ����������͵�ǰ���ط�ʱ�� */
    uint8_t *data = (uint8_t *)Malloc(8);
    *(uint32_t *)(data + 0) = zcError.flag;
    *(uint32_t *)(data + 4) = timeStamp;
    
    ZcProtocol_Request(communicate, ZC_CMD_ALARM, data, 8, TRUE, txMode);
    Free(data);  

    zcError.flagCache = zcError.flag;
  }
}

