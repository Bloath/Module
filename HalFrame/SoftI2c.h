#ifndef _SOFTI2C_H_
#define _SOFTI2C_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"

/* Public typedef -----------------------------------------------------------*/
struct SoftI2cStruct
{
    struct GpioStruct sda;
    struct GpioStruct scl;
};
/* Public define ------------------------------------------------------------*/
/* Public macro -------------------------------------------------------------*/
#define SCL_SET(i2c)                IO_HIGH(i2c->scl.port, i2c->scl.pin)
#define SCL_CLR(i2c)                IO_LOW(i2c->scl.port, i2c->scl.pin)
#define SDA_SET(i2c)                IO_HIGH(i2c->sda.port, i2c->sda.pin)
#define SDA_CLR(i2c)                IO_LOW(i2c->sda.port, i2c->sda.pin)
#define SDA_STATUS(i2c)             IO_IS_HIGH(i2c->sda.port, i2c->sda.pin)
/* Public variables ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/
/* Public functions ---------------------------------------------------------*/

int SoftI2c_SingleWrite(struct SoftI2cStruct *i2c, uint8_t deviceAddr, uint8_t registerAddr, uint8_t data);
int SoftI2c_MultiWrite(struct SoftI2cStruct *i2c, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t len);
int SoftI2c_Read(struct SoftI2cStruct *i2c, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t len);

#endif
