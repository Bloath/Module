#ifndef _COMMUNICATE_HANDLE_H_
#define _COMMUNICATE_HANDLE_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "../Common/Array.h"

/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void CallBack_UnpollMsgHandle(uint8_t *message, uint16_t len, CommunicateStruct *communicate);
BoolEnum CallBack_HD_isPooling(uint8_t *message, uint16_t len, CommunicateStruct *communicate);
void CallBack_ClearTxQueue(uint8_t *message, uint16_t len, CommunicateStruct *communicate);
BoolEnum CallBack_MessageInspect(uint8_t *message, uint16_t len, CommunicateStruct *communicate);
void CallBack_HDSlave_PollHandle(uint8_t *message, uint16_t len, CommunicateStruct *communicate);
void CallBack_HD_FillPollingPackt(CommunicateStruct *communicate);
ArrayStruct* CallBack_MsgConvert(uint8_t *message, uint16_t len, CommunicateStruct *communicate);

#endif 