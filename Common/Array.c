/* Includes ------------------------------------------------------------------*/

#include "../Common/Malloc.h"
#include "Array.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
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
  ArrayStruct* array = (ArrayStruct* )Malloc(sizeof(ArrayStruct));
  
  array->length = length;
  array->packet = (uint8_t *)Malloc(array->length);
  
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
  Free(array->packet);
  Free(array);
}

