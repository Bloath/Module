#ifndef _DATAMACRO_H_
#define _DATAMACRO_H_

#include "Module/Module_Conf.h"

// 宏定义中带有返回值的写法，只有GCC编译器支持。

#define RATE(value, base, expand)   ((base==0)? 0: ((value * expand) / base))
#define CHANGE_RATE_SIGNED(value, base, expand)  ((base == 0)? (int32_t)expand : ((int32_t)(( ((int32_t)value - (int32_t)base) * (int32_t)expand) / (int32_t)base)))    // 计算比例扩大N倍
#define CHANGE_RATE_UNSIGNED(value, base, expand) ({                  \
            int32_t temp = CHANGE_RATE_SIGNED(value, base, expand);    \
            (FLAG_IS_SET(temp, (1<<31)))? (0-temp):temp; })  

#define COMPARE_SIMILAR_SIGNED(value, base, type, expand, range)      ({      \
            type temp = COMPARE_CHANGE_RATE(value, base, type, expand);       \
            (temp < range && temp > -range)? true, false; })
#define COMPARE_SIMILAR_UNSIGNED(value, base, type, expand, range) ({         \
            type temp = COMPARE_CHANGE_RATE(value, base, type, expand);       \
            (temp < range)? true, false; })

#define IS_SAME_KIND(data1, data2, size)    (((((data1) & (1<<(size-1))) ^ ((data2) & (1<<(size-1)))) & (1<<(size-1))) == 0)
         
#define MAX_SWITCH(value, contrast)                  \
{                                                   \
    if((contrast) > (value))                        \
    {   value = contrast;   }                       \
}
#define MIN_SWITCH(value, contrast)                  \
{                                                   \
    if((contrast) < (value) || (value) == -1)    \
    {   value = contrast;   }                       \
}

#define FLAG_IS_SET(flag, bits)     ((flag & bits) != 0)
#define FLAG_IS_ALL_SET(flag, bits) ((flag & bits) == bits)
#define FLAG_IS_CLR(flag, bits)     ((flag & bits) == 0)
#define FLAG_SET(flag, bits)        {flag |= bits;}
#define FLAG_CLR(flag, bits)        {flag &= ~(bits);}

#endif