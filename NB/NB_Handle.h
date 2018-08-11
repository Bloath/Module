#ifndef _NB_HANDLE_H_
#define _NB_HANDLE_H_

/* Includes ------------------------------------------------------------------*/
#include "NB_API.h"

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void NB_ErrorHandle(NbErrorEnum error);
void NB_RxMsgHandle(uint8_t *packet, uint16_t len, void *param);
#endif