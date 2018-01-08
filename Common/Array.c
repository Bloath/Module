/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "Array.h"
#include "stm32f0xx_hal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define sysTime HAL_GetTick()

/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  �½��б�
  * @param  length:  ����̶�
  * @retval �б�ṹ��ͷָ��
  * @remark 

  ********************************************************************************************/
ArrayStruct* Array_New(uint16_t length)
{
  ArrayStruct* array = (ArrayStruct* )malloc(sizeof(ArrayStruct));
  
  array->length = length;
  array->packet = (uint8_t *)malloc(array->length);
  
  return array;
}
/*********************************************************************************************

  * @brief  �ͷ��б�
  * @param  array:  �б�ָ��
  * @retval 
  * @remark 

  ********************************************************************************************/
void Array_Free(ArrayStruct* array)
{
  free(array->packet);
  free(array);
}


