/* Includes ------------------------------------------------------------------*/

#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "Time.h"
#include "SimpleBuffer.h"
#include "ESP8266.h"
#include "ESP8266_Handle.h"
#include "Convert.h"
#include "Http.h"

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

  * @brief  ESP8266的接收处理函数
  * @param  data：需要发送的数据指针
  * @param  length：数据长度  
  * @retval 

  ********************************************************************************************/
void ESP8266_Handle()
{
  static uint32_t time = 0;
  
  
  switch(ESP8266_ConnectStatus)
  {
  case ConnectStatus_Init:      //初始化，关闭回显
    //
    ESP8266_SendAtString("AT+CWMODE_DEF=1");
    ESP8266_SendAtString("AT+CWAUTOCONN=1");
    ESP8266_SendAtString("ATE0");
    time = sysTime;
    ESP8266_ConnectStatus = ConnectStatus_Idle;
    break;
    
  case ConnectStatus_Idle:      //空闲处理
    if((time + ESP8266_INTERVAL) < sysTime)
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

  * @brief  ESP8266的接收处理函数
  * @param  data：需要发送的数据指针
  * @param  length：数据长度  
  * @retval 

  ********************************************************************************************/
void ESP8266_HttpTransmit(uint8_t *message, uint16_t length)
{
  static uint32_t time = 0;
  char *count = NULL;
  
  switch(ESP8266_TcpStatus)
  { 
  case TcpStatus_Init:          //初始化，发送TCP连接
    ESP8266_SendString("AT+CIPSTART=\"TCP\",\"");
    ESP8266_SendString(DOMAIN);
    ESP8266_SendString("\",80\r\n");
    ESP8266_TcpStatus = TcpStatus_WaitAck;
    time = sysTime;
    break;
    
  case TcpStatus_Connected:     // 发送数据数量
    count = Uint2String((uint32_t)length);
    ESP8266_SendString("AT+CIPSEND=");
    ESP8266_SendString(count);
    ESP8266_SendString("\r\n");
    ESP8266_TcpStatus = TcpStatus_WaitAck;
    time = sysTime;
    free(count);
    break;
    
  case TcpStatus_StartTrans:    //开始传输
    ESP8266_SendData(message, length);
    ESP8266_TcpStatus = TcpStatus_WaitAck;
    time = sysTime;
    break;
  
  case TcpStatus_WaitAck:
    if((time + ESP8266_INTERVAL) < sysTime)
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

  * @brief  ESP8266的接收处理函数
  * @param  data：需要发送的数据指针
  * @param  length：数据长度  
  * @retval 

  ********************************************************************************************/
void ESP8266_RxMsgHandle(uint8_t *packet, uint16_t length)
{
  char *message = (char *)packet;
  
  if(ESP8266_TcpStatus == TcpStatus_WaitAck)
  { uint8_t i = 0; }
  
  if(strstr(message, "+CWJAP") != NULL || strstr(message, "WIFI CONNECTED") != NULL)
  { ESP8266_ConnectStatus = ConnectStatus_Connected; return; }         //+CWJAP/CONNECTED，置位连接标志位
  
  if(strstr(message, "+smartconfig connected") != NULL)  // smartconfig connected，则需要释放 AT+CWSTOPSMART
  { ESP8266_SendAtString("AT+CWSTOPSMART"); return; }
  
  if(strstr(message, "No AP") != NULL)                  // No AP，清除连接标志位 
  { ESP8266_Status &= ~ESP8266_WIFI_CONNECTED; return; }
  
  if(strstr(message,"ERROR") != NULL)
  { 
    if(strstr(message, "CWSTARTSMART") != NULL)
    { ESP8266_ErrorHandle(Error_AirKissError); return; }
    
  }
  /***********TCP部分****************/
  
  if(strstr(message, "CLOSED") != NULL)                 //CLOSED 链接关闭
  { ESP8266_TcpStatus = TcpStatus_Init; return; }
    
  if(strstr(message, "CONNECT\r\n") != NULL ||
     strstr(message, "ALREADY CONNECTED") != NULL)            //CONNECT 连接成功
  { ESP8266_TcpStatus = TcpStatus_Connected; return; }
  
  if(strstr(message, "OK\r\n>") != NULL)                      // > 开始接收发送数据
  { ESP8266_TcpStatus = TcpStatus_StartTrans; return; }

  if(strstr(message, "Recv") != NULL)                   //SEND OK 发送成功
  { ESP8266_TcpStatus = TcpStatus_SendOk;  }
  
  /***********收到数据处理****************/
  if(strstr(message, "+IPD") != NULL  || strstr(message, "+ID") != NULL)
  { 
    if(strstr(message, "HTTP") != NULL)
    { 
      char* index = (char *)Http_GetResponse(message);
      
      if(index != NULL)
      { FillRxBlock(esp8266_RxBlockList, (uint8_t*)index, strlen(index)); }
    }
  }  

}


/*********************************************************************************************

  * @brief  向ESP8266发送命令,并向尾部增加一个换行字节
  * @param  data：需要发送的数据指针
  * @param  length：数据长度  
  * @retval 无
  * @remark 

  ********************************************************************************************/
void ESP8266_SendAtCmd(uint8_t *data, uint16_t length)
{
  uint8_t *temp = (uint8_t*)malloc((length + 2) * sizeof(uint8_t));             //申请一段内存
  uint16_t i;
  uint32_t time;
	
  for(i=0; i<length; i++)                                //将原有数据复制进入
  { temp[i] = data[i]; }
  temp[length] = 0x0D;                                  //最后一字节新增换行
  temp[length + 1] = 0x0A;
  ESP8266_SendData(temp, length + 2);                   //发送数据
  
  time = sysTime;
  
  while((time + 300) > sysTime);
  
  free(temp);                                           //释放一段内存
}
/*********************************************************************************************

  * @brief  向ESP8266发送命令，可以直接填字符串
  * @param  cmd：命令字节
  * @retval 无
  * @remark 

  ********************************************************************************************/
void ESP8266_SendAtString(const char* cmd)
{
  ESP8266_SendAtCmd((uint8_t *)cmd, strlen(cmd));
}


/*********************************************************************************************

  * @brief  模块的错误处理
  * @param  errorType：错误类型
  * @return 
  * @remark 

  ********************************************************************************************/
void ESP8266_ErrorHandle(ESP8266_Error errorType)
{
  
  ESP8266_HardWareReset();                                                      //硬件复位
  ESP8266_ConnectStatus = ConnectStatus_Init;                                 //初始化连接
  
  switch(errorType)
  {
    
  /* 连接服务器出错 */
  case Error_CantConnectServer:
    
    break;
    
  /* 接收超时 */
  case Error_ReceiveTimeout:
    
    break;
    
  /* 发送超时 */
  case Error_transmitTimeout:
    ESP8266_ConnectStatus = ConnectStatus_Init;                                 //初始化连接流程
    
    break;
    
  /* 启动发送时 */
  case Error_CipSendError:
    ESP8266_ConnectStatus = ConnectStatus_Init;                                 //初始化连接
    ESP8266_TcpStatus = TcpStatus_Init;
    break;
    
  case Error_AirKissError:
    ESP8266_SendAtString("AT+RST");
    break;
    
  default:break;
  }
}
