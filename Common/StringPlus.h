#ifndef _STRINGPLUS_H_
#define _STRINGPLUS_H_

/* Includes ------------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
char* String_CutByStr(const char* srcString, const char* strLeft, const char* strRight);
char* String_CutByChr(const char* srcString, char strLeft, char strRight);
#endif