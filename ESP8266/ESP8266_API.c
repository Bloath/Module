/* Includes ------------------------------------------------------------------*/
#include "ESP8266_API.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
Esp8266Struct esp8266;

/* Private function prototypes -----------------------------------------------*/
void ESP8266_SendData(uint8_t *data, uint16_t length);
void ESP8266_SendString(const char *string);
void ESP8266_ErrorHandle(ESP8266_Error errorType);
bool ESP8266_HttpTransmit(uint8_t *message, uint16_t length);
void ESP8266_ResponseHandle(uint8_t *message, uint16_t length);

/* Private functions ---------------------------------------------------------*/
/*********************************************************************************************

  * @brief  ESP8266的主处理
  * @param  
  * @param  
  * @retval 

  ********************************************************************************************/
void ESP8266_Reset()
{
    esp8266._conProcess = ConnectStatus_Reset;
}
/*********************************************************************************************

  * @brief  ESP8266是否为空闲
  * @param  
  * @param  
  * @retval 

  ********************************************************************************************/
bool ESP8266_IsIdle()
{
    return (esp8266.txQueueApp->_usedBlockQuantity == 0 
            && (esp8266._conProcess == ConnectStatus_Idle 
                || esp8266._conProcess == ConnectStatus_Connected));
}

/*********************************************************************************************

  * @brief  ESP8266的主处理
  * @param  
  * @param  
  * @retval 

  ********************************************************************************************/
void ESP8266_PowerOn()
{
    /* 有数据时且WIFI电源为切断时，开启WIFI电源，重置运行流程 */
    ESP8266_UART_DISABLE();
    ESP8266_POWER_ON();      
    esp8266.UTProcess = UTStatus_Init;
    esp8266._conProcess = ConnectStatus_ResetWait;
}

/*********************************************************************************************

  * @brief  ESP8266的主处理
  * @param  
  * @param  
  * @retval 

  ********************************************************************************************/
void ESP8266_EnableAirkiss()
{    
    esp8266._flag |= ESP8266_AIRKISS;
    esp8266._conProcess = ConnectStatus_Reset;       // 防止处于TCP接收数据部分，不响应AT指令
}

/*********************************************************************************************

  * @brief  ESP8266的主处理
  * @param  
  * @param  
  * @retval 

  ********************************************************************************************/
void ESP8266_Handle()
{
    TxQueue_Handle(esp8266.txQueueHal, esp8266.CallBack_HalTxFunc, esp8266.halTxParam);

    switch (esp8266._conProcess)
    {
    /****** 初始化，设置发送队列时长，并发送模式设置与自动连接 ********/
    case ConnectStatus_Init: 
        esp8266.txQueueHal->interval = ESP8266_HAL_TX_INTERVAL;             // 硬件层 发送时长
        esp8266.txQueueApp->interval = ESP8266_SERVICE_TX_INTERVAL;     // 业务层 发送时长
        
        ESP8266_SendString("AT+CWMODE_DEF=1\r\n");
        ESP8266_SendString("AT+CWAUTOCONN=1\r\n");
        esp8266.__time = realTime;
        if ((esp8266._flag & ESP8266_AIRKISS) != 0)
        {   
            esp8266._conProcess = ConnectStatus_AirKiss; 
            esp8266._flag &= ~ESP8266_AIRKISS;
        }
        else
        {   esp8266._conProcess = ConnectStatus_Idle;    }
        break;

    /****** 复位相关，如果复位则先发送复位，再等待2S后开UART ********/
    case ConnectStatus_Reset:
        ESP8266_RST_ENABLE();
        ESP8266_UART_DISABLE();
        esp8266.__time = realTime;
        esp8266._conProcess = ConnectStatus_ResetWait;
        break;

    case ConnectStatus_ResetWait:
        if ((esp8266.__time + 2) < realTime)
        {
            ESP8266_RST_DISABLE();
            esp8266.__time = realTime;
            esp8266._conProcess = ConnectStatus_ResetFinish;
        }
        break;
        
    case ConnectStatus_ResetFinish:
        if ((esp8266.__time + 5) < realTime)
        {   
            ESP8266_UART_ENABLE();
            esp8266._conProcess = ConnectStatus_Init; 
            esp8266.UTProcess = UTStatus_Init;
        }
        break;

    /********** 空闲状态、检查是否连接wifi ****************/
    case ConnectStatus_Idle:
        if ((esp8266.__time + ESP8266_INTERVAL) < realTime)
        {
            ESP8266_SendString("AT+CWJAP?\r\n");
            esp8266.__time = realTime;
            esp8266._conProcess = ConnectStatus_WaitAck;
        }
        break;

    /***************** airkiss相关 **********************/
    case ConnectStatus_AirKiss:                         
        ESP8266_SendString("AT+CWSTARTSMART=3\r\n");    // 1. 发送 "AT+CWSTARTSMART=3" 进入等待状态
        esp8266._conProcess = ConnectStatus_WaitAck;     // 2. 等待接受广播信号，如果接收到 "smartconfig connected" 则发送 "AT+CWSTOPSMART" 释放
        esp8266._flag |= ESP8266_AIRKISSING;
        esp8266.__time = realTime;
        break;

    case ConnectStatus_AirKissWait:                     // 3. 如果60s内没有收到广播信号，则直接发送 "AT+CWSTOPSMART" 释放
        if ((esp8266.__time + 60) < realTime)
        {
            esp8266._flag &= ~ESP8266_AIRKISSING;
            ESP8266_SendString("AT+CWSTOPSMART\r\n");
            ESP8266_ErrorHandle(Error_AirKissError);
        }
        break;

    /***************** 等待回复 **********************/
    case ConnectStatus_WaitAck:
        if ((esp8266.__time + ESP8266_INTERVAL) < realTime)
        {   ESP8266_ErrorHandle(Error_Timeout);    }    //等待超时错误处理，（AT指令发送数据后，长时间没回复）
        break;

    /***************** 已连接 **********************/
    case ConnectStatus_Connected:
        TxQueue_Handle(esp8266.txQueueApp, ESP8266_HttpTransmit, NULL);
        break;
    }
}

