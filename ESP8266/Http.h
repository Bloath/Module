#ifndef _HTTP_H_
#define _HTTP_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "Base.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
ArrayStruct* Http_Request(uint8_t *packet, uint16_t length);
ArrayStruct* Http_GetMessage(uint8_t *packet, uint16_t len);

#endif