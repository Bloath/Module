#ifndef _ZCPROTOCOL_API_H_
#define _ZCPROTOCOL_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "../UartDma/SimpleBuffer.h"
#include "ZcProtocol.h"

/* Public typedef ------------------------------------------------------------*/
typedef enum
{
  ZcHandleStatus_Init = 0,
  ZcHandleStatus_PollInterval,
  ZcHandleStatus_Trans,
  ZcHandleStatus_Wait
}ZcProtocolStatus;

typedef struct
{
  ZcProtocolStatus status;
  uint8_t holdId;
  uint32_t loopInterval;
}ZcHandleStruct;

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
extern ZcHandleStruct zcHandle;

/* Public function prototypes ------------------------------------------------*/
void ZcProtocol_Handle();

void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t* address);     //ȫ�ֱ���ZcPrtc��ʼ��������Э��ͷ��һЩ�������ݵ�д��
uint8_t ZcProtocol_Request(ZcSourceEnum source, uint8_t cmd, uint8_t *data, uint16_t dataLen, BoolEnum isUpdateId);         // ��������
void ZcProtocol_Response(ZcSourceEnum source, ZcProtocol *zcProtocol, uint8_t *data, uint16_t dataLen);                 // ���ͻظ�
void ZcProtocol_ReceiveHandle(uint8_t *message, uint16_t length, ZcSourceEnum source);
void ZcProtocol_NetPacketHandle(uint8_t *message, uint16_t length);    // ������մ����Ƚ����Ȼ���ٴ���

#endif