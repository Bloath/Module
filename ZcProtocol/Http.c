/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Common/Array.h"
#include "../Common/Convert.h"
#include "../Module/Common/Malloc.h"
#include "ZcProtocol_Conf.h"
#include "Http.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*********************************************************************************************

  * @brief  �ַ���ƴ��
  * @param  src,Դ�ַ���
            str������ƴ�ӵ��ַ���
  * @retval 
  * @remark ��sys_conf�У����л��Ƿ�ʹ�ñ�׼��ĺ궨��

  ********************************************************************************************/
void Str_Concat(char *src, char *str)
{
#ifdef CUSTOM_STANDARD_FUNCTION
  StrCat(src, str);
#else
  strcat(src, str);
#endif
}

/*********************************************************************************************
 * @brief HTTP q����ͷ
 * @para�� ��������
 * @return *httpPacket:����http�����ַ���ָ��
 * @remark 
  ********************************************************************************************/
char* Http_Request(char* string)
{  
  /* �����ڴ� */
  char *httpPacket = (char*)Malloc(HTTP_MAX_LEN);    
  memset(httpPacket, 0, HTTP_MAX_LEN);  
  
  /* ƴ��HTTPЭ�� */
  Str_Concat(httpPacket, "GET ");
  Str_Concat(httpPacket, PATH);
  Str_Concat(httpPacket, "?message=");
  Str_Concat(httpPacket, string);
  Str_Concat(httpPacket, " HTTP/1.1\r\nHost:");
#ifdef DOMAIN
  Str_Concat(httpPacket, DOMAIN);
#else
  Str_Concat(httpPacket, IP);
  Str_Concat(httpPacket, ":");
  Str_Concat(httpPacket, PORT);
#endif
  Str_Concat(httpPacket, "\r\n\r\n");
  
  return httpPacket;
}
/*********************************************************************************************
 * @brief HTTP �ӽ����������ó����ݶ�
 * @para�� packet�����ݰ��ַ���
 * @return *�ַ���
 * @remark 
  ********************************************************************************************/
char* Http_GetResponse(char *packet)
{
  /* ����HTTP��˫����λ�� */
  char * index = strstr(packet, "HTTP");        //���ҵ�HTTP
  index = strstr(index, "\r\n\r\n");           //����˫����
  
  return (index == NULL)? (char*)index:(char*)(index + 4);
}