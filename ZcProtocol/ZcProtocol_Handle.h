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
extern TxBlockTypeDef Enthernet_TxBlockList[TX_BLOCK_COUNT];                      //模块的发送缓冲
extern RxBlockTypeDef Enthernet_RxBlockList[RX_BLOCK_COUNT];                      //模块的接收缓冲

/* Public function prototypes ------------------------------------------------*/
void ZcProtocol_Handle();

void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t* address);     //全局变量ZcPrtc初始化，用于协议头部一些常用数据的写入
void ZcProtocol_NetTransmit(uint8_t cmd, uint8_t *data, uint16_t dataLen, uint8_t isUpdateId);  //协议的网络发送处理

void ZcProtocol_NetReceiveHandle(uint8_t *message, uint16_t length);    // 网络接收处理，先解包，然后再处理

#endif