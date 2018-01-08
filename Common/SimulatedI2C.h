#ifndef _SIMULATEDI2C_H_
#define _SIMULATEDI2C_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "stdint.h"
#include "stdlib.h"

/* Public typedef -----------------------------------------------------------*/
typedef struct
{
  GPIO_TypeDef* sclPort;
  uint16_t scl;
  GPIO_TypeDef* sdaPort;
  uint16_t sda;
}I2C_PinStruct;

/* Public define ------------------------------------------------------------*/
/* Public macro -------------------------------------------------------------*/
/* Public variables ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/
/* Public functions ---------------------------------------------------------*/

void SimulatedI2C_Pin_Init(I2C_PinStruct* i2cModule, GPIO_TypeDef * sclPort, uint16_t sclPin, GPIO_TypeDef * sdaPort, uint16_t sdaPin);
uint8_t SimulatedI2C_ByteWrite(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t data);
uint8_t SimulatedI2C_Write(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t len);
uint8_t SimulatedI2C_ByteRead(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data);
uint8_t SimulatedI2C_Read(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t len);

#endif