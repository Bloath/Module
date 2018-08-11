#ifndef _COMMUNICATE_API_H_
#define _COMMUNICATE_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "../BufferQueue/BufferQueue.h"
/* Public typedef -----------------------------------------------------------*/
typedef enum
{
  COM_NET = 0,
  COM_485,
  COM_24G,
  COM_NB
}CommunicateMediumEnum;

typedef struct
{
  /* Ĭ������� */
  uint32_t time;                // ��¼ʱ��
  ProcessEnum process;          // ����
  
  /* �ֶ������� */
  CommunicateMediumEnum medium; // ͨѶ����
  BoolEnum isFullDuplex;        // �Ƿ�ȫ˫��
  BoolEnum isHDMaster;          // ��˫��-���� 
  TxQueueStruct *txQueue;       // ���ͻ��壬ҵ��� �ڽ���ҵ����󣬽���Ҫ�����ҵ��ֱ����䵽�û�����(����ѭ��)
  RxQueueStruct *rxQueue;       // ���ջ��壬ҵ��� RxQueue_Handle(communicate->rxQueue, Communicate_RxHandle, (void*)communicate); 
  uint16_t loopInterval;        // ��ѯʱ�䣨��˫����Ч��
  uint32_t *refTime;            // �ο�ʱ�䣬Ϊϵͳʱ��(sysTime)������ʵʱ��(realTime) ==����˫����Ч��
  
  BoolEnum (*CallBack_TxFunc)(uint8_t*, uint16_t);                  // ҵ��㷢�ʹ�����
}CommunicateStruct;

/* Public define ------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void Communicate_Handle(CommunicateStruct *communicate);

#endif 