#ifndef _CONVERT_H_
#define _CONVERT_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "Array.h"

/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
ArrayStruct* String2Msg(char* string, uint16_t specifyLen);
char* Msg2String(uint8_t *message, uint16_t length);
char* Uint2String(uint32_t number);
ArrayStruct* Number2Array(uint32_t number, BoolEnum isPositiveSequence);
void EndianExchange(uint8_t* dst, uint8_t* src, uint8_t len);

#endif