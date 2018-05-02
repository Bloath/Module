/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"

#include "../Module/Sys_Conf.h"
#include "../Module/BufferQueue/BufferQueue.h"
#include "../Module/Common/Array.h"
#include "Communicate_API.h"
#include "Communicate_Handle.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
void Communicate_RxHandle(uint8_t *message, uint16_t len, void *param);

/*********************************************************************************************

  * @brief  ���Ϳ������
  * @param  communicate����˫��ʵ��ָ��
  * @retval ��
  * @remark 

  ********************************************************************************************/
void Comunicate_HDMaster_Polling(CommunicateStruct *communicate)
{
  /* ������״̬���д��� */
  switch(communicate->process)
  {
  /* ��ʼ�� */
  case Process_Init:
    communicate->process = Process_Idle;
    communicate->time = *communicate->refTime;
    break;
  
  /* ��ѯ���ʱ�� */
  case Process_Idle:
     /* ���һ��ʱ�� */
    if((communicate->time + communicate->loopInterval) > *communicate->refTime)
    { break; }
    else
    { communicate->process = Process_Start; }
  
  /* ����״̬������ѯ�ݴ汨�ģ��л�Ϊ�ȴ�״̬ */
  case Process_Start:
    CallBack_HD_FillPollingPackt(communicate);     // �������ݰ����
    communicate->process = Process_Wait;        // �л��ȴ�״̬
    break;
    
  /* �ȴ�״̬�����ͻ���Ϊ�ֶ���������û�лظ��Ļ��ǲ�������ģ������������� 
     �ȴ�״̬Ϊ������ֱ�����յ�ȷ�ϱ��Ļ��߲�ѯ�ݴ�ظ�*/
  case Process_Wait:
     communicate->time = *communicate->refTime;
    break;
  }
}

/*********************************************************************************************

  * @brief  ��˫��ͨѶ ������
  * @param  communicate����˫������
  * @retval ��
  * @remark 

  ********************************************************************************************/
void Communicate_Handle(CommunicateStruct *communicate)
{
  /* ��˫�� + ����ģʽ������ */
  if(communicate->isFullDuplex == FALSE && communicate->isHDMaster == TRUE)
  { Comunicate_HDMaster_Polling(communicate);}
  
  /* ���Ͷ��д��� */
  if(communicate->CallBack_TxFunc != NULL)
  { TxQueue_Handle(communicate->txQueue, communicate->CallBack_TxFunc); }
  
  RxQueue_Handle(communicate->rxQueue, Communicate_RxHandle, (void*)communicate); 
}

/*********************************************************************************************

  * @brief  ��˫��ͨѶ���������մ���
  * @param  message:  ���Ϳ�ָ��
            len�����ĳ���
            param������
  * @retval 
  * @remark 

  ********************************************************************************************/
void Communicate_RxHandle(uint8_t *message, uint16_t len, void *param)
{
  CommunicateStruct *communicate = (CommunicateStruct *)param;      // ָ���ȡ
  
  ArrayStruct *msg = CallBack_MsgConvert(message, len, communicate);
  if(msg != NULL)
  { 
    message = msg->packet;
    len = msg->length;
    Array_Free(msg);
  }
  
  /* ���ļ�� */
  if(CallBack_MessageInspect(message, len, communicate) == FALSE)
  { return; }
  
  /* �����Ӧ���Ͷ��� */
  CallBack_ClearTxQueue(message, len, communicate);
  communicate->process = Process_Idle;
  
  if(communicate->isFullDuplex == FALSE)
  {
    /* ��˫�������
       ��˫��-��ģʽ�����·����ݴ�
       ��˫��-��ģʽ����Ҫ������� */
    if(CallBack_HD_isPooling(message, len, communicate) == TRUE )
    { 
      if(communicate->isHDMaster == FALSE)
      { CallBack_HDSlave_PollHandle(message, len, communicate); }
      
      return;
    }
  }
  
  CallBack_UnpollMsgHandle(message, len, communicate);     // ����ѯ���Ĵ���
}

