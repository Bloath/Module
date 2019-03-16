#ifndef _DATAMACRO_H_
#define _DATAMACRO_H_

#include "../Module_Conf.h"

// 宏定义中带有返回值的写法，只有GCC编译器支持。

#define CHANGE_RATE_SIGNED(value, base, type, multiple)  (( ((type)value - (type)base) * multiple) / (type)base)    // 计算比例扩大N倍
#define CHANGE_RATE_UNSIGNED(value, base, type, multiple) ({            \
            type temp = (((type)value - (type)base) * multiple) / (type)base;    \
            (*(uint8_t *)(&temp + sizeof(type) - 1) >= 0x80)? (0-temp):temp; })  

#define COMPARE_SIMILAR_SIGNED(value, base, type, multiple, range)      ({      \
            type temp = COMPARE_CHANGE_RATE(value, base, type, multiple);       \
            (temp < range && temp > -range)? true, false; })
#define COMPARE_SIMILAR_UNSIGNED(value, base, type, multiple, range) ({         \
            type temp = COMPARE_CHANGE_RATE(value, base, type, multiple);       \
            (temp < range)? true, false; })

         
#endif