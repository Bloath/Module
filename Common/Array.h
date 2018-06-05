#ifndef _ARRAY_H_
#define _ARRAY_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"

/* Public typedef ------------------------------------------------------------*/
typedef struct
{
  uint8_t *packet;
  uint16_t length;
}ArrayStruct;

/* Public define -------------------------------------------------------------*/
#define Array_Max(array, length, max)      \
{\
  *max = array[0];\
  for(uint16_t i=1; i<length; i++)\
  { \
    if(array[i] > *max)\
    { *max = array[i]; }\
  }\
}

#define Array_Min(array, length, min)      \
{\
  *min = array[0];\
  for(uint16_t i=1; i<length; i++)\
  { \
    if(array[i] < *min)\
    { *min = array[i]; }\
  }\
}  
                                                    
#define Array_SortAsc(array, length)        \
{\
  for(uint16_t i=0; i<length; i++)\
  { \
    for(uint16_t j=0; j<(length - 1 - i); j++)\
    {\
      if(array[j] > array[j + 1])\
      { \
        array[j] = array[j] ^ array[j + 1];\
        array[j + 1] = array[j] ^ array[j + 1];\
        array[j] = array[j] ^ array[j + 1];\
      }\
    }\
  }\
}
                                                
#define Array_SortDesc(array, length)      \
{\
  for(uint16_t i=0; i<length; i++)\
  { \
    for(uint16_t j=0; j<(length - 1 - i); j++)\
    {\
      if(array[j] < array[j + 1])\
      { \
        array[j] = array[j] ^ array[j + 1];\
        array[j + 1] = array[j] ^ array[j + 1];\
        array[j] = array[j] ^ array[j + 1];\
      }\
    }\
  }\
}

#define Array_Average(array, length, average)      \
{\
  average = 0;\
  for(uint16_t i=0; i<length; i++)\
  { average += array[i]; }\
  average /= length;\
}

#define Array_MiddleAverage(array, length, middleAverage) \
{\
  Array_SortAsc(array, length);\
  uint16_t quarterLen = ((length % 4) == 0)? (length / 4):(length / 4 + 1); \
  middleAverage = 0;\
  for(uint16_t i=quarterLen; i<(length - quarterLen); i++)\
  { middleAverage += array[i]; }\
  middleAverage /= (length - quarterLen * 2);\
}

#define Array_Append(array, counter, value) \
{\
  array[counter] = value;\
  counter ++;\
}

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
ArrayStruct* Array_New(uint16_t length);
void Array_Free(ArrayStruct* array);
void Delay(uint16_t count);

#endif