/*********************************************************************************************

  * @brief  ESP8266的Http发送处理
  * @param  data：需要发送的数据指针
  * @param  length：数据长度  
  * @retval 基于Http的短连接

  ********************************************************************************************/
bool ESP8266_HttpTransmit(uint8_t *message, uint16_t length)
{
    static uint32_t time = 0;
    char cmdPacket[50] = {0};
    memset(cmdPacket, 0, 40);
    char *packet;

    switch (esp8266.UTProcess)
    {
    /* 发送TCP连接，AT+CIPSTART="TCP","域名",80，等待模块回复CONNECT 或者 ALREADY CONNECTED */ 
    case UTStatus_Init: 
#if defined(ESP8266_TCP)
        strcat(cmdPacket, "AT+CIPSTART=\"TCP\",\"");
#elif defined(ESP8266_UDP)
        strcat(cmdPacket, "AT+CIPSTART=\"UDP\",\"");
#endif
        strcat(cmdPacket, esp8266.http.host);
        strcat(cmdPacket, "\",");
        strcat(cmdPacket, esp8266.http.port);
#if defined(ESP8266_UDP)
        strcat(cmdPacket, ",,0");
#endif
        strcat(cmdPacket, "\r\n");
        
        ESP8266_SendString(cmdPacket); 
        
        esp8266.UTProcess = UTStatus_WaitAck;
        time = realTime;
        break;

    /* 发送数据数量，先发送数据数量, AT+CIPSEND=数量，等待模块回复 > */
    case UTStatus_Connected:
        strcat(cmdPacket, "AT+CIPSEND=");
#if defined(ESP8266_TCP)
        Uint2String(cmdPacket, esp8266.http.CallBack_PacketLength(length, esp8266.http.host, esp8266.http.port));       // 填充长度
#elif defined(ESP8266_UDP)
        Uint2String(cmdPacket, length * 2);
#endif
        strcat(cmdPacket, "\r\n");
        
        ESP8266_SendString(cmdPacket);          // 发送数据

        esp8266.UTProcess = UTStatus_WaitAck;  // 切换为等待模式
        time = realTime;
        break;

    /* 将数据写入模块，等待回复SEND OK */
    case UTStatus_StartTrans:                  
#if defined(ESP8266_TCP)
        packet = esp8266.http.CallBack_HttpPackage(message, length, esp8266.http.host, esp8266.http.port);           // 将数据重新打包
#elif defined(ESP8266_UDP)
        packet = (char*)Malloc(length * 2);
        if(packet != NULL)
        {   Msg2String(packet, message, length);    }
#endif
        
        if(packet == NULL)
        {   esp8266.UTProcess = UTStatus_SendOk;    }
        else
        {
            ESP8266_SendData((uint8_t *)packet, strlen(packet));            // 交给底层缓冲进行发送，标记为isMalloc，在发送缓冲中释放内存
            time = realTime;
            esp8266.UTProcess = UTStatus_WaitAck;                        // 切换为等待模式
        }
        break;

    /* 等待回复，超时次数过多则弹出错误 */
    case UTStatus_WaitAck:
        if ((time + ESP8266_INTERVAL) < realTime)
        {   ESP8266_ErrorHandle(Error_TcpTimeout);  } 

        break;

    /* 当模块接收到回复数据时，会回复Recv xx bytes，切换为发送成功 */
    case UTStatus_SendOk:
#if defined(ESP8266_TCP)
        esp8266.UTProcess = UTStatus_Init;
#elif defined(ESP8266_UDP)
        esp8266.UTProcess = UTStatus_Connected;
#endif
        time = realTime;
        break;
    }

    return false;
}

