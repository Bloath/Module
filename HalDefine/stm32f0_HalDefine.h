#ifndef STM32F0_HALDEFINE_H_
#define STM32F0_HALDEFINE_H_

/* include -----------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* define ------------------------------------------------------------*/
/* typedef -----------------------------------------------------------*/
/* macro -------------------------------------------------------------*/
#define IO_HIGH(port, pin)      HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
#define IO_LOw(port, pin)       HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define IO_IS_HIGH(port, pin)   (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET)
#define IO_IS_LOW(port, pin)    (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET)

struct GpioStruct
{
    GPIO_TypeDef* port;
    uint16_t pin;
};

/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*

#endif