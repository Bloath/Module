#ifndef _SIMULATEDI2C_HALAPI_H_
#define _SIMULATEDI2C_HALAPI_H_

/* Includes ------------------------------------------------------------------*/
#include "../Common/Delay.h"

/* Public define ------------------------------------------------------------*/

//#define MULTIPLE_I2C    //是否有多个I2C接口


/* 如果有多个I2C接口，则通过多个I2C实例公用同一套读写代码
   如果只有一个I2C接口，则I2C实例指针的部分填写NULL，下面得SDA SCL直接改为对应端口的操作 */
#ifdef MULTIPLE_I2C     

  #include "stm32f0xx_hal.h"

  #define SCL_Set() HAL_GPIO_WritePin(i2cPinStruct->sclPort, i2cPinStruct->scl, GPIO_PIN_SET)
  #define SCL_Clr() HAL_GPIO_WritePin(i2cPinStruct->sclPort, i2cPinStruct->scl, GPIO_PIN_RESET)
  #define SDA_Set() HAL_GPIO_WritePin(i2cPinStruct->sdaPort, i2cPinStruct->sda, GPIO_PIN_SET)
  #define SDA_Clr() HAL_GPIO_WritePin(i2cPinStruct->sdaPort, i2cPinStruct->sda, GPIO_PIN_RESET)
  #define SDA_Status  (HAL_GPIO_ReadPin(i2cPinStruct->sdaPort,i2cPinStruct->sda) == GPIO_PIN_SET)
#else                   
  
  #include "main.h"
  
  #define SCL_Set() HAL_GPIO_WritePin(SEN_CLK_GPIO_Port, SEN_CLK_Pin, GPIO_PIN_SET)
  #define SCL_Clr() HAL_GPIO_WritePin(SEN_CLK_GPIO_Port, SEN_CLK_Pin, GPIO_PIN_RESET)
  #define SDA_Set() HAL_GPIO_WritePin(SEN_DIO_GPIO_Port, SEN_DIO_Pin, GPIO_PIN_SET)
  #define SDA_Clr() HAL_GPIO_WritePin(SEN_DIO_GPIO_Port, SEN_DIO_Pin, GPIO_PIN_RESET)
  #define SDA_Status  (HAL_GPIO_ReadPin(SEN_DIO_GPIO_Port,SEN_DIO_Pin) == GPIO_PIN_SET)
#endif

#define I2C_Delay()  Delay_us(10);

/* Public typedef -----------------------------------------------------------*/

/* 多I2C端口切换用的结构体 
   当为多个I2C时，需要配置成端口+pin的形式，创建实例并通过指针切换不同I2C
   当为单个I2C时，随便用宏定义定义一种类型，因为函数中带入的对应实例指针为NULL*/ 
#ifdef MULTIPLE_I2C

  typedef struct
  {
    GPIO_TypeDef* sclPort;
    uint16_t scl;
    GPIO_TypeDef* sdaPort;
    uint16_t sda;
  }I2C_PinStruct;       
#else
  #define I2C_PinStruct uint8_t
#endif

/* Chip I2C Instance pointer define -----------------------------------------*/

/* 将所有的I2C相关的片子的I2C端口定位都写在这里
   如果为多端口，则创建实例后，将实例指针通过宏定义的方式定义
   如果为单端口，则直接定义为NULL，修改上面的SCL_SET宏定义即可*/

#define DPS310_I2C_POINTER      NULL

#endif
