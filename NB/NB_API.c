/* Includes ------------------------------------------------------------------*/
#include "Module/Module.h"
#include "NB.h"

/* Private define -------------------------------------------------------------*/
#define NB_HTTP_HEADER  ""

/* Private macro --------------------------------------------------------------*/ 
#define NB_AT_TRANSMIT(string)  NB_HalTransmit(string ,strlen(string), NULL)
/* Private typedef ------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
struct NBStruct nb;

DATA_PREFIX char *nbConfiguration[] = {
    "AT+CGMR\r\n",                           
    "AT+NCSEARFCN\r\n",                     // 清除先验频点
    "AT+CFUN=1\r\n",                        // 启动射频     ----> 启动连接
    "AT+CMEE=1\r\n",                        // 开启错误序号提示
    "AT+CPSMS=0\r\n",
    "AT+CGATT=1\r\n",                       // 开始附着
    "AT+CEREG=4\r\n",                       // 开启信息提示
    "AT+CSCON=1\r\n",                       // 打开信号提示自动上报
    NULL,                          
};

DATA_PREFIX char *nbSetBand[] = {
    "AT+CFUN=0\r\n",
    "AT+CFUN?\r\n",
    "AT+NBAND=8\r\n",
    NULL,
};

DATA_PREFIX char *nbQuery[] = {
    "AT+CSQ\r\n",
    "AT+CCLK?\r\n",
    NULL,                          
};

DATA_PREFIX char *nbOcSet[] = {
    "AT+CFUN=0\r\n",
    "AT+NCDP=180.101.147.115,5683\r\n",
    "AT+NNMI=1\r\n",
    "AT+MLWM2MENABLE=1\r\n",
    NULL,                          
};

DATA_PREFIX char *nbReadSim[] = {
    "AT+CFUN=1\r\n",
    "AT+CGSN=1\r\n",
    "AT+CIMI\r\n",
    NULL,                          
};

/* Private function prototypes ------------------------------------------------*/
void NB_RxHandle(struct RxUnitStruct *unit, void *param);
void NB_HalTransmit(uint8_t *message, uint16_t length, void *param);
void NB_SocketLoopHandle(void (*handleFunc)(struct NBSocketStruct *, void *), void *param);
void NBSocket_ErrorHandleById(struct NBSocketStruct *socket, void *param);
void NBSocket_Reset(struct NBSocketStruct *socket, void *param);
/*********************************************************************************************

  * @brief  添加新的套接字
  * @param  socket：套接字指针 
  * @retval 

  ********************************************************************************************/   
void NB_AddNewSocket(struct NBSocketStruct *socket)      
{           
    struct NBSocketStruct *socketTemp;
    if(nb.sockets == NULL)
    {   nb.sockets = socket;    }
    else
    {
        socketTemp = nb.sockets;
        while(true)
        {
            if(socketTemp->next == NULL)
            {   
                socketTemp->next = socket;  
                break;
            }
            else
            {   socketTemp = socketTemp->next;  }
        }
    }
}
/*********************************************************************************************

  * @brief  添加新的套接字
  * @param  id：套接字指针 
  * @retval 符合要求的套接字指针

  ********************************************************************************************/   
struct NBSocketStruct* NB_FindSocketById(int8_t id)      
{           
    struct NBSocketStruct *socketTemp;
    socketTemp = nb.sockets;
    while(true)
    {
        if(socketTemp == NULL)
        {   break;  }
        
        if(socketTemp->_socketId == id)
        {   return socketTemp;  }
      
        socketTemp = socketTemp->next; 
    }
    
    return NULL;
}

/*********************************************************************************************

  * @brief  nb 套接字中是否空闲
  * @param  *socket: 存储有任务的套接字
  * @retval 

  ********************************************************************************************/   
