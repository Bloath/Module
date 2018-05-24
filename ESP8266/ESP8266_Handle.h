#ifndef _ESP8266_HANDLE_H_
#define _ESP8266_HANDLE_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "../BufferQueue/BufferQueue.h"
#include "ESP8266_Handle.h"

/* Public typedef ------------------------------------------------------------*/
typedef enum
{
  ConnectStatus_Init = 0,
  ConnectStatus_Idle,
  ConnectStatus_Reset,
  ConnectStatus_ResetWait,
  ConnectStatus_AirKiss,
  ConnectStatus_AirKissWait,
  ConnectStatus_Connected,
  ConnectStatus_WaitAck,
}ESP8266_ConnectStatusEnum;

/* Public define -------------------------------------------------------------*/
#define ESP8266_INTERVAL 10

#define ESP8266_WIFI_CONNECTED       (1<<0)
#define ESP8266_TCP_CONNECTED        (1<<1)

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern TxQueueStruct Enthernet_TxQueue;                      //模块的发送缓冲
extern RxQueueStruct Enthernet_RxQueue;                      //模块的接收缓冲
extern TxQueueStruct Enthernet_HalTxQueue;
extern ESP8266_ConnectStatusEnum ESP8266_ConnectStatus;
/* Public function prototypes ------------------------------------------------*/

void ESP8266_Handle();
void ESP8266_RxMsgHandle(uint8_t *message, uint16_t length, void *param);
void ESP8266_SendAtString(const char* cmd);

#endif