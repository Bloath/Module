#ifndef _COMMUNICATE_API_H_
#define _COMMUNICATE_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Module/Sys_Conf.h"
#include "../BufferQueue/BufferQueue.h"
/* Public typedef -----------------------------------------------------------*/
typedef struct
{
  /* Ĭ������� */
  uint32_t time;                // ��¼ʱ��
  ProcessEnum process;          // ����
  
  /* �ֶ������� */
  uint8_t tag;                  // ���
  BoolEnum isFullDuplex;        // �Ƿ�ȫ˫��
  BoolEnum isHDMaster;          // ��˫��-���� 
  TxQueueStruct *txQueue;       // ���ͻ���
  RxQueueStruct *rxQueue;       // ���ջ���
  uint16_t loopInterval;        // ��ѯʱ�䣨��˫����Ч��
  uint32_t *refTime;            // �ο�ʱ�䣬Ϊϵͳʱ��(sysTime)������ʵʱ��(realTime) ==����˫����Ч��
  
  void (*CallBack_TxFunc)(uint8_t*, uint16_t);                  // Ӳ�����ͺ���
}CommunicateStruct;

/* Public define ------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void Communicate_Handle(CommunicateStruct *communicate);

#endif 