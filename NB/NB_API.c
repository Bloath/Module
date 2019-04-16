/* Includes ------------------------------------------------------------------*/
#include "../Common/Common.h"
#include "NB.h"

/* Private define -------------------------------------------------------------*/
#define NB_HTTP_HEADER  ""
/* Private macro --------------------------------------------------------------*/
/* Private typedef ------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
NBStruct nb;

__persistent const char *nbCmd[] = {
    "AT+CFUN=0\r\n",                        // 0. 关闭射频
    "AT+CMEE=1\r\n",                        // 1. 开启错误序号提示
    "AT+NPSMR=1\r\n",                       // 2. 提示是否进入休眠
    "AT+NCONFIG=CELL_RESELECTION,true\r\n", // 3. 开启小区重选
    "AT+CEREG=4\r\n",                       // 4. 开启信息提示
    "AT+CFUN=1\r\n",                        // 5. 启动射频     ----> 启动连接
    "AT+CGATT=1\r\n",                       // 6. 开始附着
    "AT+CGDCONT?\r\n",                      // 7. 开启PSM
    "AT+CGMR\r\n",
    "AT+CPSMS=1,,,00111001,00000001\r\n",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "AT+CFUN=0\r\n",                        // 21. 关闭射频     ----> 重新连接
    "AT+NCSEARFCN\r\n",                     // 22. 清除先验频点
    "AT+CSQ\r\n",                           // 23. 信号强度
    "AT+CCLK?\r\n"                          // 24. 时间
};

char *nbSocketConfig = "AT+HTTPHEADER=1,\"Connection: close\"\r\n";

/* Private function prototypes ------------------------------------------------*/
void NB_StringTrans(const char *string);
void NB_ErrorHandle(NbErrorEnum error);

#define NB_JUMP_ORDERAT(x)            \
    {                                 \
        NB_SetProcess(Process_OrderAt);\
        nb.__orderAt.index = x;       \
    }

#define NB_AT_CONNECTED   9

#define NB_INIT() NB_JUMP_ORDERAT(0)
#define NB_FINISH_RST() NB_JUMP_ORDERAT(2)
#define NB_ClEAR_CONNECT() NB_JUMP_ORDERAT(21)
#define NB_START_CONNECT() NB_JUMP_ORDERAT(5)
#define NB_GET_SS_TIME() NB_JUMP_ORDERAT(23)

#define NB_RETRY_CON_NEXT()              \
    {                                      \
        nb.__startConnectTime = realTime;  \
        NB_SetProcess(Process_LongWait);   \
    }
#define NB_SetProcess(process)  \
    {                                       \
        nb._lastProcess = nb._process;      \
        nb._process = process;              \
    }
/*********************************************************************************************

  * @brief  NB主处理函数
  * @param  
  * @retval 无
  * @remark 放置在主程序中

  ********************************************************************************************/