bool NB_SocketsIsIdle(struct NBSocketStruct **socket)      
{           
    struct NBSocketStruct *socketTemp;
    socketTemp = nb.sockets;
    while(true)
    {
        if(socketTemp == NULL)
        {   break;  }
        
        if(socketTemp->txQueueApp._usedBlockQuantity != 0)
        {   
            if(socket != NULL)
            {   *socket = socketTemp;   }
            return false;  
        }
      
        socketTemp = socketTemp->next; 
    }
    
    return true;
}

/*********************************************************************************************

  * @brief  发送命令列表
  * @param  list：命令字列表 
  * @retval 

  ********************************************************************************************/   
void NB_SendATCommandList(char **list)      
{                                   
    nb.cmdList = list;              
    PROCESS_CHANGE_WITH_TIME(nb._process, Process_OrderAt, REALTIME); 
    nb._orderAT.index = 0;
}
/*********************************************************************************************

    NB 发送数据

  ********************************************************************************************/    
void NB_HalTransmit(uint8_t *message, uint16_t length, void *param)
{
    if(param != NULL
       && nb._process.current == Process_Run)
    {   
        PROCESS_CHANGE_WITH_TIME(nb._process, Process_RunWait, REALTIME);   
        nb.socketCurrent = (struct NBSocketStruct *)param;
    }
    
    nb.CallBack_Transmit(message, length, param);
}

/*********************************************************************************************

  * @brief  NB 启动电源

  ********************************************************************************************/    
void NB_PowerOn()
{
    /* 有数据时且WIFI电源为切断时，开启WIFI电源，重置运行流程 */    
    NB_POWER_ON();
    PROCESS_CHANGE_WITH_TIME(nb._process, Process_Reset, REALTIME);
    NB_SocketLoopHandle(NBSocket_Reset, NULL);
}

/*********************************************************************************************

  * @brief  NB 启动电源

  ********************************************************************************************/    
void NB_PowerOff()
{
    /* 关闭电源 */    
    NB_POWER_OFF();
}
/*********************************************************************************************

  * @brief  判断Nb是否处于空闲状态

  ********************************************************************************************/    
bool NB_IsIdle()
{
    return (nb._process.current == Process_Idle 
            || (nb._process.current == Process_Run && nb._isTransmitting == false)
            || nb._process.current == Process_Lock);
}
/*********************************************************************************************

  * @brief  NB主处理函数
  * @param  
  * @retval 无
  * @remark 放置在主程序中

  ********************************************************************************************/
