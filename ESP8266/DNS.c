
/**
  ******************************************************************************
  * @file    DNS.c
  * @author  bloath young
  * @brief   DNSЭ�飬�����򵥵Ļ�ȡ�Լ��ظ�����
  *
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
    [..]
      DNS������ͨ��UDPЭ����DNS��������53�˿ڷ����������ݣ�������������д��������
    �ڷ������ظ��������оͰ����˸�������IP��ַ
    [..]
         (+) uint8_t* Dns_Request(uint16_t id, char *domain)

             ʹ�øú����������ͷ��idΪ���ID����������д��domianΪ������д����
         ��������ʽ������ Dns_Request(1111, "www.baidu.com")������һ���ַ������ݣ�
         ��ʹ����Ϻ���Ҫͨ��free�ͷ�

         (+) uint8_t* Dns_AnalyseIp(uint8_t *packet, uint16_t length)

              ʹ�øú���������������ָIP������4�ֽ��ַ���


  @endverbatim
**/

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "SimpleBuffer.h"
#include "Base.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

typedef struct
{
  uint16_t id;
  uint16_t flag;
  uint16_t questionCount;
  uint16_t answerCount;
  uint16_t authorityCount;
  uint16_t addtionnalCount;  
}DnsHeadStruct;

typedef struct
{
  uint16_t questionType;
  uint16_t questionClass;
}DnsQuestionStruct;

typedef struct
{
  DnsHeadStruct head;
  uint8_t *questionName;
  DnsQuestionStruct question;
}DnsStruct;


/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
uint8_t* LittleToBig_HalfWord(uint8_t *packet, uint16_t length);

/*********************************************************************************************
 * @brief  DNS����ͷ
 * @para�� id�������
           domain�������ַ���������ֱ������"www.baidu.com"
 * @return *dns������һ������ָ�룬�ǵ����ͨ��free�ͷŵ�
 * @remark 
  ********************************************************************************************/
ArrayStruct* Dns_Request(uint16_t id, char *domain)
{
  uint16_t domainLen = strlen((char const *)domain);        // ��������
  
  /* ��ȡ���ݰ��ܳ��ȣ����������ֽ�����������ǰ��ǰ��һ���ֽ��ǳ��ȣ������ǽ����� */
  ArrayStruct* dns = Array_New(sizeof(DnsQuestionStruct) + sizeof(DnsHeadStruct) + domainLen + 2);
  
  /* dns��ͷ��ʼ�� */  
  DnsHeadStruct dnsHead = {0};
  
  dnsHead.id = id;
  dnsHead.flag = 0x0100;
  dnsHead.questionCount = 1;
  dnsHead.answerCount = 0;
  dnsHead.authorityCount = 0;
  dnsHead.addtionnalCount = 0;
  
  /* dns question ��ʼ�� */ 
  DnsQuestionStruct dnsQuestion = {0};
  
  dnsQuestion.questionType = 1;
  dnsQuestion.questionClass = 1;
  
  /* �������Ĳ�ͬ����ת��Ϊ���Ⱥ��ַ� */ 
  uint8_t *domainTemp = (uint8_t *)malloc(domainLen + 2);     // �����ڴ�
  
  CopyPacket((uint8_t *)domain, domainTemp + 1, domainLen);        // ���������Ƶ����У�Ԥ��ǰ���ֽ�
  
  uint8_t lengthPointer = 0;          // �޸ĳ����õ�ָ��                      
  
  for(uint8_t i=0; i<domainLen + 1; i++)
  {
    if(domainTemp[i] == '.' || i == domainLen)
    {
      uint8_t temp = (i == domainLen)? 0 : 1;             // Ϊʲô����ֱ��д�����棬��Ҫ��дһ����ʱ����
//      domainTemp[lengthPointer] = i - lengthPointer - (i == (domainLen + 1))? 0 : 1;
      domainTemp[lengthPointer] = i - lengthPointer - temp;
      lengthPointer = i;
    }
  }
  domainTemp[domainLen + 1] = 0; // ��ӽ�����
  
  /* ��������ͷ */
  uint8_t* dnsHeaderTemp = LittleToBig_HalfWord((uint8_t *)&dnsHead, sizeof(DnsHeadStruct));
  CopyPacket(dnsHeaderTemp, dns->packet, sizeof(DnsHeadStruct));
  free(dnsHeaderTemp);
  
  
  /* ����������������� */
  CopyPacket(domainTemp, dns->packet + sizeof(DnsHeadStruct), domainLen + 2);
  
  /* �������������type��class */
  uint8_t* dnsQuestionPara = LittleToBig_HalfWord((uint8_t *)&dnsQuestion, sizeof(DnsQuestionStruct));
  CopyPacket(dnsQuestionPara, dns->packet + sizeof(DnsHeadStruct) + domainLen + 2, sizeof(DnsQuestionStruct));
  free(dnsQuestionPara);
  
  free(domainTemp);
    
  return dns;
}
/*********************************************************************************************
 * @brief  ��ȡIp
 * @para�� packet�����ݰ�ָ��
           length�����ݰ�����
 * @return ip 4�ֽ����飬�ǵ�free
 * @remark 
  ********************************************************************************************/
ArrayStruct* Dns_AnalyseIp(uint8_t *packet, uint16_t length)
{
  ArrayStruct* ip = Array_New(4);
  
  CopyPacket(packet + (length - 4), ip->packet, 4);
  
  return ip;
}

/*********************************************************************************************
 * @brief  С��ת��ˣ�����
 * @para�� packet����Ҫת����ָ��
           length����Ҫת�����ܳ��ȣ�����Ϊ2�ı���
 * @return *result������һ�����飬�ǵ�free��
 * @remark 
  ********************************************************************************************/
uint8_t* LittleToBig_HalfWord(uint8_t *packet, uint16_t length)
{
  uint8_t *result = (uint8_t *)malloc(length);
  
  for(int i=0; i<length; i=i+2)
  {
    result[i] = packet[i + 1];
    result[i + 1] = packet[i];
  }
  
  return result;
}