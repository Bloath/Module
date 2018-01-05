/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "Base.h"
#include "stm32f0xx_hal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define sysTime HAL_GetTick()

/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  新建列表
  * @param  length:  数组程度
  * @retval 列表结构体头指针
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

  * @brief  释放列表
  * @param  array:  列表指针
  * @retval 
  * @remark 

  ********************************************************************************************/
void Array_Free(ArrayStruct* array)
{
  free(array->packet);
  free(array);
}
/*********************************************************************************************

  * @brief  延时
  * @param  count
  * @retval 
  * @remark 

  ********************************************************************************************/
void Delay(uint16_t count)
{
  uint32_t time = 0;
  
  time = sysTime;
  while((time + count) > sysTime);
}

