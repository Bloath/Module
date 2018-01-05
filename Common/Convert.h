#ifndef _CONVERT_H_
#define _CONVERT_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "Base.h"

/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
ArrayStruct* String_ConvertMessage(uint8_t *message, uint16_t length);
ArrayStruct* Message_ConvertString(uint8_t *message, uint16_t length);
uint8_t Hex_ConvertString(uint8_t x);

#endif