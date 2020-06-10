#ifndef _CONVERT_H_
#define _CONVERT_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"

/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/
struct CalendarStruct
{
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t min;
    int32_t sec;
    uint32_t numOfDay;
};
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
int String2Msg(uint8_t **dst, char *srcStr, uint16_t specifyLen);
int HexString2Msg(uint8_t **dst, char *srcStr, uint16_t specifyLen);
int Msg2String(char *dst, uint8_t *message, uint16_t length);
int Msg2HexString(char *dst, uint8_t *message, uint16_t length);
void StringCat(char *dst, char *src);
void* Memory2HexString(void *dst, void *src, uint16_t length, bool isBigEndian);
int Uint2String(char *dst, uint32_t number);

uint32_t NumberString2Uint(const char *numString);
int Number2Array(uint8_t **dst, uint32_t number, bool isPositiveSequence);
void* EndianExchange(void *dst, void *src, uint16_t len);

void TimeStamp2Calendar(uint32_t time, struct CalendarStruct *calendar, uint8_t timeZone);
uint32_t Calendar2TimeStamp(struct CalendarStruct *calendar, uint8_t timeZone);
#endif
