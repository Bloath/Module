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
  strstr(httpPacket, "GET ");
  strstr(httpPacket, PATH);
  strstr(httpPacket, "?message=");
  strstr(httpPacket, string);
  strstr(httpPacket, " HTTP/1.1\r\nHost:");
#ifdef DOMAIN
  strstr(httpPacket, DOMAIN);
#else
  strstr(httpPacket, IP);
  strstr(httpPacket, ":");
  strstr(httpPacket, PORT);
#endif
  strstr(httpPacket, "\r\n\r\n");
  
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