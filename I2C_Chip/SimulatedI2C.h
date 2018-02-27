#ifndef _SIMULATEDI2C_H_
#define _SIMULATEDI2C_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "SimulatedI2C_HAL.h"

/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/
/* Public macro -------------------------------------------------------------*/
/* Public variables ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/
/* Public functions ---------------------------------------------------------*/

uint8_t SimulatedI2C_ByteWrite(I2C_PinStruct *i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t data);
uint8_t SimulatedI2C_Write(I2C_PinStruct *i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t len);
uint8_t SimulatedI2C_ByteRead(I2C_PinStruct *i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data);
uint8_t SimulatedI2C_Read(I2C_PinStruct *i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t len);

#endif