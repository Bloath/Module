#ifndef _DNS_H_
#define _DNS_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "Base.h"

/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
ArrayStruct* Dns_Request(uint16_t id, char *domain);
ArrayStruct* Dns_AnalyseIp(uint8_t *packet, uint16_t length);

#endif