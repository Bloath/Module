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
void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t* address, uint8_t startId);     //全局变量ZcPrtc初始化，用于协议头部一些常用数据的写入

// 主动进行上传处理，使用该函数
uint8_t ZcProtocol_Request(CommunicateStruct *communicate, 
                           uint8_t cmd, 
                           uint8_t *data, 
                           uint16_t dataLen, 
                           BoolEnum isUpdateId,
                           TxModeEnum txMode);  

// 在处理接收报文时，使用reponse进行回复
void ZcProtocol_Response(CommunicateStruct *communicate, 
                         ZcProtocol *zcProtocol, 
                         uint8_t *data, uint16_t 
                           dataLen);                                 

void ZcError_Upload(CommunicateStruct *communicate, TxModeEnum txMode);
#endif