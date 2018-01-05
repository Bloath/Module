/* Includes ------------------------------------------------------------------*/
#include "ESP8266.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "StringHandle.h"
#include "Time.h"
#include "SimpleBuffer.h"
#include "ESP8266_HalHandle.h"
#include "Convert.h"

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
ESP8266_ConnectStatusEnum ESP8266_ConnectStatus = ConnectStatus_Init;
ESP8266_TcpStatusEnum ESP8266_TcpStatus = TcpStatus_Init;
uint16_t ESP8266_Status = 0;


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
    ESP8266_SendAtString("ATE0");
    time = sysTime;
    ESP8266_ConnectStatus = ConnectStatus_Idle;
    break;
    
  case ConnectStatus_Idle:      //���д���
    if((time + INTERVAL) < sysTime)
    { 
      ESP8266_SendAtString("AT+CWJAP?");
      time = sysTime;
    }
    break;
    
  
  case ConnectStatus_AirKiss:   //Airkiss
    ESP8266_SendAtString("AT+CWSTARTSMART=3");
    ESP8266_ConnectStatus = ConnectStatus_WaitAck;
    break;
    
  case ConnectStatus_WaitAck:
    break;
    
  case ConnectStatus_Connected:
    TxBlockListHandle(esp8266_TxBlockList, ESP8266_HttpTransmit, 1000);
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
  IntSringTypeDef *count;
  static uint32_t time = 0;
  
  switch(ESP8266_TcpStatus)
  { 
  case TcpStatus_Init:          //��ʼ��������TCP����
    ESP8266_SendString("AT+CIPSTART=\"TCP\",\"");
    ESP8266_SendString(DOMAIN);
    ESP8266_SendString("\",80\r\n");
    ESP8266_TcpStatus = TcpStatus_WaitAck;
    time = sysTime;
    break;
    
  case TcpStatus_Connected:     // ������������
    count = IntConvertToString((uint32_t)length);
    ESP8266_SendString("AT+CIPSEND=");
    ESP8266_SendData(count->string, count->length);
    ESP8266_SendString("\r\n");
    ESP8266_TcpStatus = TcpStatus_WaitAck;
    time = sysTime;
    free(count);
    break;
    
  case TcpStatus_StartTrans:    //��ʼ����
    ESP8266_SendData(message, length);
    ESP8266_TcpStatus = TcpStatus_WaitAck;
    time = sysTime;
    break;
  
  case TcpStatus_WaitAck:
    if((time + INTERVAL) < sysTime)
    {
      ESP8266_ErrorHandle(Error_CipSendError);
      ESP8266_TcpStatus = TcpStatus_Init;
    }
    
    break;
    
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
void ESP8266_RxMsgHandle(uint8_t *message, uint16_t length)
{
  if(ESP8266_TcpStatus == TcpStatus_WaitAck)
  { uint8_t i = 0; }
  
  if(!StringInclude("+CWJAP", message, length) || !StringInclude("WIFI CONNECTED", message, length))
  { ESP8266_ConnectStatus = ConnectStatus_Connected; return; }         //+CWJAP/CONNECTED����λ���ӱ�־λ
  
  if(!StringInclude("smartconfig connected", message, length))  // smartconfig connected������Ҫ�ͷ� AT+CWSTOPSMART
  { ESP8266_SendAtString("AT+CWSTOPSMART"); return; }
  
  if(!StringInclude("No AP", message, length))                  // No AP��������ӱ�־λ 
  { ESP8266_Status &= ~ESP8266_WIFI_CONNECTED; return; }
  
  if(!StringInclude("ERROR", message, length))
  { 
    if(!StringInclude("CWSTARTSMART", message, length))
    { ESP8266_ErrorHandle(Error_AirKissError); return; }
    
  }
  /***********TCP����****************/
  
  if(!StringInclude("CLOSED", message, length))                 //CLOSED ���ӹر�
  { ESP8266_TcpStatus = TcpStatus_Init; return; }
    
  if(!StringInclude("CONNECT\r\n", message, length) ||
     !StringInclude("ALREADY CONNECTED", message, length))            //CONNECT ���ӳɹ�
  { ESP8266_TcpStatus = TcpStatus_Connected; return; }
  
  if(!StringInclude("OK\r\n>", message, length))                      // > ��ʼ���շ�������
  { ESP8266_TcpStatus = TcpStatus_StartTrans; return; }

  if(!StringInclude("Recv", message, length))                   //SEND OK ���ͳɹ�
  { ESP8266_TcpStatus = TcpStatus_SendOk;  }
  
  /***********�յ����ݴ���****************/
  if(!StringInclude("+IPD", message, length) || !StringInclude("+ID", message, length))
  { 
    if(!StringInclude("HTTP", message, length))
    { 
      uint16_t index = StringFindIndex("\r\n\r\n68", message, length);
      
      if(index != 0xFFFF)
      { FillRxBlock(esp8266_RxBlockList, message + index + 4, length - index - 4); }
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
  uint8_t *temp = (uint8_t*)malloc((length + 2) * sizeof(uint8_t));             //����һ���ڴ�
  uint16_t i;
  uint32_t time;
	
  for(i=0; i<length; i++)                                //��ԭ�����ݸ��ƽ���
  { temp[i] = data[i]; }
  temp[length] = 0x0D;                                  //���һ�ֽ���������
  temp[length + 1] = 0x0A;
  ESP8266_SendData(temp, length + 2);                   //��������
  
  time = sysTime;
  
  while((time + 300) > sysTime);
  
  free(temp);                                           //�ͷ�һ���ڴ�
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
  uint16_t i=0;
  
  ESP8266_HardWareReset();                                                      //Ӳ����λ
  ESP8266_ConnectStatus = ConnectStatus_Init;                                 //��ʼ������
  
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
