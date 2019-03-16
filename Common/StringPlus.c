/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Module.h"

#include "Malloc.h"

/* Private define -------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private typedef ------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  字符串裁剪
  * @param  srcString：原始字符串
            strLeft：左边界字符串
            strRight：右边界字符串
  * @retval 返回截取的字符串
  * @remark 

  ********************************************************************************************/
char *String_CutByStr(const char *srcString, const char *strLeft, const char *strRight)
{
    char *left = strstr(srcString, strLeft);        // 查找左边界位置
    if (left == NULL)
    {   return NULL;    }

    left += strlen(strLeft);

    char *right = strstr(srcString, strRight);      // 查找右边界位置
    if (right == NULL)
    {   return NULL;    }

    int8_t temp8 = (uint32_t)right - (uint32_t)left;    // 获取中间字符串长度
    if (temp8 < 0)
    {   return NULL;    }

    char *str = (char *)Malloc(temp8 + 1);
    
    if(str == NULL)
    {   return NULL;    }

    memcpy(str, left, temp8);
    *(uint8_t *)(str + temp8) = 0;

    return str;                                     // 返回截取的字符串
}
/*********************************************************************************************

  * @brief  字符串裁剪
  * @param  srcString：原始字符串
            strLeft：左边界字符串
            strRight：右边界字符串
  * @retval 中间字符串
  * @remark 

  ********************************************************************************************/
char *String_CutByChr(const char *srcString, char leftChar, char rightChar)
{
    char *left = strchr(srcString, leftChar);
    if (left == NULL)
    {   return NULL;    }

    left += 1;

    char *right = strchr(srcString, rightChar);
    if (right == NULL)
    {   return NULL;    }

    int8_t temp8 = (uint32_t)right - (uint32_t)left;
    if (temp8 < 0)
    {   return NULL;    }

    char *str = (char *)Malloc(temp8 + 1);
    
    if(str == NULL)
    {   return NULL;    }

    memcpy(str, left, temp8);
    *(uint8_t *)(str + temp8) = 0;

    return str;
}
