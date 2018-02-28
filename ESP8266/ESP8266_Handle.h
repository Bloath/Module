#ifndef _ESP8266_H_
#define _ESP8266_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "ESP8266_Handle.h"

/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
#define ESP8266_INTERVAL 5

#define ESP8266_WIFI_CONNECTED       (1<<0)
#define ESP8266_TCP_CONNECTED        (1<<1)

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern TxBlockTypeDef Enthernet_TxBlockList[TX_BLOCK_COUNT];                      //模块的发送缓冲
extern RxBlockTypeDef Enthernet_RxBlockList[RX_BLOCK_COUNT];                      //模块的接收缓冲

/* Public function prototypes ------------------------------------------------*/

void ESP8266_Handle();
void ESP8266_RxMsgHandle(uint8_t *message, uint16_t length);
void ESP8266_SendAtString(const char* cmd);

#endif