/*********************************************************************************************

  * @brief  ESP8266的接收处理函数
  * @param  data：需要发送的数据指针
  * @param  length：数据长度  
  * @retval 

  ********************************************************************************************/
void ESP8266_RxMsgHandle(uint8_t *packet, uint16_t length, void *param)
{
    char *message = (char *)packet;
    
    if (strstr(message, "SPI Mode") != NULL)
    {
        esp8266.illegalResetCounter++;
        if(esp8266.illegalResetCounter > 3)
        {   
            ESP8266_ErrorHandle(Error_GetResetInfo);    
            esp8266.illegalResetCounter = 0;
        }
        return;
    }
    
    /***********airkiss部分****************/
    if (strstr(message, "AT+CWSTARTSMART=3") != NULL)
    {
        esp8266._flag &= ~ESP8266_AIRKISSING;
        if (strstr(message, "ERROR") != NULL)
        {   ESP8266_ErrorHandle(Error_AirKissError);    }
        else
        {   esp8266._conProcess = ConnectStatus_AirKissWait;  }
    }

    if (strstr(message, "smartconfig connected") != NULL)           // smartconfig connected，则需要释放 AT+CWSTOPSMART
    {
        esp8266._flag &= ~ESP8266_AIRKISSING;
        esp8266._flag |= ESP8266_AIRKISSED;
        
        ESP8266_SendString("AT+CWSTOPSMART\r\n");
        esp8266._conProcess = ConnectStatus_Idle;                    // Airkiss连接成功，返回
        return;
    }

    /***********连接wifi部分****************/
    /* 在模块上电时，接收到WIFI CONNECT时，不一定真正连接，必须通过CWJAP判断 */
    if (strstr(message, "CWJAP?") != NULL)
    {
        if (strstr(message, "No AP") != NULL)
        {   
            esp8266._conProcess = ConnectStatus_Idle; 
            esp8266.__time = realTime;
            esp8266._flag &= ~ESP8266_WIFI_CONNECTED;
            
            
            // 没配过wifi，则直接写入默认wifi，
            if(esp8266.CallBack_Airkiss(Esp8266Airkiss_IsWriteDefault) == true)
            {   ESP8266_ErrorHandle(Error_NoAP);    }
            else
            {   
                ESP8266_SendString(ESP8266_DEFAULT_WIFI); 
                esp8266.CallBack_Airkiss(Esp8266Airkiss_WrittingDefault);
            }
        }
        else if (strstr(message, "busy") != NULL)
        {   
            ESP8266_ErrorHandle(Error_NoAP);
        }
        else 
        {   
            esp8266._conProcess = ConnectStatus_Connected;
            esp8266._flag |= ESP8266_WIFI_CONNECTED;
        }
        return;
    }

    /************错误ERROR处理************/
    if (strstr(message, "ERROR\r\nCLOSED") != NULL          // "ERROR\r\nCLOSED" 是在无法建立TCP连接
        || strstr(message, "SEND FAIL") != NULL)            // "SEND FAIL" 是在建立TCP连接后发送失败
    {
        ESP8266_ErrorHandle(Error_CipSendError);
        return;
    }
    /**************TCP部分****************/

    // TCP CONNECT 连接成功
    if (strstr(message, "CONNECT\r\n\r\nOK") != NULL)
    {
        esp8266.UTProcess = UTStatus_Connected;
        return;
    }
    
    // 发现ALREADY，则重复发送
    if (strstr(message, "ALREADY CONNECTED") != NULL)
    {
        esp8266.UTProcess = UTStatus_SendOk;
        ESP8266_SendString("AT+CIPCLOSE\r\n");
//        ESP8266_ErrorHandle(Error_CipSendError);
        return;
    }

    // > 开始接收发送数据
    if (strstr(message, "CIPSEND") != NULL && strstr(message, ">") != NULL)
    {
        esp8266.UTProcess = UTStatus_StartTrans;
        return;
    }

    //SEND OK 发送成功，清零发送错误标志
    if(strstr(message, "SEND OK") != NULL)                   
    {   
      esp8266._tcpFailCounter = 0; 
      //esp8266.UTProcess = UTStatus_SendOk;
      esp8266.illegalResetCounter = 0;              // 清空非法复位计数器
    }
    
    // 每次发送成功后都要关闭TCP连接
//    if(strstr(message, "CIPCLOSE") != NULL && strstr(message, "CLOSED") != NULL)
    if(strstr(message, "CLOSED") != NULL)
    {   esp8266.UTProcess = UTStatus_Init;    }

    /**************数据提取处理部分****************/
    /* 根据IPD的头部，将HTTP的内容部分提取出来填充到接收缓冲 */
    if (strstr(message, "+IPD") != NULL || strstr(message, "+ID") != NULL)
    {
        esp8266.UTProcess = UTStatus_SendOk;
#ifdef ESP8266_TCP
        if (strstr(message, "HTTP") != NULL)                                            //找到HTTP字符串
#endif
        {
            char *index = strstr(message, "\n68");                                        //通过两次换行找到回复体
            char *temp = NULL;
            
            /* 回复body提取可用字符串 */
            if (index != NULL)
            {   
                index += 1;                 
                esp8266._tcpFailCounter = 0;
                
                /* 找到换行符，将换行符改为结束符，再对报文进行转换 */
                temp = strstr(index, "\r\n");
                if(temp != NULL)
                {   *temp = '\0';   }
                
                uint8_t *packet = NULL;
                int count = String2Msg(&packet, index, 0);
                if (count > 0)
                {   RxQueue_Add(esp8266.rxQueueApp, packet, count, true);     }
            }     
            else
            {   ESP8266_ErrorHandle(Error_ResponseError);   }
        }
    }
}

