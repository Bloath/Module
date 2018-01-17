#ifndef _PROTOCOLHANDLE_H_
#define _PROTOCOLHANDLE_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "../UartDma/SimpleBuffer.h"

/* Public typedef ------------------------------------------------------------*/
typedef enum
{
  ZcHandleStatus_Init = 0,
  ZcHandleStatus_Idle,
  ZcHandleStatus_Wait
}ZcProtocolStatus;

typedef struct
{
  ZcProtocolStatus status;
  uint8_t holdId;
  uint32_t loopInterval;
}ZcHandleStruct;

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern ZcProtocol zcPrtc;
extern TxBlockTypeDef Enthernet_TxBlockList[TX_BLOCK_COUNT];                      //ģ��ķ��ͻ���
extern RxBlockTypeDef Enthernet_RxBlockList[RX_BLOCK_COUNT];                      //ģ��Ľ��ջ���

/* Public function prototypes ------------------------------------------------*/
void ZcProtocol_Handle();

void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t* address);     //ȫ�ֱ���ZcPrtc��ʼ��������Э��ͷ��һЩ�������ݵ�д��
void ZcProtocol_NetTransmit(uint8_t cmd, uint8_t *data, uint16_t dataLen, uint8_t isUpdateId);  //Э������緢�ʹ���

void ZcProtocol_NetReceiveHandle(uint8_t *message, uint16_t length);    // ������մ����Ƚ����Ȼ���ٴ���

#endif