void NB_Handle()
{
    TxQueue_Handle(nb.txQueueHal, nb.CallBack_HalTxFunc, nb.halTxParam);        // 硬件层 发送队列处理
    char *httpCreate = NULL;

    switch (nb._process)
    {
    /* 初始化时设置参数
     参数设置只要模块没问题基本都会成功，直接跳转即可 */
    case Process_Init:
        nb.txQueueHal->interval = NB_HAL_TX_INTERVAL;
        nb.txQueueHal->isTxUnordered = true;
        nb.txQueueHal->maxTxCount = NB_HAL_RETX_COUNT;
        
        nb.txQueueService->interval = NB_SERVICE_TX_INTERVAL;
        nb.txQueueService->isTxUnordered = true;
        NB_INIT();
        break;

    /* 按顺序发送AT指令，当发送完成后切换到等待状态
     根据接收到的AT中包含的OK或者ERROR进行下一步的判断*/
    case Process_OrderAt:
        NB_StringTrans(nbCmd[nb.__orderAt.index]);
        nb.__orderAt.isGetOk = false;
        nb.__orderAt.isGetError = false;
        nb.__time = realTime;
        NB_SetProcess(Process_OrderAtWait);
        break;

    /* 等待3s如果没有回复则重发命令 
     收到OK，则根据当前的索引进行下一步的操作，默认为继续发下一个AT指令
     收到错误，则编写对应的错误处理 */
    case Process_OrderAtWait:
        if ((nb.__time + 3) < realTime)
        {   NB_SetProcess(Process_OrderAt);   }
        else
        {
            if (nb.__orderAt.isGetOk == true)
            {
                switch (nb.__orderAt.index)
                {
                case 1:                         // 完成NCDP设置
                    NB_SetProcess(Process_Reset);
                    break;
                case NB_AT_CONNECTED:           // 完成复位后的启动射频以及启动附着设置
                    nb.__startConnectTime = realTime;
                    NB_SetProcess(Process_Wait);
                    break;
                case 22:                        // 完成清除连接信息后跳转到重新连接
                    NB_START_CONNECT();
                    break;

                case 24: //
                    if (timeStamp < 1500000000)
                    {   NB_GET_SS_TIME();   }
                    else
                    {   NB_SetProcess(Process_Start);   }
                    break;

                default: // 其他的则顺序向下执行
                    nb.__orderAt.index++;
                    NB_SetProcess(Process_OrderAt);
                    break;
                }
            }
            if (nb.__orderAt.isGetError == true)
            {
                switch (nb.__orderAt.index)
                {
                case 5:                  // "AT+CFUN=1 ERROR"
                    NB_ErrorHandle(NbError_Undetected);     // 直接等待下次连接
                    break;
                }
            }
        }
        break;

    /* 等待连接 */
    case Process_Wait:
        // 每5s发送一次查询连接
        if ((nb.__time + 5) < realTime)
        {
            nb.__time = realTime;
            NB_StringTrans("AT+CGATT?\r\n");
        }

        // 60s之后未连接，则等待一天后再次重连
        if ((nb.__startConnectTime + 60) < realTime)
        {   NB_ErrorHandle(NbError_Undetected); }
        break;

    /* 长时间等待，等待下次重连，24小时后重连 */
    case Process_LongWait:
        if ((nb.__startConnectTime + SECONDS_DAY) < realTime)
        //{   NB_ClEAR_CONNECT(); }
        {   NB_SetProcess(Process_Reset);   }
        break;

    /* 启动UDP */
    case Process_Start:
        httpCreate = (char *)Malloc(64);
        strcat(httpCreate, "AT+HTTPCREATE=\"http://");
        strcat(httpCreate, nb.host);
        strcat(httpCreate, ":");
        strcat(httpCreate, nb.port);
        strcat(httpCreate, "\"\r\n");
        NB_StringTrans(httpCreate);
        Free(httpCreate);
        nb.__time = realTime;
        NB_SetProcess(Process_Wait);
        break;
        
    /* 开始工作部分，对于NB来说，有数据直接发送即可，等待回复 */
    case Process_Run:
        TxQueue_Handle(nb.txQueueService, nb.CallBack_HalTxFunc, NULL);
        if (nb.txQueueService->_usedBlockQuantity != 0)
        {
            nb.__time = realTime;
            nb._isTransmitting = true;
        }

        if (nb._isTransmitting == true 
            && nb.txQueueService->_usedBlockQuantity == 0 
            && nb.rxQueueService->_usedBlockQuantity == 0)
        {
            // 延迟4s再进入休眠
            if ((nb.__time + 4) < realTime)
            {
                //NB_StringTrans("AT+MLWULDATAEX=1,FE,0x0101\r\n");
                NB_StringTrans("AT+HTTPCLOSE=0\r\n");
                nb._isTransmitting = false;
            }
        }
        break;

    case Process_Reset:
        NB_StringTrans("AT+NRB\r\n");
        NB_SetProcess(Process_ResetWait);
        nb.__time = realTime;
        break;

    case Process_ResetWait:
        if((nb.__time + 10) < realTime)
        {   NB_SetProcess(Process_Reset);   }
        break;
    }
}
/*********************************************************************************************

  * @brief  NB接收部分处理
  * @param  packet：接收数据包
            len：数据包长度
            param：参数
  * @retval 无
  * @remark 

  ********************************************************************************************/