void NB_Handle()
{   
    struct NBSocketStruct *nbSocketTemp;
  
    switch(nb._process.current)
    {
    /* 上电先查看SIM卡信息，之后才会有不同的处理方式 */
    case Process_Init:
      
        if(nb._sim == Unknown)
        {   NB_SendATCommandList(nbReadSim);  }
        else
        {   PROCESS_CHANGE_WITH_TIME(nb._process, Process_Idle, REALTIME);    }
        break;
   
    /* 空闲状态，没什么事就进入处理 */ 
    case Process_Idle:
        NB_SendATCommandList(nbConfiguration);
        break;

    /* 按顺序发送AT指令，当发送完成后切换到等待状态
     根据接收到的AT中包含的OK或者ERROR进行下一步的判断*/
    case Process_OrderAt:
        NB_AT_TRANSMIT(nb.cmdList[nb._orderAT.index]);
        nb._orderAT.isGetOk = false;
        nb._orderAT.isGetError = false;
        PROCESS_CHANGE_WITH_TIME(nb._process, Process_OrderAtWait, REALTIME);
        break;

    /* 等待3s如果没有回复则重发命令 
     收到OK，则根据当前的索引进行下一步的操作，默认为继续发下一个AT指令
     收到错误，则编写对应的错误处理 */
    case Process_OrderAtWait:
        if ((nb._process.__time + 5) < REALTIME)
        {   
            PROCESS_CHANGE_WITH_TIME(nb._process, Process_OrderAt, REALTIME);
            nb._orderAT.errorCounter ++;
            if(nb._orderAT.errorCounter > 5)
            {   
                nb._orderAT.errorCounter = 0;
                PROCESS_CHANGE_WITH_TIME(nb._process, Process_Lock, REALTIME);    
                if(nb.CallBack_ErrorHandle != NULL)
                {   nb.CallBack_ErrorHandle(ERR_CON_TIMEOUT); }
            }
        }
        else
        {
            /* AT指令发送成功 */
            if (nb._orderAT.isGetOk == true)
            {
                nb._orderAT.index++;                       // AT指令索引+1
                
                /* 如果发现发送到末尾，则进入处理部分 */
                if(nb.cmdList[nb._orderAT.index] != NULL)
                {    PROCESS_CHANGE_WITH_TIME(nb._process, Process_OrderAt, REALTIME); }
                else
                {   
                    if(nb.cmdList == nbConfiguration)        // 配置参数发送完成 => 进入查询附着状态
                    {   
                        PROCESS_CHANGE_WITH_TIME(nb._process, Process_Wait, REALTIME);
                        nb._startConnectTime = REALTIME;
                    }  
                    else if (nb.cmdList == nbQuery)
                    {   PROCESS_CHANGE_WITH_TIME(nb._process, Process_Start, REALTIME);   }   // 查询参数发送完成 => 进入准备发送状态
                    else if (nb.cmdList == nbSetBand)
                    {   PROCESS_CHANGE_WITH_TIME(nb._process, Process_Reset, REALTIME);   }
                    else
                    {   NB_SendATCommandList(nbConfiguration);  }   // 其他的则直接进入连接状态部分，有可能有的在初始化部分要处理
                }
            }
            else if (nb._orderAT.isGetError == true)
            {}
        }
        break;

    /* 等待连接 */
    case Process_Wait:
        // 每5s发送一次查询连接
        if ((nb._process.__time + 2) < REALTIME)
        {
            nb._process.__time = REALTIME;
            NB_AT_TRANSMIT("AT+CGATT?\r\n");
        }

        // 长时间之后未连接处理
        if ((nb._startConnectTime + 50) < REALTIME)
        {   
            PROCESS_CHANGE_WITH_TIME(nb._process, Process_Lock, REALTIME);    
            if(nb.CallBack_ErrorHandle != NULL)
            {   nb.CallBack_ErrorHandle(ERR_CON_INIT); }
        }
        break;

    /* 启动连接 */
    case Process_Start:
        PROCESS_CHANGE_WITH_TIME(nb._process, Process_Run, REALTIME);
        break;
        
    /* 开始工作部分，对于NB来说，有数据直接发送即可，等待回复 */
    case Process_Run:
        if(nb._isTransmitting == true)
        {
            /* 查看是否有需要发送的数据 */
            if(NB_SocketsIsIdle(NULL) == true)
            {
                if ((nb._process.__time + 2) < REALTIME)
                {   nb._isTransmitting = false; }
            }
            /* 有需要发送的, 则进行处理, 在进行硬件发送时, 退出到等待模式 */
            else
            {
                nbSocketTemp = nb.sockets;
                while(true)
                {
                    if(nbSocketTemp == NULL)
                    {   break;  }
                    if(nbSocketTemp->txQueueApp._usedBlockQuantity != 0)
                    {   
                        TxQueue_Handle(&(nbSocketTemp->txQueueApp));    
                        break;
                    }
//                    if(nb._process.current != Process_Run)
//                    {   break;  }
                    nbSocketTemp = nbSocketTemp->next;
                }
            }
        }
        else
        {
            nb._process.__time = REALTIME;
            nb._isTransmitting = true;
        }
        break;
    
    /* 发送等待部分，等待3s发送下一条 */
    case Process_RunWait:
        if ((nb._process.__time + 3) < REALTIME)
        {   
            PROCESS_CHANGE_WITH_TIME(nb._process, Process_Run, REALTIME);  
            nb.socketCurrent->_counterError += 1;
        }
        break;
        
    case Process_Reset:
        NB_AT_TRANSMIT("AT+NRB\r\n");
        NB_AT_TRANSMIT("AT+NRB\r\n");
        PROCESS_CHANGE_WITH_TIME(nb._process, Process_ResetWait, REALTIME);
        break;

    case Process_ResetWait:
        if((nb._process.__time + 10) < REALTIME)
        {   
            PROCESS_CHANGE_WITH_TIME(nb._process, Process_Lock, REALTIME);
            if(nb.CallBack_ErrorHandle != NULL)
            {   nb.CallBack_ErrorHandle(ERR_NO_RESPONSE); }    
        }
        break;
    }
}
/*********************************************************************************************

  * @brief  NB接收部分处理
  * @param  unit块
  * @retval 无
  * @remark 

  ********************************************************************************************/

