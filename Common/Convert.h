#ifndef _CONVERT_H_
#define _CONVERT_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "Array.h"

/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/
typedef struct
{
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
}CalendarStruct;
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
ArrayStruct* String2Msg(char* string, uint16_t specifyLen);
char* Msg2String(uint8_t *message, uint16_t length);
char* Uint2String(uint32_t number);
uint32_t NumberString2Uint(const char* numString);
ArrayStruct* Number2Array(uint32_t number, BoolEnum isPositiveSequence);
void EndianExchange(uint8_t* dst, uint8_t* src, uint8_t len);

void TimeStamp2Calendar(uint32_t timeStamp, CalendarStruct *calendar, uint8_t timeZone);
uint32_t Calendar2TimeStamp(CalendarStruct *calendar, uint8_t timeZone);
#endif