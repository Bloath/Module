/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "stdio.h"

#include "../Common/Malloc.h"
#include "Convert.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  二进制转字符
  * @param  x：二进制byte
  * @remark 单字节转换

  ********************************************************************************************/
char Hex2Char(uint8_t x)
{
    x &= 0x0F;
    return (x > 9) ? (0x41 + x - 10) : (0x30 + x);
}
/*********************************************************************************************

  * @brief  字符转二进制
  * @param  x：字符
  * @remark 单字节转换

  ********************************************************************************************/
uint8_t Char2Hex(char x)
{
    return (x > 0x40) ? (x - 0x41 + 10) : (x - 0x30);
}
/*********************************************************************************************

  * @brief  10的N次方运算
  * @param  x：字符
  * @remark 单字节转换

  ********************************************************************************************/
uint32_t Math_10nthPower(uint8_t nth)
{
    return (uint32_t)(pow(10, nth) + 0.2);
}
/*********************************************************************************************

  * @brief  字符串转报文
  * @param  string：字符串
            specifyLen：指定长度
            packet：转换完成的数组
  * @remark 转换完成的报文数组指针，需要通过Free释放，错误返回-1

  ********************************************************************************************/
int String2Msg(uint8_t **dst, char *srcStr, uint16_t specifyLen)
{
    int length = (specifyLen == 0) ? strlen(srcStr) : specifyLen;
    uint8_t *msg = (uint8_t *)Malloc(length / 2);
    
    if(msg == NULL)
    {   return -1;  }

    for (uint16_t i = 0; i < (length / 2); i++)
    {
        msg[i] = 0;
        msg[i] |= Char2Hex(srcStr[i * 2]) << 4;
        msg[i] |= Char2Hex(srcStr[i * 2 + 1]);
    }

    *dst = msg;
    return (length / 2);
}
/*********************************************************************************************

  * @brief  报文转字符串
  * @param  dst：目的字符串
            message：报文指针
            length：长度
  * @retval 字符串长度
  * @remark 

  ********************************************************************************************/
int Msg2String(char *dst, uint8_t *message, uint16_t length)
{
    int index = 0, i = 0;
    
    /* 找到字符串末尾 */
    while(dst[index] != '\0')
    {   index++;    }
    
    /* 开始填充 */
    for (i = 0; i < length; i++)
    {
        dst[i * 2 + index] = Hex2Char(message[i] >> 4);
        dst[i * 2 + 1 + index] = Hex2Char(message[i] & 0x0F);
    }

    dst[index + i * 2] = '\0'; //结束符

    return (index + i * 2 - 1);
}
/*********************************************************************************************

  * @brief  uint转字符串
  * @param  dst：目的字符串
            number 数字
  * @retval 字符串长度
  * @remark 

  ********************************************************************************************/
int Uint2String(char *dst, uint32_t number)
{
    int index = 0, i = 0;
    int len = (uint8_t)log10((double)number) + 1;
    uint32_t temp32u;
    
    /* 找到字符串末尾 */
    while(dst[index] != '\0')
    {   index++;    }

    /* 字符串赋值 */
    temp32u = number;                                                               // 缓存number
    for (i = 0; i < len; i++)
    {
        dst[i + index] = Hex2Char(temp32u / (int)Math_10nthPower(len - 1 - i));     // 最高位开始转换
        temp32u = temp32u % (int)Math_10nthPower(len - 1 - i);                      // 记录剩余值
    }

    dst[index + i] = '\0'; //结束符

    return (index + i - 1);
}
/*********************************************************************************************

  * @brief  将内存转换为16进制
  * @param  dst：目的地址
            src：源地址
            length：长度
            isBigEndian：是否为大端
  * @retval 返回幅值之后的指针
  * @remark 

  ********************************************************************************************/
void* Memory2HexString(void *dst, void *src, uint16_t length, bool isBigEndian)
{
    uint8_t value = 0;
    uint8_t *d = (uint8_t *)dst, *s = (uint8_t *)src;
  
    for(int i=0; i<length; i++)
    {
        value = (isBigEndian == true)? s[length - 1 + i] : s[i];
        d[i * 2] = Hex2Char(value >> 4);
        d[i * 2 + 1] = Hex2Char(value);
    }
    
    return (void *)((uint32_t)dst + 2 * length);
}

/*********************************************************************************************

  * @brief  字符串数字转换为无符号整数
  * @param  numString：数字字符串
  * @retval 计算完成的无符号数
  * @remark 

  ********************************************************************************************/
uint32_t NumberString2Uint(const char *numString)
{
    uint32_t temp32u = 0;
    uint8_t len = strlen(numString);

    for (uint8_t i = 0; i < len; i++)
    {
        temp32u += (*(uint8_t *)(numString + i) - 0x30) * Math_10nthPower(len - 1 - i);
    }

    return temp32u;
}

