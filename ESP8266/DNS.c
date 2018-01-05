
/**
  ******************************************************************************
  * @file    DNS.c
  * @author  bloath young
  * @brief   DNS协议，包含简单的获取以及回复解析
  *
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
    [..]
      DNS请求是通过UDP协议向DNS服务器的53端口发送请求数据，在请求数据中写入域名，
    在服务器回复的数据中就包含了该域名的IP地址
    [..]
         (+) uint8_t* Dns_Request(uint16_t id, char *domain)

             使用该函数打包请求头，id为随机ID，可任意填写，domian为域名，写成字
         符串的形式，例如 Dns_Request(1111, "www.baidu.com")，返回一个字符串数据，
         在使用完毕后，需要通过free释放

         (+) uint8_t* Dns_AnalyseIp(uint8_t *packet, uint16_t length)

              使用该函数解析出域名所指IP，返回4字节字符串


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
 * @brief  DNS请求头
 * @para： id：随机数
           domain：域名字符串，可以直接填入"www.baidu.com"
 * @return *dns，返回一个报文指针，记得最后通过free释放掉
 * @remark 
  ********************************************************************************************/
ArrayStruct* Dns_Request(uint16_t id, char *domain)
{
  uint16_t domainLen = strlen((char const *)domain);        // 域名长度
  
  /* 获取数据包总长度，多了两个字节是在域名的前后，前面一个字节是长度，后面是结束符 */
  ArrayStruct* dns = Array_New(sizeof(DnsQuestionStruct) + sizeof(DnsHeadStruct) + domainLen + 2);
  
  /* dns包头初始化 */  
  DnsHeadStruct dnsHead = {0};
  
  dnsHead.id = id;
  dnsHead.flag = 0x0100;
  dnsHead.questionCount = 1;
  dnsHead.answerCount = 0;
  dnsHead.authorityCount = 0;
  dnsHead.addtionnalCount = 0;
  
  /* dns question 初始化 */ 
  DnsQuestionStruct dnsQuestion = {0};
  
  dnsQuestion.questionType = 1;
  dnsQuestion.questionClass = 1;
  
  /* 将域名的不同部分转换为长度和字符 */ 
  uint8_t *domainTemp = (uint8_t *)malloc(domainLen + 2);     // 申请内存
  
  CopyPacket((uint8_t *)domain, domainTemp + 1, domainLen);        // 将域名复制到其中，预留前后字节
  
  uint8_t lengthPointer = 0;          // 修改长度用的指针                      
  
  for(uint8_t i=0; i<domainLen + 1; i++)
  {
    if(domainTemp[i] == '.' || i == domainLen)
    {
      uint8_t temp = (i == domainLen)? 0 : 1;             // 为什么不能直接写在里面，需要先写一个临时变量
//      domainTemp[lengthPointer] = i - lengthPointer - (i == (domainLen + 1))? 0 : 1;
      domainTemp[lengthPointer] = i - lengthPointer - temp;
      lengthPointer = i;
    }
  }
  domainTemp[domainLen + 1] = 0; // 添加结束符
  
  /* 复制请求头 */
  uint8_t* dnsHeaderTemp = LittleToBig_HalfWord((uint8_t *)&dnsHead, sizeof(DnsHeadStruct));
  CopyPacket(dnsHeaderTemp, dns->packet, sizeof(DnsHeadStruct));
  free(dnsHeaderTemp);
  
  
  /* 复制问题区域的域名 */
  CopyPacket(domainTemp, dns->packet + sizeof(DnsHeadStruct), domainLen + 2);
  
  /* 复制问题区域的type和class */
  uint8_t* dnsQuestionPara = LittleToBig_HalfWord((uint8_t *)&dnsQuestion, sizeof(DnsQuestionStruct));
  CopyPacket(dnsQuestionPara, dns->packet + sizeof(DnsHeadStruct) + domainLen + 2, sizeof(DnsQuestionStruct));
  free(dnsQuestionPara);
  
  free(domainTemp);
    
  return dns;
}
/*********************************************************************************************
 * @brief  获取Ip
 * @para： packet：数据包指针
           length：数据包长度
 * @return ip 4字节数组，记得free
 * @remark 
  ********************************************************************************************/
ArrayStruct* Dns_AnalyseIp(uint8_t *packet, uint16_t length)
{
  ArrayStruct* ip = Array_New(4);
  
  CopyPacket(packet + (length - 4), ip->packet, 4);
  
  return ip;
}

/*********************************************************************************************
 * @brief  小端转大端，半字
 * @para： packet：需要转换的指针
           length：需要转换的总长度，必须为2的倍数
 * @return *result，返回一个数组，记得free掉
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