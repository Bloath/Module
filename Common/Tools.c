/* Includes ------------------------------------------------------------------*/
#include "Tools.h"
#include "stdarg.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  ms级别延时
  * @param  count 
  * @retval 无
  * @remark 

  ********************************************************************************************/
void Delay_ms(uint16_t count)
{
    uint32_t time = 0;

    time = SYSTIME;
    while ((time + count) > SYSTIME)
        ;
}

/*********************************************************************************************

  * @brief  us级别延时
  * @param  count 
  * @retval 无
  * @remark 

  ********************************************************************************************/
void Delay_us(uint16_t count)
{
    while (count--)
    {
        for (uint16_t j = 0; j < (CPU_1US_COUNT >> 2); j++)
        {   ;   }
    }
}
/*********************************************************************************************

  * @brief  NB接收部分处理
  * @param  log：log结构体
            format：格式
  * @retval 无
  * @remark 

  ********************************************************************************************/
void Log(struct LogStruct *log, const char *format, ...)
{
	char logContent[200] = {0};
    if(log->CallBack_Transmit == NULL)
    {   return; }
    
    if(log->calendar != NULL)
    {
        TimeStamp2Calendar(TIMESTAMP, log->calendar, 8);
        sprintf(logContent, "[%02d:%02d:%02d]", log->calendar->hour, log->calendar->min, log->calendar->sec);
    }
	
	/* 读取可变参数，先进行格式化便携 */
	va_list args;
	va_start(args, format);
	vsprintf(logContent + strlen(logContent), format, args);
	va_end(args);
    
    log->CallBack_Transmit((uint8_t *)logContent, strlen(logContent));
}

