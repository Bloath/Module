#ifndef _ZCPROTOCOL_API_H_
#define _ZCPROTOCOL_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "../BufferQueue/BufferQueue.h"
#include "ZcProtocol.h"

/* Public typedef ------------------------------------------------------------*/
typedef enum
{
  ZcHandleStatus_Init = 0,
  ZcHandleStatus_PollInterval,
  ZcHandleStatus_Trans,
  ZcHandleStatus_Wait
}ZcProtocolStatus;

typedef enum
{
  ZcSource_Net = 0,
  ZcSource_24G,
  ZcSource_485
}ZcSourceEnum;

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern ZcProtocol zcPrtc;
extern ZcErrorStruct zcError;
/* Public function prototypes ------------------------------------------------*/
void ZcProtocol_NetTxProcess();
uint32_t ZcProtocol_TimeStamp(uint32_t timeStamp);

void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t* address, uint8_t startId);     //ȫ�ֱ���ZcPrtc��ʼ��������Э��ͷ��һЩ�������ݵ�д��
uint8_t ZcProtocol_Request(ZcSourceEnum source, uint8_t cmd, uint8_t *data, uint16_t dataLen, BoolEnum isUpdateId, TxModeEnum txMode);  // ��������
void ZcProtocol_Response(ZcSourceEnum source, ZcProtocol *zcProtocol, uint8_t *data, uint16_t dataLen);                                 // ���ͻظ�

void ZcError_NetUpload();
#endif