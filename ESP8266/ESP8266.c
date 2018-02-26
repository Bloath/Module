/* Includes ------------------------------------------------------------------*/

#include "string.h"
#include "stdlib.h"
#include "Time.h"

#include "../UartDma/SimpleBuffer.h"
#include "../Common/Convert.h"
#include "../Common/Delay.h"
#include "../Common/Malloc.h"

#include "../ZcProtocol/Http.h"
#include "../ZcProtocol/ZcProtocol_Conf.h"

#include "ESP8266.h"
#include "ESP8266_Handle.h"


/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  ConnectStatus_Init = 0,
  ConnectStatus_Idle,
  ConnectStatus_AirKiss,
  ConnectStatus_Connected,
  ConnectStatus_WaitAck,
}ESP8266_ConnectStatusEnum;

typedef enum
{
  Error_ReceiveTimeout,
  Error_transmitTimeout,
  Error_CantConnectServer,
  Error_CipSendError,
  Error_TxBufferFull,
  Error_AirKissError
}ESP8266_Error;

typedef enum
{
  TxStatus_Idle,
  TxStatus_SendTxCmd,
  TxStatus_WaitCmdAck,
  TxStatus_SendData,
  TxStatus_WaitDataAck,
  TxStatus_WaitFinishAck,
}WifiTxStatus;

typedef enum
{
  TcpStatus_Init,
  TcpStatus_Connected,
  TcpStatus_StartTrans,
  TcpStatus_WaitAck,
  TcpStatus_SendOk
}ESP8266_TcpStatusEnum;


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ESP8266_ConnectStatusEnum ESP8266_ConnectStatus = ConnectStatus_Init;   //WIFI����״̬
ESP8266_TcpStatusEnum ESP8266_TcpStatus = TcpStatus_Init;               // TCP����״̬
uint16_t ESP8266_Status = 0;

TxBlockTypeDef Enthernet_TxBlockList[TX_BLOCK_COUNT];                      //ģ��ķ��ͻ���
RxBlockTypeDef Enthernet_RxBlockList[RX_BLOCK_COUNT];                      //ģ��Ľ��ջ���

/* Private function prototypes -----------------------------------------------*/
void ESP8266_SendAtCmd(uint8_t *data, uint16_t length);
void ESP8266_SendAtString(const char* cmd);

void ESP8266_ErrorHandle(ESP8266_Error errorType);
void ESP8266_HttpTransmit(uint8_t *message, uint16_t length);
void ESP8266_ResponseHandle(uint8_t *message, uint16_t length);

/* Private functions ---------------------------------------------------------*/

/*********************************************************************************************

  * @brief  ESP8266�Ľ��մ�����
  * @param  data����Ҫ���͵�����ָ��
  * @param  length�����ݳ���  
  * @retval 

  ********************************************************************************************/
void ESP8266_Handle()
{
  static uint32_t time = 0;
  
  
  switch(ESP8266_ConnectStatus)
  {
  case ConnectStatus_Init:      //��ʼ�����رջ���
    //
    ESP8266_SendAtString("AT+CWMODE_DEF=1");
    ESP8266_SendAtString("AT+CWAUTOCONN=1");
    //ESP8266_SendAtString("ATE0");
    time = realTime;
    ESP8266_ConnectStatus = ConnectStatus_Idle;
    break;
    
  case ConnectStatus_Idle:      //���д���
    if((time + ESP8266_INTERVAL) < sysTime)
    { 
      ESP8266_SendAtString("AT+CWJAP?");
      time = realTime;
      ESP8266_ConnectStatus = ConnectStatus_WaitAck;
    }
    break;
    
  
  case ConnectStatus_AirKiss:   //Airkiss
    ESP8266_SendAtString("AT+CWSTARTSMART=3");
    ESP8266_ConnectStatus = ConnectStatus_WaitAck;
    time = realTime;
    break;
    
  case ConnectStatus_WaitAck:
    if((time + ESP8266_INTERVAL) < realTime)
    { ESP8266_ErrorHandle(Error_transmitTimeout);}
    break;
    
  case ConnectStatus_Connected:
    TxBlockListHandle(Enthernet_TxBlockList, ESP8266_HttpTransmit, 2000);
    break;
  }
}

/*********************************************************************************************

  * @brief  ESP8266�Ľ��մ�����
  * @param  data����Ҫ���͵�����ָ��
  * @param  length�����ݳ���  
  * @retval 

  ********************************************************************************************/
void ESP8266_HttpTransmit(uint8_t *message, uint16_t length)
{
  static uint32_t time = 0;
  char *count = NULL;
  
  switch(ESP8266_TcpStatus)
  { 
    /* ��ʼ��������TCP���ӣ�AT+CIPSTART="TCP","����",80���ȴ�ģ��ظ�CONNECT ���� ALREADY CONNECTED */
  case TcpStatus_Init:          
    ESP8266_SendString("AT+CIPSTART=\"TCP\",\"");
#ifdef DOMAIN
    ESP8266_SendString(DOMAIN);
    ESP8266_SendString("\",80\r\n");
#else
    ESP8266_SendString(IP);
    ESP8266_SendString("\",");
    ESP8266_SendString(PORT);
    ESP8266_SendString("\r\n");
#endif
    ESP8266_TcpStatus = TcpStatus_WaitAck;
    time = realTime;
    break;
  
    /* ���������������ȷ�����������, AT+CIPSEND=�������ȴ�ģ��ظ� > */
  case TcpStatus_Connected:     
    count = Uint2String((uint32_t)length);
    ESP8266_SendString("AT+CIPSEND=");
    ESP8266_SendString(count);
    ESP8266_SendString("\r\n");
    ESP8266_TcpStatus = TcpStatus_WaitAck;
    time = realTime;
    Free(count);
    break;
    
    /* ������д��ģ�飬�ȴ��ظ�SEND OK */
  case TcpStatus_StartTrans:    //��ʼ����
    ESP8266_SendData(message, length);
    ESP8266_TcpStatus = TcpStatus_WaitAck;
    time = realTime;
    break;
  
  case TcpStatus_WaitAck:
    if((time + ESP8266_INTERVAL) < realTime)
    { ESP8266_ErrorHandle(Error_CipSendError); }        //�ȴ���ʱ��������ATָ������ݺ󣬳�ʱ��û�ظ���
    
    break;
    
    /* ��ģ����յ��ظ�����ʱ����ظ�Recv xx bytes���л�Ϊ���ͳɹ� */
  case TcpStatus_SendOk:
    ESP8266_TcpStatus = TcpStatus_Init;
    break;
  }
}