/*********************************************************************************************

  * @brief  数字转换为数字 4321 => {4,3,2,1}
  * @param  dst：存放该数字的数组指针
            number：需要转换的数字
            isPositiveSequence：是否为正序，4321 => {4,3,2,1} 倒序 {1,2,3,4}
  * @retval 数组长度，-1则为申请失败
  * @remark 

  ********************************************************************************************/
int Number2Array(uint8_t **dst, uint32_t number, bool isPositiveSequence)
{
    uint8_t powIndex = (uint8_t)log10(number);
    uint32_t temp32 = number;
    uint32_t remain = 0;

    /* 申请内存并填充 */
    uint8_t *numArray = (uint8_t *)Malloc(powIndex + 1); // 申请对应内存
    
    if(numArray == NULL)
    {   return -1;  }

    for (int8_t i = powIndex; i >= 0; i--)
    {
        if (isPositiveSequence == true)
        {   numArray[powIndex - i] = temp32 / Math_10nthPower(i);   }
        else
        {   numArray[i] = temp32 / Math_10nthPower(i);  }

        remain = Math_10nthPower(i); // pow返回浮点数，有可能99.999转换为99，则加0.1保证
        temp32 %= remain;
    }
    
    *dst = numArray;
    return (powIndex + 1);
}
/*********************************************************************************************

  * @brief  字节顺序互相转换
  * @param  dst：目的指针
  * @param  src：源指针
  * @param  len：长度
  * @retval 
  * @remark 

  ********************************************************************************************/
void* EndianExchange(void *dst, void *src, uint16_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        *((uint8_t *)dst + i) = *((uint8_t *)src + len - i - 1);
    }
    
    return (void *)((uint32_t)dst + len);
}
/*********************************************************************************************

  * @brief  时间戳转日历
  * @param  timeStamp：时间戳
  * @param  calendar：日历指针
            timeZone：时区
  * @retval 
  * @remark 

  ********************************************************************************************/
void TimeStamp2Calendar(uint32_t timeStamp, CalendarStruct *calendar, uint8_t timeZone)
{
    timeStamp += timeZone * SECONDS_HOUR;
    uint32_t daySec = SECONDS_DAY;
    calendar->numOfDay = timeStamp / daySec;
    uint32_t secs = timeStamp % daySec;
    uint16_t years4List[4] = {365, 365, 366, 365};
    uint8_t monthList[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t i = 0;

    uint16_t remainDays = calendar->numOfDay % 1461; // 4年之内的

    calendar->year = 1970 + (calendar->numOfDay / 1461) * 4;

    /************** 年份计算 **************/
    for (i = 0; i < 4; i++)
    {
        if (remainDays <= years4List[i])
        {
            calendar->year += i; // 年增加
            if (i == 2)
            {   monthList[1] = 29;  } // 闰月二月加一天
            else
            {   monthList[1] = 28;  }
            break;
        }
        remainDays -= years4List[i];
    }

    /************** 月日计算 **************/
    for (i = 0; i < 12; i++)
    {
        if ((remainDays + 1) <= monthList[i])
        {
            calendar->month = i + 1;
            calendar->day = remainDays + 1;
            break;
        }
        remainDays -= monthList[i];
    }

    calendar->hour = secs / SECONDS_HOUR;
    calendar->min = (secs % SECONDS_HOUR) / SECONDS_MIN;
    calendar->sec = secs % SECONDS_MIN;
}
/*********************************************************************************************

  * @brief  日历转时间戳
  * @param  calendar：日历指针
            timeZone：时区
  * @retval 
  * @remark 

  ********************************************************************************************/
uint32_t Calendar2TimeStamp(CalendarStruct *calendar, uint8_t timeZone)
{
    uint32_t temp32u = 0;
    uint8_t i = 0;
    uint16_t years4List[4] = {365, 365, 366, 365};
    uint8_t monthList[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    uint8_t temp8u = (calendar->year - 1970L) / 4;
    temp32u += temp8u * 1461L;                  // 先计算出经历过多少个4年
    temp8u = (calendar->year - 1970L) % 4;      // 再单独加上年份
    
    for (i = 0; i < temp8u; i++)
    {   temp32u += years4List[i];   }

    if (temp8u == 2)
    {   monthList[1] = 29;  }                   // 如果为闰年，则2月为29

    for (i = 0; i < (calendar->month - 1); i++) // 累加月份天数
    {   temp32u += monthList[i];    }

    temp32u += (calendar->day - 1);             

    temp32u *= SECONDS_DAY;

    temp32u += calendar->hour * SECONDS_HOUR;
    temp32u += calendar->min * SECONDS_MIN;
    temp32u += calendar->sec;

    temp32u -= timeZone * SECONDS_HOUR;

    return temp32u;
}