int NB_Http_ReceiveHandle(struct RxUnitStruct *unit, void *param);


void NB_RxHandle(struct RxUnitStruct *unit, void *param)
{
    char *message = (char *)(unit->message);
    char *location = NULL, *temp = NULL;
    struct CalendarStruct calendarTemp = {0};
    int32_t temp32 = 0;


    if (nb.cmdList == nbReadSim && (location = strstr(message, "460")) != NULL)
    {   
        if(location[4] == '0' || location[4] == '2' || location[4] == '4' || location[4] == '7')
        {   
            nb._sim = ChinaMobile;  
            nbSetBand[2][9] = '8';
            NB_SendATCommandList(nbSetBand);
        }
        else if(location[4] == '3' || location[4] == '5' || (location[3] == '1' && location[4] == '1'))
        {   
            nb._sim = ChinaTelecom;  
            nbSetBand[2][9] = '5';
            NB_SendATCommandList(nbSetBand);
        }
        
    }
    
    // 在顺序发送AT出现OK或者ERROR的情况
    if (nb._process.current == Process_OrderAtWait)
    {
        if (strstr(message, "OK") != NULL)
        {   
            nb._orderAT.isGetOk = true;  
            nb._orderAT.errorCounter = 0;
        }

        if (strstr(message, "ERROR") != NULL)
        {   
            nb._orderAT.isGetError = true;   
            nb._orderAT.errorCounter ++;
            if(nb._orderAT.errorCounter > 5)
            {   
                nb._orderAT.errorCounter = 0;
                PROCESS_CHANGE_WITH_TIME(nb._process, Process_Lock, REALTIME);
                if(nb.CallBack_ErrorHandle != NULL)
                {   nb.CallBack_ErrorHandle(ERR_CON_INIT); }    
            }
        }
    }
    
    // 读取NB的IEMI码
    location = strstr(message, "+CGSN:");
    if (location != NULL)
    {
        if(nb._imei[0] == 0)
        {   
            memcpy(nb._imei, location + 6, 15);
            nb._imei[15] = '\0';
            if(nb.CallBack_GetImei != NULL)
            {   nb.CallBack_GetImei();  }
        }
        return; 
    }
    
    
    // 是否附着正常判断 CGATT
    location = strstr(message, "+CGATT:1");
    if (location != NULL)
    {
        if (location[7] == '1')
        {   NB_SendATCommandList(nbQuery);   }

        return;
    }

    // 同步NB时间 
    location = strstr(message, "+CCLK:");
    if (location != NULL && nb.CallBack_TimeUpdate != NULL)
    {
        location += 6;                          // 跳过cclk

        sscanf(location, 
              "%02d/%02d/%02d,%02d:%02d:%02d", 
              &temp32, &calendarTemp.month, &calendarTemp.day,
              &calendarTemp.hour, &calendarTemp.min, &calendarTemp.sec);
        calendarTemp.year = 2000 + temp32;
        
        nb.CallBack_TimeUpdate(Calendar2TimeStamp(&calendarTemp, 0));
        return;
    }
    
    // 信号强度判断
    location = strstr(message, "+CSQ");
    if (location != NULL)
    {
        sscanf(location, "+CSQ:%d,", &temp32);
        nb._signal = temp32;
        return;
    }
    
    // 重启完成
    if (strstr(message, "REBOOT_") != NULL && nb._process.current == Process_ResetWait)
    {
        PROCESS_CHANGE_WITH_TIME(nb._process, Process_Init, REALTIME);   
        return;
    }
    
    
    
    // 发送过程中失败的问题
    if(nb._process.current == Process_Run || nb._process.current == Process_RunWait)
    {  
        if((location = strstr(message, "HTTPERR")) != NULL
           && *(location - 1) == '\n')
        {
            sscanf(message, "+HTTPERR:%d,", &temp32);
            NB_SocketLoopHandle(NBSocket_ErrorHandleById, &temp32);
        }
        else if((location = strstr(message, "+CME ERROR")) != NULL
                && *(location - 1) == '\n')
        {
            nb._errorCounter++;
            if(nb._errorCounter > 5)
            {   
                nb._errorCounter = 0;
                PROCESS_CHANGE_WITH_TIME(nb._process, Process_Lock, REALTIME);
                if(nb.CallBack_ErrorHandle != NULL)
                {   nb.CallBack_ErrorHandle(ERR_CON_TX); }   
            }
        }
        else
        {   
            // HTTP 接收处理部分
            if(NB_Http_ReceiveHandle(unit, param) != ERR_NO_TASK)
            {   return; }
        }
        
        PROCESS_CHANGE_WITH_TIME(nb._process, Process_Run, REALTIME);
    }
}

