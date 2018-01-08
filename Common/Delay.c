/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stm32f0xx_hal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define sysTime HAL_GetTick()

/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/*******************************************************************************
* Function Name   : Delay_ms
* Description     : ���뼶��ʱ��׼ȷ
* Parameter       : count����ʱʱ��
* Return          : 
*******************************************************************************/
void Delay_ms(uint16_t count)
{
  uint32_t time = 0;
  
  time = sysTime;
  while((time + count) > sysTime);
}

/*******************************************************************************
* Function Name   : Delay_us
* Description     : ΢�뼶��ʱ����׼ȷ
* Parameter       : count����ʱʱ��
* Return          : 
*******************************************************************************/
void Delay_us(uint16_t count)
{
  uint16_t i=0;;
  
  while(count --)
  {
    for(uint16_t j=0; j<48; j++)
    {  i++; }
  }
}
