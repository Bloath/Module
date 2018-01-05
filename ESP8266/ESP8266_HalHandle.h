#ifndef _ESP8266_HALHANDLE_H_
#define _ESP8266_HALHANDLE_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stm32f0xx_hal.h"
#include "SimpleBuffer.h"
/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
#define sysTime HAL_GetTick()

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern TxBlockTypeDef esp8266_TxBlockList[TX_BLOCK_COUNT];                      //模块的发送缓冲
extern RxBlockTypeDef esp8266_RxBlockList[RX_BLOCK_COUNT];                      //模块的接收缓冲

/* Public function prototypes ------------------------------------------------*/
void ESP8266_SendData(uint8_t *data, uint16_t length);
void ESP8266_SendString(const char *string);
void ESP8266_HardWareReset();
void ESP8266_ReceiveDataHandle(uint8_t *data, uint16_t length);
void ESP8266_Test();

#endif