/*********************************************************************************************

    NB HTTP AT 接收处理

  ********************************************************************************************/
int NB_Http_ReceiveHandle(struct RxUnitStruct *unit, void *param)
 {
   char *location = NULL, *temp = NULL;
   struct NBSocketStruct *nbSocketTemp;
   struct HttpParamStruct *httpParam = (struct HttpParamStruct *)(nb.socketCurrent->param);
   int32_t temp32, socketId = -1;
   
   location = strstr((char *)(unit->message), "+HTTPDISCON:0");
   if(location != NULL)
   {    
        sscanf(location, "+HTTPDISCON:%d", &socketId);
        nbSocketTemp = nb.sockets;

        /* 根据socketId发到不同的socket中 */
        while(true)
        {
            if(nbSocketTemp == NULL)
            {   break;  }
            if(nbSocketTemp->_socketId == socketId)
            {
                if(nbSocketTemp->CallBack_ErrorHandle != NULL)
                {   nbSocketTemp->CallBack_ErrorHandle(nbSocketTemp, ERR_CON_CLOSE); }  
                break;
            }
            nbSocketTemp = nbSocketTemp->next;
        }
        return ERR_SUCCESS;
   }

    // 打开socket的处理
    location = strstr((char *)(unit->message), "+HTTPCREATE");
    if(location != NULL
       && nb.socketCurrent != NULL
       && nb.socketCurrent->_process.current == Process_Init)
    {
        sscanf(location, "+HTTPCREATE:%d\r\nOK", &temp32);
        if(nb.socketCurrent != NULL)
        {
            nb.socketCurrent->_socketId = temp32;
            FLAG_SET(nb.socketCurrent->_flag, NBSOCKET_FLAG_CREATED);
        }
        PROCESS_CHANGE_WITH_TIME(nb.socketCurrent->_process, Process_BeforeStart, REALTIME);
        return ERR_SUCCESS;
    }
   
    // 发送过程中的处理问题
    if(nb._process.last == Process_Run
       && nb.socketCurrent != NULL)
    {
        location = strstr((char *)(unit->message), "OK");
        if(location != NULL)
        {
            switch(nb.socketCurrent->_process.current)
            {
            case Process_BeforeStart:
                httpParam->_counterConfig += 1;
                if(httpParam->config[httpParam->_counterConfig] == NULL)
                {   
                    PROCESS_CHANGE_WITH_TIME(nb.socketCurrent->_process, Process_Start, REALTIME);    
                    httpParam->_counterConfig = 0;
                }
                break;
            case Process_Start:
                PROCESS_CHANGE_WITH_TIME(nb.socketCurrent->_process, Process_BeforeRun, REALTIME); 
                break;
            case Process_BeforeRun:
                PROCESS_CHANGE_WITH_TIME(nb.socketCurrent->_process, Process_Run, REALTIME);
                break;
            case Process_Run:
                PROCESS_CHANGE_WITH_TIME(nb.socketCurrent->_process, Process_Wait, REALTIME);
                break;
            }
        }        
    }

    
    // 获取到内容则为+HTTPNMIC，获取到头则为+HTTPNMIH
    location = strstr((char *)(unit->message), "+HTTPNMIC");
    if (location != NULL)
    {
        /* 查看当前内容的数量 */
        socketId = -1;
        sscanf(location, "+HTTPNMIC:%d,%*d,%*d,%*d,%d\r\n", &socketId, &temp32);
        if(temp32 == 0)
        {   return ERR_SUCCESS; }
        
        location = strstr(location, "\r\n");
        
        /* 回复body提取可用字符串 */
        if (location != NULL)
        {                 
            location += 2;
            location[temp32] = 0;
            nbSocketTemp = nb.sockets;
            
            /* 根据socketId发到不同的socket中 */
            while(true)
            {
                if(nbSocketTemp == NULL)
                {   break;  }
                if(nbSocketTemp->_socketId == socketId)
                {
                    if(RxQueue_Add(&(nbSocketTemp->rxQueueApp), (uint8_t *)location, temp32, false) < 0)
                    {
                        if(nbSocketTemp->CallBack_ErrorHandle != NULL)
                        {   nbSocketTemp->CallBack_ErrorHandle(nbSocketTemp, ERR_ALLOC_FAILED); }  
                    }
                    break;
                }
                nbSocketTemp = nbSocketTemp->next;
            }
        }     
        PROCESS_CHANGE_WITH_TIME(nbSocketTemp->_process, Process_Init, REALTIME);        // 重启发送
        
        return ERR_SUCCESS;
    }
    
    return ERR_NO_TASK;
 }

