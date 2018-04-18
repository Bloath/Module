#ifndef _ZCPROTOCOL_HANDLE_H_
#define _ZCPROTOCOL_HANDLE_H_

/* Includes ------------------------------------------------------------------*/
#include "../Communicate/Communicate_API.h"
#include "../Sys_Conf.h"
/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void ZcProtocol_HDMaster_UnpollMsgHandle(uint8_t *message, uint16_t len, CommunicateStruct *communicate);
void ZcProtocol_HDSlave_UnpollMsgHandle(uint8_t *message, uint16_t len, CommunicateStruct *communicate);
void ZcProtocol_FD_UnpollMsgHandle(uint8_t *message, uint16_t len, CommunicateStruct *communicate);

#endif