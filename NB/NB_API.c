/* Includes ------------------------------------------------------------------*/
#include "../Common/Common.h"
#include "NB_API.h"

/* Private define -------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private typedef ------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
NBStruct nb;

const char *nbCmd[] = {
    "AT+CFUN=0\r\n",                        // 0. 关闭射频
    "AT+NCDP=180.101.147.115\r\n",          // 1. 设置地址
    "AT+NNMI=1\r\n",                        // 2. 设置下发数据前缀
    "AT+NCONFIG=CELL_RESELECTION,true\r\n", // 3. 开启小区重选
    "AT+CEREG=1\r\n",                       // 4. 开启信息提示
    "AT+CFUN=1\r\n",                        // 5. 启动射频     ----> 启动连接
    "AT+CGATT=1\r\n",                       // 6. 开始附着
    "AT+CFUN=0\r\n",                        // 7. 关闭射频     ----> 重新连接
    "AT+NCSEARFCN\r\n",                     // 8. 清除先验频点
    "AT+CSQ\r\n",                           // 9. 信号强度
    "AT+CCLK?\r\n"                          // 10. 时间
};

/* Private function prototypes ------------------------------------------------*/
void NB_StringTrans(const char *string);

#define NB_JUMP_ORDERAT(x)            \
    {                                 \
        nb.process = Process_OrderAt; \
        nb.orderAt.index = x;         \
    }

#define NB_INIT() NB_JUMP_ORDERAT(0)
#define NB_FINISH_RST() NB_JUMP_ORDERAT(2)
#define NB_ClEAR_CONNECT() NB_JUMP_ORDERAT(7)
#define NB_START_CONNECT() NB_JUMP_ORDERAT(5)
#define NB_GET_SS_TIME() NB_JUMP_ORDERAT(9)

