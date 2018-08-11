/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Sys_Conf.h"

#include "Malloc.h"


/* Private define -------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private typedef ------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  ×Ö·û´®²Ã¼ô
  * @param  srcString£ºÔ­Ê¼×Ö·û´®
            strLeft£º×ó±ß½ç×Ö·û´®
            strRight£ºÓÒ±ß½ç×Ö·û´®
  * @retval ÎÞ
  * @remark 

  ********************************************************************************************/
char* String_CutByStr(const char* srcString, const char* strLeft, const char* strRight)
{
  char* left = strstr(srcString, strLeft);
  if(left == NULL)
  { return NULL; }
  
  left += strlen(strLeft);
  
  char *right = strstr(srcString, strRight);
  if(right == NULL)
  { return NULL; }
  
  int8_t temp8 = (uint32_t)right - (uint32_t)left;
  if(temp8 < 0)
  { return NULL; }
  
  char *str = (char *)Malloc(temp8 + 1);
  
  memcpy(str, left , temp8);
  *(uint8_t *)(str + temp8) = 0;
  
  return str;
}
/*********************************************************************************************

  * @brief  ×Ö·û´®²Ã¼ô
  * @param  srcString£ºÔ­Ê¼×Ö·û´®
            strLeft£º×ó±ß½ç×Ö·û´®
            strRight£ºÓÒ±ß½ç×Ö·û´®
  * @retval ÎÞ
  * @remark 

  ********************************************************************************************/
char* String_CutByChr(const char* srcString, char strLeft, char strRight)
{
  char* left = strchr(srcString, strLeft);
  if(left == NULL)
  { return NULL; }
  
  left += 1;
  
  char *right = strchr(srcString, strRight);
  if(right == NULL)
  { return NULL; }
  
  int8_t temp8 = (uint32_t)right - (uint32_t)left;
  if(temp8 < 0)
  { return NULL; }
  
  char *str = (char *)Malloc(temp8 + 1);
  
  memcpy(str, left , temp8);
  *(uint8_t *)(str + temp8) = 0;
  
  return str;
}