/*********************************************************************************************

  * @brief  ESP8266�Ľ��մ�����
  * @param  data����Ҫ���͵�����ָ��
  * @param  length�����ݳ���  
  * @retval 

  ********************************************************************************************/
void ESP8266_RxMsgHandle(uint8_t *packet, uint16_t length)
{
  char *message = (char *)packet;
  
  /***********����wifi����****************/
  if(strstr(message, "+CWJAP") != NULL || strstr(message, "WIFI CONNECTED") != NULL)
  { ESP8266_ConnectStatus = ConnectStatus_Connected; return; }         //+CWJAP/CONNECTED����λ���ӱ�־λ
  
  if(strstr(message, "+smartconfig connected") != NULL)  // smartconfig connected������Ҫ�ͷ� AT+CWSTOPSMART
  { ESP8266_SendAtString("AT+CWSTOPSMART"); return; }
  
  if(strstr(message, "No AP") != NULL)                  // No AP��������ӱ�־λ 
  { ESP8266_Status &= ~ESP8266_WIFI_CONNECTED; return; }
  
  /************����ERROR����************/
  if(strstr(message,"ERROR") != NULL)
  { 
    if(strstr(message, "CWSTARTSMART") != NULL)
    { ESP8266_ErrorHandle(Error_AirKissError); return; }
    
     if(strstr(message, "CLOSED") != NULL)
    { ESP8266_ErrorHandle(Error_CantConnectServer); return; }
    
  }
  /**************TCP����****************/
  
  if(strstr(message, "CLOSED") != NULL)                 //CLOSED ���ӹر�
  { ESP8266_TcpStatus = TcpStatus_Init; return; }
    
  if(strstr(message, "CONNECT\r\n") != NULL ||
     strstr(message, "ALREADY CONNECTED") != NULL)            //CONNECT ���ӳɹ�
  { ESP8266_TcpStatus = TcpStatus_Connected; return; }
  
  if(strstr(message, "OK\r\n>") != NULL)                      // > ��ʼ���շ�������
  { ESP8266_TcpStatus = TcpStatus_StartTrans; return; }

  if(strstr(message, "SEND OK") != NULL)                   //SEND OK ���ͳɹ�
  { ESP8266_TcpStatus = TcpStatus_SendOk; }
  
  /***********�յ����ݴ���****************/
  if(strstr(message, "+IPD") != NULL  || strstr(message, "+ID") != NULL)
  { 
    if(strstr(message, "HTTP") != NULL)                 //�ҵ�HTTP�ַ���
    { 
      char* index = (char *)Http_GetResponse(message);  //ͨ�����λ����ҵ��ظ���
     
      if(index != NULL)
      { FillRxBlock(Enthernet_RxBlockList, (uint8_t*)index, strlen(index)); }     // �ظ��崦��
    }
  }  

}


/*********************************************************************************************

  * @brief  ��ESP8266��������,����β������һ�������ֽ�
  * @param  data����Ҫ���͵�����ָ��
  * @param  length�����ݳ���  
  * @retval ��
  * @remark 

  ********************************************************************************************/
void ESP8266_SendAtCmd(uint8_t *data, uint16_t length)
{
  uint8_t end[2] = {0x0D, 0x0A};
  ESP8266_SendData(data, length);       //��������
  ESP8266_SendData(end, 2);             //���ͻ���
  
  Delay_ms(20);
}
/*********************************************************************************************

  * @brief  ��ESP8266�����������ֱ�����ַ���
  * @param  cmd�������ֽ�
  * @retval ��
  * @remark 

  ********************************************************************************************/
void ESP8266_SendAtString(const char* cmd)
{
  ESP8266_SendAtCmd((uint8_t *)cmd, strlen(cmd));
}


/*********************************************************************************************

  * @brief  ģ��Ĵ�����
  * @param  errorType����������
  * @return 
  * @remark 

  ********************************************************************************************/
void ESP8266_ErrorHandle(ESP8266_Error errorType)
{
  
  ESP8266_HardWareReset();                                                      //Ӳ����λ
  
  switch(errorType)
  {
    
  /* ���ӷ��������� */
  case Error_CantConnectServer:
    
    break;
    
  /* ���ճ�ʱ */
  case Error_ReceiveTimeout:
    
    break;
    
  /* ���ͳ�ʱ */
  case Error_transmitTimeout:
    ESP8266_ConnectStatus = ConnectStatus_Init;                                 //��ʼ����������
    
    break;
    
  /* ��������ʱ */
  case Error_CipSendError:
    ESP8266_ConnectStatus = ConnectStatus_Init;                                 //��ʼ������
    ESP8266_TcpStatus = TcpStatus_Init;
    break;
    
  case Error_AirKissError:
    ESP8266_SendAtString("AT+RST");
    break;
    
  default:break;
  }
}
