/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stm32f0xx_hal.h"
#include "Delay.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define CS_HIGH() 
#define CS_LOW()
#define RD_HIGH()
#define RD_LOW()
#define WR_HIGH()
#define WR_LOW()
#define DATA_HIGH()
#define DATA_LOW()

#define sysTime HAL_GetTick()
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void HT1621B_Write()
{
  CS_HIGH();
  Delay(10);
  CS_LOW();
  
  
  
}