/* Includes ------------------------------------------------------------------*/
#include "../Common/Common.h"
#include "NB.h"

/* Private define -------------------------------------------------------------*/
#define NB_HTTP_HEADER  ""


/* Private macro --------------------------------------------------------------*/
/* Private typedef ------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
NBStruct nb;

DATA_PREFIX char *nbConfiguration[] = {
    "AT+CGMR\r\n",
    "AT+CMEE=1\r\n",                        // 1. 开启错误序号提示
    "AT+NPSMR=1\r\n",                       // 2. 提示是否进入休眠
    "AT+NCONFIG=CELL_RESELECTION,true\r\n", // 3. 开启小区重选
    "AT+CEREG=4\r\n",                       // 4. 开启信息提示
    "AT+CSCON=1\r\n",                       // 5. 打开信号提示自动上报
    "AT+CFUN=1\r\n",                        // 6. 启动射频     ----> 启动连接
    "AT+CGATT=1\r\n",                       // 7. 开始附着
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
    NULL,                          
};

DATA_PREFIX char *nbReadSim[] = {
    "AT+CIMI\r\n",
    NULL,                          
};

/* Private function prototypes ------------------------------------------------*/
void NB_StringTrans(const char *string);
void NB_ErrorHandle(NbErrorEnum error);

/*********************************************************************************************

  * @brief  
  * @param  
  * @param  
  * @retval 

  ********************************************************************************************/    
void NB_SetProcess(ProcessEnum process)
{
    nb._lastProcess = nb._process;
    nb._process = process;
    nb.__time = realTime;
}
/*********************************************************************************************

  * @brief  
  * @param  
  * @param  
  * @retval 

  ********************************************************************************************/   
void NB_SendATCommandList(char **list)      
{                                   
    nb.cmdList = list;              
    NB_SetProcess(Process_OrderAt); 
    nb.__orderAt.index = 0;
}
/*********************************************************************************************

  * @brief  ESP8266的主处理
  * @param  
  * @param  
  * @retval 

  ********************************************************************************************/    
