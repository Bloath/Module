#ifndef _ARRAYMACRO_H_
#define _ARRAYMACRO_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"

/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
#define Array_Max(array, length, max)                   \
    {                                                   \
        max = array[0];                                 \
        for (uint16_t __i = 1; __i < length; __i++)     \
        {                                               \
            if (array[__i] > max)                       \
            {                                           \
                max = array[__i];                       \
            }                                           \
        }                                               \
    }

#define Array_Min(array, length, min)                   \
    {                                                   \
        min = array[0];                                 \
        for (uint16_t __i = 1; __i < length; __i++)     \
        {                                               \
            if (array[__i] < min)                       \
            {                                           \
                min = array[__i];                       \
            }                                           \
        }                                               \
    }

#define Array_Sum(array, length, base)                  \
    {                                                   \
        for (uint16_t __i = 0; __i < length; __i++)     \
        {                                               \
            base += array[__i];                         \
        }                                               \
    }

#define Array_MaxIndex(array, length, index)            \
    {                                                   \
        index = 0;                                      \
        for (uint16_t __i = 1; __i < length; __i++)     \
        {                                               \
            if (array[__i] > array[index])              \
            {                                           \
                index = __i;                            \
            }                                           \
        }                                               \
    }

#define Array_SortAsc(array, length)                                    \
    {                                                                   \
        for (uint16_t __i = 0; __i < length; __i++)                     \
        {                                                               \
            for (uint16_t __j = 0; __j < (length - 1 - __i); __j++)     \
            {                                                           \
                if (array[__j] > array[__j + 1])                        \
                {                                                       \
                    array[__j] = array[__j] ^ array[__j + 1];           \
                    array[__j + 1] = array[__j] ^ array[__j + 1];       \
                    array[__j] = array[__j] ^ array[__j + 1];           \
                }                                                       \
            }                                                           \
        }                                                               \
    }

#define Array_SortDesc(array, length)                                   \
    {                                                                   \
        for (uint16_t __i = 0; __i < length; __i++)                     \
        {                                                               \
            for (uint16_t __j = 0; __j < (length - 1 - __i); __j++)     \
            {                                                           \
                if (array[__j] < array[__j + 1])                        \
                {                                                       \
                    array[__j] = array[__j] ^ array[__j + 1];           \
                    array[__j + 1] = array[__j] ^ array[__j + 1];       \
                    array[__j] = array[__j] ^ array[__j + 1];           \
                }                                                       \
            }                                                           \
        }                                                               \
    }

#define Array_Average(array, length, average)           \
    {                                                   \
        average = 0;                                    \
        for (uint16_t __i = 0; __i < length; __i++)     \
        {                                               \
            average += array[__i];                      \
        }                                               \
        average /= length;                              \
    }

#define Array_MiddleAverage(array, length, middleAverage)                               \
    {                                                                                   \
        Array_SortAsc(array, length);                                                   \
        uint16_t quarterLen = ((length % 4) == 0) ? (length / 4) : (length / 4 + 1);    \
        middleAverage = 0;                                                              \
        for (uint16_t __i = quarterLen; __i < (length - quarterLen); __i++)             \
        {                                                                               \
            middleAverage += array[__i];                                                \
        }                                                                               \
        middleAverage /= (length - quarterLen * 2);                                     \
    }

#define Array_Append(array, counter, value) \
    {                                       \
        array[counter] = value;             \
        counter++;                          \
    }

#define Compare_Similarity(v1, v2, rate) (((v1 / v2 - 1) < rate) ? true : false)

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
#endif