/*********************************************************************************************

    NB: 处理

  ********************************************************************************************/
void NB_SocketLoopHandle(void (*handleFunc)(struct NBSocketStruct *, void *param), void *param)
{
    struct NBSocketStruct *nbSocketTemp = nb.sockets;
    while(true)
    {
        if(nbSocketTemp == NULL)
        {   break;  }
        (*handleFunc)(nbSocketTemp, param);
        nbSocketTemp = nbSocketTemp->next;
    }
}
/*********************************************************************************************

    NBSOCKET 相关回调, 重置所有的socket

  ********************************************************************************************/
void NBSocket_Reset(struct NBSocketStruct *socket, void *param)
{
    bool isAllClear = (param == NULL)? false: *(bool *)param;
  
    socket->_flag = 0;
    socket->_socketId = -1;
    socket->_counterError = 0;
    
    PROCESS_CHANGE(socket->_process, Process_Init);
    
    if(isAllClear == true)
    {   TxQueue_FreeAll(&(socket->txQueueApp)); }
}
/*********************************************************************************************

    NBSOCKET 指定id的错误处理

  ********************************************************************************************/
void NBSocket_ErrorHandleById(struct NBSocketStruct *socket, void *param)
{
      uint32_t temp32 = *((uint32_t *)param);
      
      if(socket->_socketId == temp32)
      {
            if(socket->CallBack_ErrorHandle != NULL)
            {   socket->CallBack_ErrorHandle(socket, ERR_CON_CLOSE);        }
      }
}       