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
TxQueueStruct NB_HalTxQueue;                   // ģ��Ӳ�����ַ��ͣ����ڷ���ATָ���
TxQueueStruct NB_TxQueue;                      // ģ��ķ��ͻ���
RxQueueStruct NB_RxQueue;                      // ģ��Ľ��ջ���

NbStruct nb;

/* Private function prototypes ------------------------------------------------*/
void NB_DataTrans(uint8_t *message, uint16_t len);
void NB_StringTrans(const char *string);

/*********************************************************************************************

  * @brief  NB��������
  * @param  
  * @retval ��
  * @remark ��������������

  ********************************************************************************************/
void NB_Handle()
{
  TxQueue_Handle(&NB_HalTxQueue, NB_HalTrans);
  RxQueue_Handle(&NB_RxQueue, NB_RxMsgHandle, 0);
  
  switch(nb.process)
  {
  // ��ʼ��ʱ���ò���
  // ��������ֻҪģ��û�����������ɹ���ֱ����ת����
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
    
  // ����ʱ��ȷ���ź�ǿ�ȡ����ӵ���Ϣ�� 
  // ����+CSQ:21,99�� 0-1΢���������ߣ���2-31���ź�, 99 SIM���γ�
  // �����ź�ǿ�ȵȣ���Ҫ���и�λ��SIM����Ϣֻ�����ϵ��һ�Σ�
  case Process_Idle:
    if((nb.time + 3) < realTime)
    { 
      NB_StringTrans("AT+CSQ\r\n");
      //NB_StringTrans("AT+CSCON?\r\n");
      nb.time = realTime;
    }
    break;
    
    
  // ��ʼ�������֣�����NB��˵��������ֱ�ӷ��ͼ��ɣ��ȴ��ظ�
  // 
  case Process_Start:
    TxQueue_Handle(&NB_TxQueue, NB_HalTrans);
    break;
   
    
  // �ȴ�״̬���ȴ�����������ɵ�
  // �ڽ��յ�����ʱ���ڽ��մ���ʱ�����л���ֻ����ʱ������
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

  * @brief  NB���ղ��ִ���
  * @param  packet���������ݰ�
            len�����ݰ�����
            param������
  * @retval ��
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
  
  // ���յ�����
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
  
  // �ź�ǿ���ж�
  location = strstr(message, "+CSQ");
  if(location != NULL)
  {
    data = String_CutByChr(location, ':', ',');         // ���ź�ǿ�Ȳ��ֲü�����
    uint32_t temp32u = NumberString2Uint(data);         // ת��Ϊ����
    
    // 0-10 ΢���ź� 11-31��ǿ 99�ղ���
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

  // ����״̬�ж�
  // 2018-07-25 ȥ�����������ݷ���ʱ��ģ�鲻���������������
  //            ����������˵�������ظ����ͣ��ȴ�ok����
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
  
  // ���յ�����
  if(strstr(message, "+NNMI") != NULL)
  {
    char *msgStr = String_CutByChr(strstr(message, "+NNMI"), ',', '\r');         // �����ݴ�ָ���вü�����
    ArrayStruct *msg = String2Msg(msgStr, 0);                   // ת��Ϊ�ֽ������ʽ
    TxQueue_FreeById(&NB_TxQueue, msg->packet[0]);              // ������ͻ�����������ֶ���ͬ�Ŀ�
    RxQueue_Add(&NB_RxQueue, msg->packet, msg->length);         // ��䵽���ջ��嵱��
    Array_Free(msg);
    Free(msgStr);
    return;
  }
  
  
  // �������
  if(strstr(message, "REBOOT_") != NULL)
  {
    nb.process = Process_Init;
    return;
  }
  
  // ���ڷ���״̬
  // ����Errorʱ���п��ܲ�δ������ERROR����10������ģ��
  if(nb.process == Process_Start)
  {
    // �����ݷ��ʹ����ֽ��յ�error����Ҫ��λ
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

  * @brief  NB �ַ�������
  * @param  string���ַ���
  * @retval ��
  * @remark 

  ********************************************************************************************/
void NB_StringTrans(const char *string)
{
  TxQueue_Add(&NB_HalTxQueue, (uint8_t *)string, strlen(string), TX_ONCE_AC);
}
/*********************************************************************************************

  * @brief  NB �ַ�������
  * @param  message:����
            len������
  * @retval ��
  * @remark 

  ********************************************************************************************/
void NB_DataTrans(uint8_t *message, uint16_t len)
{
  // �����ڴ� 
  // AT+NMGS= 8λ
  // ����+���� 4λ
  // \r\n 2λ    
  char *packet = (char*)Malloc(14 + len * 2);        
  memset(packet, 0, 14 + len * 2);  
  
  /* ƴ��ָ��Э�� */
  strcat(packet, "AT+NMGS=");                   // ATͷ
  
  char* msgLenString = Uint2String(len);
  strcat(packet, msgLenString);                 // ��䱨�ĳ���
  Free(msgLenString);
  
  strcat(packet, ",");
  
  char* msgString = Msg2String(message, len);   
  strcat(packet, msgString);                    // ��䱨��
  Free(msgString);
  
  strcat(packet, "\r\n");                       // ��任��

  // ������������ΪID��䵽������
  TxQueue_AddWithId(&NB_TxQueue, (uint8_t *)packet, strlen(packet), TX_MULTI_AC, message[0]);
  
  Free(packet);
}