void NB_RxHandle(uint8_t *packet, uint16_t len, void *param)
{
    char *message = (char *)packet;
    char *location = NULL, *temp = NULL;

    // 在顺序发送AT出现OK或者ERROR的情况
    if (nb._process == Process_OrderAtWait)
    {
        if (strstr(message, "OK") != NULL)
        {   
            nb.__orderAt.isGetOk = true;  
            nb.__orderAt.errorCounter = 0;
        }

        if (strstr(message, "ERROR") != NULL)
        {   
            nb.__orderAt.isGetError = true;   
            nb.__orderAt.errorCounter ++;
            if(nb.__orderAt.errorCounter > 3)
            {   NB_ErrorHandle(NbError_AtError);    }
        }
    }

    // 是否附着正常判断 CGATT
    location = strstr(message, "+CGATT:1");
    if (location != NULL)
    {
        if (location[7] == '1')
        {   NB_GET_SS_TIME();   }

        return;
    }
    
    // 是否正常打开SOCKET
    if (nb._lastProcess == Process_Start)
    {
        if(strstr(message, "ERROR"))
        {   NB_INIT();  }
        else
        {
            location = strstr(message, "\r\n\r\nOK\r\n");
            location -= 1;
            if( *location >= 30 && *location <= 0x39)
            {
                nb.socketId = *location - 0x30;
                NB_SetProcess(Process_Run);
                nbSocketConfig[14] = nb.socketId + 0x30;
                NB_StringTrans(nbSocketConfig);
            }
            else
            {   NB_INIT();  }
        }
        return;
    }

    // 接收到日期 -> 正式进入发送状态
    location = strstr(message, "+CCLK");
    if (location != NULL)
    {
        CalendarStruct calendar;
        calendar.year = 2000 + (location[6] - 0x30) * 10 + location[7] - 0x30;
        calendar.month = (location[9] - 0x30) * 10 + location[10] - 0x30;
        calendar.day = (location[12] - 0x30) * 10 + location[13] - 0x30;
        calendar.hour = (location[15] - 0x30) * 10 + location[16] - 0x30;
        calendar.min = (location[18] - 0x30) * 10 + location[19] - 0x30;
        calendar.sec = (location[21] - 0x30) * 10 + location[22] - 0x30;
        //uint8_t timeZone = (location[24] - 0x30) * 10 + location[25] - 0x30;
        timeStampCounter = Calendar2TimeStamp(&calendar, 0);
    }

    // 信号强度判断
    location = strstr(message, "+CSQ");
    if (location != NULL)
    {
        temp = String_CutByChr(location, ':', ','); // 将信号强度部分裁剪出来
        uint32_t temp32u = NumberString2Uint(temp); // 转换为数字

        // 0-2 微弱信号 2-10一般 11-31较强 99收不到
        if (temp32u < 2)
        {   nb._signal = NbSignal_Weak;  }
        else if (temp32u == 99)
        {   nb._signal = NbSignal_Undetected;    }
        else
        {   nb._signal = NbSignal_Normal;    }
        Free(temp);
        return;
    }
    
    // 获取到内容则为+HTTPNMIC，获取到头则为+HTTPNMIH
    location = strstr(message, "+HTTPNMIC");
    if (location != NULL)
    {
        nb._errorCounter = 0;
        location = strstr(message, "\n68");
            
        /* 回复body提取可用字符串 */
        if (location != NULL)
        {   
            location += 1;                 
            
            /* 找到换行符，将换行符改为结束符，再对报文进行转换 */
            temp = strstr(location, "16\r\n");
            if(temp != NULL)
            {   temp[2] = '\0';   }
            
            uint8_t *packet = NULL;
            int count = String2Msg(&packet, location, 0);
            if (count > 0)
            {   RxQueue_Add(nb.rxQueueService, packet, count, true);     }
        }     
        else
        {   ESP8266_ErrorHandle(Error_ResponseError);   }
        return;
    }

    // 重启完成
    if (strstr(message, "REBOOT_") != NULL && nb._process == Process_ResetWait)
    {
        NB_FINISH_RST();
        return;
    }
    
    /* 处于发送状态
     出现Error时，有可能并未入网，ERROR出现10次重启模块 */
    if (nb._process == Process_Run)
    {
//        if (strstr(message, "OK") != NULL)
//        {   TxQueue_FreeByIndex(nb.txQueueService, nb.txQueueService->_lastIndex); }

        // 在数据发送处理部分接收到error，需要复位
        if (strstr(message, "ERROR") != NULL || strstr(message, "HTTPERR") != NULL)
        {
            nb._errorCounter++;

            // 发现错误次数比较多，（HTTP CREATE是可以打开的，发送才会出现ERROR）
            if ((nb._errorCounter / 4) >= 2)
            {   NB_ErrorHandle(NbError_NeedReset); }
            else if ((nb._errorCounter % 4) == 3)
            {   NB_ErrorHandle(NbError_TxFail); }
        }
    }
}
/*********************************************************************************************

  * @brief  NB_DataPackage
  * @param  block
  * @retval 无
  * @remark 

  ********************************************************************************************/
int NB_DataPackage(TxBaseBlockStruct *block, void *param, PacketStruct *packet)//需要处理Malloc
{
    /* 申请内存
       AT+NMGS= 8位
       长度+逗号 4位
       \r\n 2位      */
    uint16_t totalLen = 40 + block->length * 2;
    char *msg = (char *)Malloc(totalLen);

    /* 拼接指令协议 */
    strcat(msg, "AT+HTTPSEND=");                        // AT头
    Uint2String(msg, nb.socketId);                      // 填充数字
    strcat(msg, ",0,");                                 // 填充，
    strcat(msg, "/communication?message=");             // 填充path
    Msg2String(msg, block->message, block->length);     // 填充报文
    strcat(msg, "\r\n");                                // 填充换行

    packet->data = (uint8_t *)msg;
    packet->length = strlen(msg);                       // 将打包好的数据指向packet，后面会自己free
    
    return 0;
}
/*********************************************************************************************

  * @brief  NB 字符串发送
  * @param  string：字符串
  * @retval 无
  * @remark 

  ********************************************************************************************/
void NB_StringTrans(const char *string)
{
    TxQueue_Add(nb.txQueueHal, (uint8_t *)string, strlen(string), TX_ONCE_AC);
}
/*********************************************************************************************

  * @brief  NB_ErrorHandle
  * @param  
  * @retval 
  * @remark NB错误处理

  ********************************************************************************************/
void NB_ErrorHandle(NbErrorEnum error)
{
    switch(error)
    {
    // 未发现网络
    case NbError_Undetected:
    case NbError_AtError:
        NB_RETRY_CON_NEXT();        // 
        break;
        
    case NbError_NeedReset:
        NB_SetProcess(Process_Init);
        nb._errorCounter = 0;
    case NbError_TxFail:
    default:
        if(nb.CallBack_TxError != NULL)
        {   nb.CallBack_TxError(error); }
        break;
    }
  
    
}
