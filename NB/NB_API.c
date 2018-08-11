/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"

#include "../BufferQueue/BufferQueue.h"
#include "../Common/Common.h"

#include "NB_HAL.h"
#include "NB_Handle.h"
#include "NB_Conf.h"
#include "NB_API.h"

/* Private define -------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private typedef ------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
TxQueueStruct NB_HalTxQueue;                   // 模块硬件部分发送，用于发送AT指令等
TxQueueStruct NB_TxQueue;                      // 模块的发送缓冲
RxQueueStruct NB_RxQueue;                      // 模块的接收缓冲

NbStruct nb;

/* Private function prototypes ------------------------------------------------*/
void NB_DataTrans(uint8_t *message, uint16_t len);
void NB_StringTrans(const char *string);

/*********************************************************************************************

  * @brief  NB主处理函数
  * @param  
  * @retval 无
  * @remark 放置在主程序中

  ********************************************************************************************/
void NB_Handle()
{
  TxQueue_Handle(&NB_HalTxQueue, NB_HalTrans);
  RxQueue_Handle(&NB_RxQueue, NB_RxMsgHandle, 0);
  
  switch(nb.process)
  {
  // 初始化时设置参数
  // 参数设置只要模块没问题基本都会成功，直接跳转即可
  case Process_Init:
    NB_HalTxQueue.interval = NB_TX_AT_INTERVAL;
    NB_HalTxQueue.maxTxCount = NB_RETX_COUNT;
    NB_HalTxQueue.isTxUnordered = TRUE;
    
    NB_TxQueue.interval = NB_TX_DATA_INTERVAL;
    NB_TxQueue.maxTxCount = NB_RETX_COUNT;
    NB_TxQueue.isTxUnordered = TRUE;
    
    NB_StringTrans("AT+NNMI=1\r\n");
    //NB_StringTrans("AT+NCDP?\r\n");
    //NB_StringTrans("AT+NCDP=180.101.147.115\r\n");
    //NB_StringTrans("AT+CGSN=1\r\n");
    nb.process = Process_Idle;
    break;
    
  // 空闲时，确认信号强度、连接等信息， 
  // 返回+CSQ:21,99， 0-1微弱（拔天线）、2-31有信号, 99 SIM卡拔出
  // 测试信号强度等，需要进行复位（SIM卡信息只会在上电读一次）
  case Process_Idle:
    if((nb.time + 3) < realTime)
    { 
      NB_StringTrans("AT+CSQ\r\n");
      //NB_StringTrans("AT+CSCON?\r\n");
      nb.time = realTime;
    }
    break;
    
    
  // 开始工作部分，对于NB来说，有数据直接发送即可，等待回复
  // 
  case Process_Start:
    TxQueue_Handle(&NB_TxQueue, NB_HalTrans);
    break;
   
    
  // 等待状态，等待参数设置完成等
  // 在接收到数据时，在接收处理时进行切换，只处理超时等问题
  case Process_Wait:
    if((nb.time + 60) < realTime)
    { 
      nb.time = realTime;
      nb.process = Process_Reset;
    }
    break;
    
  case Process_Reset:
    NB_HalTrans("AT+NRB\r\n", 8);
    nb.process = Process_Wait;
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
  extern TxQueueStruct debugTxQueue;
  TxQueue_Add(&debugTxQueue, packet, len, TX_ONCE_AC);
#endif
  
  char *message = (char *)packet;
  char *data = NULL;
  char *location = NULL;
  
  // 接收到日期
  location = strstr(message, "+CCLK");
  if(location != NULL)
  {
    CalendarStruct calendar;
    calendar.year = 2000 + (location[6] - 0x30) * 10 + location[7] - 0x30;
    calendar.month = (location[9] - 0x30) * 10 + location[10] - 0x30;
    calendar.day = (location[12] - 0x30) * 10 + location[13] - 0x30;
    calendar.hour = (location[15] - 0x30) * 10 + location[16] - 0x30;
    calendar.min = (location[18] - 0x30) * 10 + location[19] - 0x30;
    calendar.sec = (location[21] - 0x30) * 10 + location[22] - 0x30;
    uint8_t timeZone = (location[24] - 0x30) * 10 + location[25] - 0x30;
    timeStampCounter = Calendar2TimeStamp(&calendar, timeZone);
  } 
  
  // 信号强度判断
  location = strstr(message, "+CSQ");
  if(location != NULL)
  {
    data = String_CutByChr(location, ':', ',');         // 将信号强度部分裁剪出来
    uint32_t temp32u = NumberString2Uint(data);         // 转换为数字
    
    // 0-10 微弱信号 11-31较强 99收不到
    if(temp32u < 10)
    { nb.signal = NbSignal_Weak; }
    else if(temp32u == 99)
    { nb.signal = NbSignal_Undetected; }
    else
    { 
      nb.signal = NbSignal_Normal; 
      nb.process = Process_Start;
      NB_StringTrans("AT+CCLK?\r\n");
    }
    
    Free(data);
    
    return;
  }

  // 连接状态判断
  // 2018-07-25 去掉，当无数据发送时，模块不会主动加入核心网
  //            对于数据来说，不断重复发送，等待ok即可
//  location = strstr(message, "+CSCON");
//  if(location != NULL)
//  {
//    if(location[9] == '1')
//    { nb.process = Process_Start; }
//    else if 
//    {}
//    
//    return;
//  }
  
  // 接收到数据
  if(strstr(message, "+NNMI") != NULL)
  {
    char *msgStr = String_CutByChr(strstr(message, "+NNMI"), ',', '\r');         // 将数据从指令中裁剪出来
    ArrayStruct *msg = String2Msg(msgStr, 0);                   // 转换为字节数组格式
    TxQueue_FreeById(&NB_TxQueue, msg->packet[0]);              // 清楚发送缓冲中与接收字段相同的快
    RxQueue_Add(&NB_RxQueue, msg->packet, msg->length);         // 填充到接收缓冲当中
    Array_Free(msg);
    Free(msgStr);
    return;
  }
  
  
  // 重启完成
  if(strstr(message, "REBOOT_") != NULL)
  {
    nb.process = Process_Init;
    return;
  }
  
  // 处于发送状态
  // 出现Error时，有可能并未入网，ERROR出现10次重启模块
  if(nb.process == Process_Start)
  {
    // 在数据发送处理部分接收到error，需要复位
    if(strstr(message, "ERROR") != NULL)
    {
      nb.errorCounter ++;
      
      if(nb.errorCounter > NB_TX_DATA_MAX)
      {
        nb.process = Process_Reset;
        nb.errorCounter = 0;
      }
    }
    if(strstr(message, "OK") != NULL)
    { nb.errorCounter = 0; }
  }
  
 
}

/*********************************************************************************************

  * @brief  NB 字符串发送
  * @param  string：字符串
  * @retval 无
  * @remark 

  ********************************************************************************************/
void NB_StringTrans(const char *string)
{
  TxQueue_Add(&NB_HalTxQueue, (uint8_t *)string, strlen(string), TX_ONCE_AC);
}
/*********************************************************************************************

  * @brief  NB 字符串发送
  * @param  message:数据
            len：长度
  * @retval 无
  * @remark 

  ********************************************************************************************/
void NB_DataTrans(uint8_t *message, uint16_t len)
{
  // 申请内存 
  // AT+NMGS= 8位
  // 长度+逗号 4位
  // \r\n 2位    
  char *packet = (char*)Malloc(14 + len * 2);        
  memset(packet, 0, 14 + len * 2);  
  
  /* 拼接指令协议 */
  strcat(packet, "AT+NMGS=");                   // AT头
  
  char* msgLenString = Uint2String(len);
  strcat(packet, msgLenString);                 // 填充报文长度
  Free(msgLenString);
  
  strcat(packet, ",");
  
  char* msgString = Msg2String(message, len);   
  strcat(packet, msgString);                    // 填充报文
  Free(msgString);
  
  strcat(packet, "\r\n");                       // 填充换行

  // 将报文命令作为ID填充到队列中
  TxQueue_AddWithId(&NB_TxQueue, (uint8_t *)packet, strlen(packet), TX_MULTI_AC, message[0]);
  
  Free(packet);
}