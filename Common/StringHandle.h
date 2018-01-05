#ifndef _STRINGHANDLE_H_
#define _STRINGHANDLE_H_

/* Includes ------------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/

typedef struct
{
  uint8_t *string;
  uint16_t length;
}StringStruct;

typedef struct
{
  StringStruct **list;
  uint16_t length;
}StringListStruct;

typedef struct
{
  uint8_t string[10];
  uint8_t length;
}IntSringTypeDef;

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
IntSringTypeDef* IntConvertToString(uint32_t number);
float StringConvertToNumber(uint8_t *string, uint16_t stringLength);

uint16_t StringFindIndex(uint8_t *string, uint8_t *message, uint16_t len);
uint8_t StringSameJudge(uint8_t *src, uint8_t *des);
uint8_t StringInclude(uint8_t *src, uint8_t *des, uint16_t desLength);
//
//LinkListBlock* StringSplit(uint8_t *splitSting, uint8_t *des, uint16_t length);
void StringListFree(StringListStruct* stringList);

uint8_t Hex_ConvertString(uint8_t x);

#endif