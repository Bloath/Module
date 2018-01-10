#ifndef _PROTOCOLHANDLE_H_
#define _PROTOCOLHANDLE_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "ZcProtoCOl.h"

/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void ZcProtocol_ReponseHandle(uint8_t *message, uint16_t length);
void ZcProtocol_NetReceiveHandle(uint8_t *message, uint16_t length);
void ZcProtocol_Init();

#endif