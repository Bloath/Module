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
  strcat(httpPacket, "GET ");
  strcat(httpPacket, PATH);
  strcat(httpPacket, "?message=");
  strcat(httpPacket, string);
  strcat(httpPacket, " HTTP/1.1\r\nHost:");
#ifdef DOMAIN
  strcat(httpPacket, DOMAIN);
#else
  strcat(httpPacket, IP);
  strcat(httpPacket, ":");
  strcat(httpPacket, PORT);
#endif
  strcat(httpPacket, "\r\n\r\n");
  
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