#define NB_RETRY_CON_NEXT()              \
    {                                    \
        if (nb.CallBack_TxError != NULL) \
        {                                \
            nb.CallBack_TxError();       \
        }                                \
        nb.startConnectTime = realTime;  \
        nb.process = Process_LongWait;   \
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

    switch (nb.process)
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
        NB_StringTrans(nbCmd[nb.orderAt.index]);
        nb.orderAt.isGetOk = false;
        nb.orderAt.isGetError = false;
        nb.time = realTime;
        nb.process = Process_OrderAtWait;
        break;

    /* 等待3s如果没有回复则重发命令 
     收到OK，则根据当前的索引进行下一步的操作，默认为继续发下一个AT指令
     收到错误，则编写对应的错误处理 */
    case Process_OrderAtWait:
        if ((nb.time + 3) < realTime)
        {   nb.process = Process_OrderAt;   }
        else
        {
            if (nb.orderAt.isGetOk == true)
            {
                switch (nb.orderAt.index)
                {
                case 1: // 完成NCDP设置
                    nb.process = Process_Reset;
                    break;
                case 6: // 完成复位后的启动射频以及启动附着设置
                    nb.startConnectTime = realTime;
                    nb.process = Process_Wait;
                    break;
                case 8: // 完成清除连接信息后跳转到重新连接
                    NB_START_CONNECT();
                    break;

                case 10: //
                    if (timeStamp < 1500000000)
                    {   NB_GET_SS_TIME();   }
                    else
                    {   nb.process = Process_Run;   }
                    break;

                default: // 其他的则顺序向下执行
                    nb.orderAt.index++;
                    nb.process = Process_OrderAt;
                    break;
                }
            }
            if (nb.orderAt.isGetError == true)
            {
                switch (nb.orderAt.index)
                {
                case 5:                  // "AT+CFUN=1 ERROR"
                    NB_RETRY_CON_NEXT(); // 直接等待下次连接
                    break;
                }
            }
        }
        break;

    // 等待连接
    case Process_Wait:
        // 每5s发送一次查询连接
        if ((nb.time + 5) < realTime)
        {
            nb.time = realTime;
            NB_StringTrans("AT+CGATT?\r\n");
        }

        // 60s之后未连接，则等待一天后再次重连
        if ((nb.startConnectTime + 60) < realTime)
        {   NB_RETRY_CON_NEXT();    }
        break;

    // 长时间等待，等待下次重连，24小时后重连
    case Process_LongWait:
        if ((nb.startConnectTime + 86400L) < realTime)
        {   NB_ClEAR_CONNECT(); }
        break;

    // 开始工作部分，对于NB来说，有数据直接发送即可，等待回复
    case Process_Run:
        TxQueue_Handle(nb.txQueueService, nb.CallBack_HalTxFunc, NULL);
        if (nb.txQueueService->usedBlockQuantity != 0)
        {
            nb.time = realTime;
            nb.isTransmitting = true;
        }

        if (nb.isTransmitting == true && nb.txQueueService->usedBlockQuantity == 0 && nb.rxQueueService->usedBlockQuantity == 0)
        {
            // 延迟4s再进入休眠
            if ((nb.time + 4) < realTime)
            {
                NB_StringTrans("AT+MLWULDATAEX=1,FE,0x0101\r\n");
                nb.isTransmitting = false;
            }
        }
        break;

    case Process_Reset:
        NB_StringTrans("AT+NRB\r\n");
        nb.process = Process_ResetWait;
        break;

    case Process_ResetWait:

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
#ifdef DEBUG
    if (nb.CallBack_HalRxGetMsg != NULL)
    {   nb.CallBack_HalRxGetMsg(packet, len, param);    }
#endif

    char *message = (char *)packet;
    char *data = NULL;
    char *location = NULL;

    // 在顺序发送AT出现OK或者ERROR的情况
    if (nb.process == Process_OrderAtWait)
    {
        if (strstr(message, "OK") != NULL)
        {   nb.orderAt.isGetOk = true;  }

        if (strstr(message, "ERROR") != NULL)
        {   nb.orderAt.isGetError = true;   }
    }

    // 是否附着正常判断 CGATT
    location = strstr(message, "+CGATT:1");
    if (location != NULL)
    {
        if (location[7] == '1')
        {   NB_GET_SS_TIME();   }

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
        data = String_CutByChr(location, ':', ','); // 将信号强度部分裁剪出来
        uint32_t temp32u = NumberString2Uint(data); // 转换为数字

        // 0-2 微弱信号 2-10一般 11-31较强 99收不到
        if (temp32u < 2)
        {   nb.signal = NbSignal_Weak;  }
        else if (temp32u == 99)
        {   nb.signal = NbSignal_Undetected;    }
        else
        {   nb.signal = NbSignal_Normal;    }

        Free(data);
        return;
    }

    // 接收到数据
    if (strstr(message, "+NNMI") != NULL)
    {
        nb.errorCounter = 0;

        char *tempPointer = (char *)message;
        bool isNeedCheck = false;
        for (uint8_t i = 0; i < 10; i++)
        {
            tempPointer = strstr(tempPointer, "+NNMI"); // 找到NMI
            if (tempPointer == NULL)
            {   break;  } // 没找到则直接退出

            /* 有可能发生字符串黏连的情况，则通过不同方式进行切割
               有换行符，则使用String_CutByChr，没有，则直接找到,后面字符作为指针 */
            char *msgStr = NULL;
            if (strstr(tempPointer, "\r") == NULL)
            {
                msgStr = (char *)((uint32_t)strstr(tempPointer, ",") + 1);
                isNeedCheck = false;
            }
            else
            {
                msgStr = String_CutByChr(tempPointer, ',', '\r');
                tempPointer = strstr(tempPointer, "\r");
                isNeedCheck = true;
            }

            uint8_t *msg = NULL;
            int count = String2Msg(&msg, msgStr, 0);                    // 转换为字节数组格式
            if(count > 0)
            {   RxQueue_Add(nb.rxQueueService, msg, count, true);     }   // 填充到接收缓冲当中
            
            Free(msgStr);

            if (isNeedCheck == false)
            {   break;  }
        }
    }

    // 重启完成
    if (strstr(message, "REBOOT_") != NULL && nb.process == Process_ResetWait)
    {
        NB_FINISH_RST();
        return;
    }

    /* 处于发送状态
     出现Error时，有可能并未入网，ERROR出现10次重启模块 */
    if (nb.process == Process_Run)
    {
        if (strstr(message, "OK") != NULL)
        {   TxQueue_FreeByIndex(nb.txQueueService, nb.txQueueService->lastIndex); }

        // 在数据发送处理部分接收到error，需要复位
        if (strstr(message, "ERROR") != NULL)
        {
            nb.errorCounter++;

            // 发现错误次数超过缓冲的重发次数
            if (nb.errorCounter >= nb.txQueueService->maxTxCount * 2)
            {
                nb.process = Process_Init;
                nb.errorCounter = 0;
            }
        }
    }
}
/*********************************************************************************************

  * @brief  NB_DataPackage
  * @param  block
  * @retval 无
  * @remark 

  ********************************************************************************************/
void NB_DataPackage(TxBaseBlockStruct *block, void *param, PacketStruct *packet)
{
    /* 申请内存
       AT+NMGS= 8位
       长度+逗号 4位
       \n 1位      */
    char *msg = (char *)Malloc(14 + block->length * 2);
    memset(msg, 0, 14 + block->length * 2);

    /* 拼接指令协议 */
    strcat(msg, "AT+NMGS=");                            // AT头
    Uint2String(msg, block->length);                    // 填充数字
    strcat(msg, ",");                                   // 填充，
    Msg2String(msg, block->message, block->length);     // 填充报文
    strcat(msg, "\r\n");                                  // 填充换行

    packet->data = (uint8_t *)msg;
    packet->length = strlen(msg);                       // 将打包好的数据指向packet，后面会自己free
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