/*********************************************************************************************

  * @brief  模块的错误处理
  * @param  errorType：错误类型
  * @return 
  * @remark 

  ********************************************************************************************/
void ESP8266_ErrorHandle(ESP8266_Error errorType)
{
    switch (errorType)
    {
    /* 接收超时 */
    case Error_Timeout:
        esp8266._conProcess = ConnectStatus_Reset;
        esp8266.UTProcess = UTStatus_Init;
        esp8266._timeOutCounter ++;
        if (esp8266._timeOutCounter >= ESP8266_RST_MAX_RETRY)
        {   
            esp8266._timeOutCounter = 0;
            if(esp8266.CallBack_ErrorHandle != NULL)
            {   esp8266.CallBack_ErrorHandle(Error_Rst3Times); }    
        }
        break;

    /* AIRKISS超时 */
    case Error_AirKissError:
        esp8266._conProcess = ConnectStatus_Reset;       //初始化连接流程
        break;
    
    /* 启动发送时 */
    case Error_CipSendError:
        esp8266._tcpFailCounter++;
        esp8266.UTProcess = UTStatus_Init;
        if (esp8266._tcpFailCounter > ESP8266_TCP_MAX_RETRY)
        {
            esp8266._conProcess = ConnectStatus_Reset;
            esp8266.UTProcess = UTStatus_Init;
            esp8266._tcpFailCounter = 0;
            
            esp8266._timeOutCounter ++;
            if (esp8266._timeOutCounter >= ESP8266_RST_MAX_RETRY)
            {
                esp8266._timeOutCounter = 0;
                if(esp8266.CallBack_ErrorHandle != NULL)
                {   esp8266.CallBack_ErrorHandle(Error_Rst3Times); } 
            }
        }
        else
        {   goto errorEnd;    }                         // 重发失败次数不到上限不执行回调
        break;
        
    /* TCP发送超时 */
    case Error_TcpTimeout:
        ESP8266_SendString("AT+CIPCLOSE\r\n");
        esp8266._timeOutCounter ++;
        if (esp8266._timeOutCounter >= ESP8266_RST_MAX_RETRY)
        {
            esp8266._timeOutCounter = 0;
            if(esp8266.CallBack_ErrorHandle != NULL)
            {   esp8266.CallBack_ErrorHandle(Error_Rst3Times); } 
        }
        break;
        
    /* 未连接wifi */
    case Error_NoAP:
        esp8266._flag &= ~ESP8266_WIFI_CONNECTED;
        break;
    
    /* HTTP回复体中没数据 */
    case Error_ResponseError:
        break;
        
    default:
        break;
    }
    
    // ESP8266_ErrorHandle相当于过滤器，将需要esp8266内部处理的先处理好后调用该回调
    // 虽然使用同样的参数，但是要区分两层错误处理，内部处理以及外部处理
    if(esp8266.CallBack_ErrorHandle != NULL)
    {   esp8266.CallBack_ErrorHandle(errorType); }
errorEnd:
    return;
}
/*********************************************************************************************

  * @brief  向ESP8266发送数据
  * @param  data：需要发送的数据指针
  * @param  length：数据长度  
  * @retval 无
  * @remark 

  ********************************************************************************************/
void ESP8266_SendData(uint8_t *data, uint16_t length)
{
    TxQueue_Add(esp8266.txQueueHal, data, length, TX_ONCE_AC | TX_FLAG_IS_MALLOC);
}
/*********************************************************************************************

  * @brief  向ESP8266发送字符串
  * @param  string
  * @retval 无
  * @remark 

  ********************************************************************************************/
void ESP8266_SendString(const char *string)
{
    TxQueue_Add(esp8266.txQueueHal, (uint8_t *)string, strlen(string), TX_ONCE_AC);
}
