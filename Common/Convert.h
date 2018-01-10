#ifndef _CONVERT_H_
#define _CONVERT_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "Array.h"

/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
ArrayStruct* String2Msg(char* string);
char* Msg2String(uint8_t *message, uint16_t length);
char* Uint2String(uint32_t number);

#endif