void NB_PowerOn()
{
    /* 有数据时且WIFI电源为切断时，开启WIFI电源，重置运行流程 */    
    NB_POWER_ON();
    NB_SetProcess(Process_Reset);
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
    
    
    switch(nb._process)
    {
    /* 上电先查看SIM卡信息，之后才会有不同的处理方式 */
    case Process_Init:
        if(nb.sim == Unknown)
        {   NB_SendATCommandList(nbReadSim);  }
        else
        {   NB_SetProcess(Process_Idle);    }
        break;
   
    /* 空闲状态，没什么事就进入处理 */ 
    case Process_Idle:
        switch(nb.sim)
        {
        case Unknown:
            break;
        case ChinaMobile:
            nb.CallBack_StartConenct = NB_Http_StartConnect;
            nb.CallBack_ReceiveHandle = NB_Http_ReceiveHandle;
            nb.txQueueService->CallBack_PackagBeforeTransmit = NB_Http_PacketPackage;

            break;
        case ChinaTelecom:
            nb.CallBack_HandleBeforeNetting = NB_OC_HandleBeforeNetting;
            nb.CallBack_ReceiveHandle = NB_OC_ReceiveHandle;
            nb.txQueueService->CallBack_PackagBeforeTransmit = NB_OC_PacketPackage;
            break;
        }
      
        if(nb.CallBack_HandleBeforeNetting != NULL)
        {   nb.CallBack_HandleBeforeNetting(); }
        else
        {   NB_SendATCommandList(nbConfiguration);  }               // 进入连接处理
        break;

    /* 按顺序发送AT指令，当发送完成后切换到等待状态
     根据接收到的AT中包含的OK或者ERROR进行下一步的判断*/
    case Process_OrderAt:
        NB_StringTrans((const char *)nb.cmdList[nb.__orderAt.index]);
        nb.__orderAt.isGetOk = false;
        nb.__orderAt.isGetError = false;
        NB_SetProcess(Process_OrderAtWait);
        break;

    /* 等待3s如果没有回复则重发命令 
     收到OK，则根据当前的索引进行下一步的操作，默认为继续发下一个AT指令
     收到错误，则编写对应的错误处理 */
    case Process_OrderAtWait:
        if ((nb.__time + 5) < realTime)
        {   
            NB_SetProcess(Process_OrderAt);   
            nb.__orderAt.errorCounter ++;
            if(nb.__orderAt.errorCounter > 5)
            {   NB_ErrorHandle(NbError_AtError);    }
        }
        else
        {
            /* AT指令发送成功 */
            if (nb.__orderAt.isGetOk == true)
            {
                nb.__orderAt.index++;                       // AT指令索引+1
                
                /* 如果发现发送到末尾，则进入处理部分 */
                if(nb.cmdList[nb.__orderAt.index] != NULL)
                {   NB_SetProcess(Process_OrderAt); }
                else
                {   
                    if(nb.cmdList == nbConfiguration)        // 配置参数发送完成 => 进入查询附着状态
                    {   
                        NB_SetProcess(Process_Wait);    
                        nb.__startConnectTime = realTime;
                    }  
                    else if (nb.cmdList == nbQuery)
                    {   NB_SetProcess(Process_Start);   }   // 查询参数发送完成 => 进入准备发送状态
                    else if (nb.cmdList == nbSetBand)
                    {   NB_SetProcess(Process_Reset);   }
                    else
                    {   NB_SendATCommandList(nbConfiguration);  }   // 其他的则直接进入连接状态部分，有可能有的在初始化部分要处理
                }
            }
            else if (nb.__orderAt.isGetError == true)
            {}
        }
        break;

    /* 等待连接 */
    case Process_Wait:
        // 每5s发送一次查询连接
        if ((nb.__time + 2) < realTime)
        {
            nb.__time = realTime;
            NB_StringTrans("AT+CGATT?\r\n");
        }

        // 长时间之后未连接处理
        if ((nb.__startConnectTime + 30) < realTime)
        {   NB_ErrorHandle(NbError_AttTimeout); }
        break;
        
    /* 长时间等待，一般是发了AT指令没有正确回复 */
    case Process_LongWait:
        if ((nb.__time + 3) < realTime)
        {
            
        }
        break;

    /* 启动连接 */
    case Process_Start:
        if(nb.CallBack_StartConenct != NULL)
        {   
            nb.CallBack_StartConenct(); 
            NB_SetProcess(Process_Wait);
        }
        else
        {   NB_SetProcess(Process_Run);    }
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
            if ((nb.__time + 2) < realTime)
            {   nb._isTransmitting = false; }
        }
        break;

    case Process_Reset:
        NB_StringTrans("AT+NRB\r\n");
        NB_SetProcess(Process_ResetWait);
        break;

    case Process_ResetWait:
        if((nb.__time + 10) < realTime)
        {   NB_ErrorHandle(NbError_AtError);    }
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


    if (nb.cmdList == nbReadSim && (location = strstr(message, "460")) != NULL)
    {   
        if(location[4] == '0' || location[4] == '2' || location[4] == '4' || location[4] == '7')
        {   
            nb.sim = ChinaMobile;  
            nbSetBand[2][9] = '8';
            NB_SendATCommandList(nbSetBand);
        }
        else if(location[4] == '3' || location[4] == '5' || (location[3] == '1' && location[4] == '1'))
        {   
            nb.sim = ChinaTelecom;  
            nbSetBand[2][9] = '5';
            NB_SendATCommandList(nbSetBand);
        }
        
    }
    
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
        {   NB_SendATCommandList(nbQuery);   }

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
        //timeStampCounter = Calendar2TimeStamp(&calendar, 0);
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
    
    // 重启完成
    if (strstr(message, "REBOOT_") != NULL && nb._process == Process_ResetWait)
    {
        NB_SetProcess(Process_Init);   
        return;
    }
    
    /* 发送过程中失败的问题 */
    if(nb._process == Process_Run)
    {
        if(strstr(message, "ERROR"))
        {   
            nb._errorCounter++;
            if(nb._errorCounter > 5)
            {   
                nb._errorCounter = 0;
                NB_ErrorHandle(NbError_TxFail); 
            }
        }
    }
    
    // 针对不同应用的接收处理部分
    if(nb.CallBack_ReceiveHandle != NULL)
    {   nb.CallBack_ReceiveHandle(message, len); }
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
    case NbError_AttTimeout:        // 附着失败
    case NbError_AtError:
    case NBError_ConnectError:
        NB_SetProcess(Process_Lock);
        break;
        
    case NbError_NeedReset:
        NB_SetProcess(Process_Init);
        nb._errorCounter = 0;
    case NbError_TxFail:
    default:
        break;
    }
    if(nb.CallBack_TxError != NULL)
    {   nb.CallBack_TxError(error); }
}
