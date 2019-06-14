#ifndef _CONVERT_H_
#define _CONVERT_H_

/* Includes ------------------------------------------------------------------*/
#include "../Module.h"

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
    uint32_t numOfDay;
} CalendarStruct;
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
int String2Msg(uint8_t **dst, char *srcStr, uint16_t specifyLen);
int HexString2Msg(uint8_t **dst, char *srcStr, uint16_t specifyLen);
int Msg2String(char *dst, uint8_t *message, uint16_t length);
int Msg2HexString(char *dst, uint8_t *message, uint16_t length);
int Uint2String(char *dst, uint32_t number);
void* Memory2HexString(void *dst, void *src, uint16_t length, bool isBigEndian);

uint32_t NumberString2Uint(const char *numString);
int Number2Array(uint8_t **dst, uint32_t number, bool isPositiveSequence);
void* EndianExchange(void *dst, void *src, uint16_t len);

void TimeStamp2Calendar(uint32_t time, CalendarStruct *calendar, uint8_t timeZone);
uint32_t Calendar2TimeStamp(CalendarStruct *calendar, uint8_t timeZone);
#endif
