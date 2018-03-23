#ifndef _SOFTI2C_H_
#define _SOFTI2C_H_

/* Includes ------------------------------------------------------------------*/
#include "SoftI2C_HAL.h"

/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/
/* Public macro -------------------------------------------------------------*/
/* Public variables ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/
/* Public functions ---------------------------------------------------------*/

uint8_t SoftI2C_SingleWrite(uint8_t deviceAddr, DATA_TYPE registerAddr, DATA_TYPE data);
uint8_t SoftI2C_MultiWrite(uint8_t deviceAddr, DATA_TYPE registerAddr, DATA_TYPE *data, uint8_t len);
uint8_t SoftI2C_Read(uint8_t deviceAddr, DATA_TYPE registerAddr, DATA_TYPE *data, uint8_t len);

#endif