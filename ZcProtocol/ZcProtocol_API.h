#ifndef _ZCPROTOCOL_API_H_
#define _ZCPROTOCOL_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "../BufferQueue/BufferQueue.h"
#include "../Communicate/Communicate_API.h"
#include "ZcProtocol.h"

/* Public typedef ------------------------------------------------------------*/
typedef enum
{
  ZcHandleStatus_Init = 0,
  ZcHandleStatus_PollInterval,
  ZcHandleStatus_Trans,
  ZcHandleStatus_Wait
}ZcProtocolStatus;


/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern ZcProtocol zcPrtc;
extern ZcErrorStruct zcError;
/* Public function prototypes ------------------------------------------------*/
void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t* address, uint8_t startId);     //ȫ�ֱ���ZcPrtc��ʼ��������Э��ͷ��һЩ�������ݵ�д��

// ���������ϴ�����ʹ�øú���
uint8_t ZcProtocol_Request(CommunicateStruct *communicate, 
                           uint8_t cmd, 
                           uint8_t *data, 
                           uint16_t dataLen, 
                           BoolEnum isUpdateId,
                           TxModeEnum txMode);  

// �ڴ�����ձ���ʱ��ʹ��reponse���лظ�
void ZcProtocol_Response(CommunicateStruct *communicate, 
                         ZcProtocol *zcProtocol, 
                         uint8_t *data, uint16_t 
                           dataLen);                                 

void ZcError_Upload(CommunicateStruct *communicate, TxModeEnum txMode);
#endif