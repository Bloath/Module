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

  * @brief  ����DMA�Ĵ��ڽ��� ��ʼ��
  * @param  uartRxDma��dmaStruct�ṹ��
            rxDma��dma�ײ�����ָ��
            uart�����ڵײ�����ָ��
  * @return 
  * @remark ��ʼ��DMA���գ�UART���գ��Լ����������ж�

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

  * @brief  ����DMA�Ĵ��ڽ��� ���ڽ��մ���
  * @param  uartRxDma��dmaStruct�ṹ��
  * @return 
  * @remark ���ڿ����жϽ��գ����������жϱ�־λ�����

  ********************************************************************************************/
void Stm32_UartRxDma_IntHandle(UartRxDmaStruct *uartRxDma)
{
  uartRxDma->end = uartRxDma->bufferLength - uartRxDma->rxDma->Instance->CNDTR;     //CNDTR��DMA��ʣ�ഫ������
  
  /* ͨ���ж�end��start��λ�ã����в�ͬ�Ĵ��� */
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
