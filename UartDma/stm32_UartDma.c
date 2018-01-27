/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Module/Common/Malloc.h"
#include "stm32f0xx_hal.h"
#include "stm32_UartDma.h"

/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/

/*********************************************************************************************

  * @brief  基于DMA的串口接收 初始化
  * @param  uartRxDma：dmaStruct结构体
            rxDma：dma底层配置指针
            uart：串口底层配置指针
  * @return 
  * @remark 初始化DMA接收，UART接收，以及开启空闲中断

  ********************************************************************************************/
void Stm32_UartRxDma_Init(UartRxDmaStruct *uartRxDma, DMA_HandleTypeDef *rxDma, UART_HandleTypeDef *uart)
{
  uartRxDma->rxDma = rxDma;
  uartRxDma->uart = uart;
  uartRxDma->bufferLength = BUFFER_LENGTH;
  uartRxDma->start = 0;
  uartRxDma->end = 0;
  
  HAL_DMA_Start_IT(uartRxDma->rxDma, 
                   (uint32_t)&(uartRxDma->uart->Instance->RDR), 
                   (uint32_t)uartRxDma->bufferBlock.rxBuffer.buffer, 
                   uartRxDma->bufferLength);
  
  HAL_UART_Receive_DMA(uartRxDma->uart, uartRxDma->bufferBlock.rxBuffer.buffer, uartRxDma->bufferLength);
  
  uartRxDma->uart->Instance->CR1 |= USART_CR1_IDLEIE;
}
/*********************************************************************************************

  * @brief  基于DMA的串口接收 串口接收处理
  * @param  uartRxDma：dmaStruct结构体
  * @return 
  * @remark 串口空闲中断接收，包含空闲中断标志位的清除

  ********************************************************************************************/
void Stm32_UartRxDma_IntHandle(UartRxDmaStruct *uartRxDma)
{
  uartRxDma->end = uartRxDma->bufferLength - uartRxDma->rxDma->Instance->CNDTR;     //CNDTR是DMA中剩余传输数量
  
  /* 通过判断end与start的位置，进行不同的处理 */
  if(uartRxDma->end > uartRxDma->start)
  { FillRxBlock(uartRxDma->bufferBlock.rxBlockList, 
                uartRxDma->bufferBlock.rxBuffer.buffer + uartRxDma->start, 
                uartRxDma->end - uartRxDma->start); }
  else if(uartRxDma->end < uartRxDma->start)
  {
    uint8_t *message = (uint8_t *)Malloc(uartRxDma->bufferLength - uartRxDma->start + uartRxDma->end );
    
    memcpy(message, 
           uartRxDma->bufferBlock.rxBuffer.buffer + uartRxDma->start,     
           uartRxDma->bufferLength - uartRxDma->start);
    
    memcpy(message + uartRxDma->bufferLength - uartRxDma->start - 1, 
           uartRxDma->bufferBlock.rxBuffer.buffer,     
           uartRxDma->end + 1);
    
    FillRxBlock(uartRxDma->bufferBlock.rxBlockList,
                message, 
                uartRxDma->bufferLength - uartRxDma->start + uartRxDma->end);
    Free(message);
  }
  
  uartRxDma->start = uartRxDma->end; 

  __HAL_UART_CLEAR_IT(uartRxDma->uart, UART_CLEAR_IDLEF);
}
