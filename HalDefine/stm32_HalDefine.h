#ifndef STM32_HALDEFINE_H_
#define STM32_HALDEFINE_H_

#define IO_HIGH(port, pin)      HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
#define IO_LOW(port, pin)       HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define IO_IS_HIGH(port, pin)   (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET)
#define IO_IS_LOW(port, pin)    (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET)

#define UART_DISABLE(uart)  __HAL_UART_DISABLE(uart)
#define UART_ENABLE(uart)   __HAL_UART_ENABLE(uart)

#define DISABLE_ALL_INTERRPUTS() __disable_irq()
#define ENABLE_ALL_INTERRPUTS() __enable_irq()

struct GpioStruct
{
    GPIO_TypeDef* port;
    uint16_t pin;
};

#endif