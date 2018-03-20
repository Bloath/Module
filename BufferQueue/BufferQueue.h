#ifndef _SIMPLE_BUFFER_H_
#define _SIMPLE_BUFFER_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "BufferQueue_Conf.h"

/* Public define -------------------------------------------------------------*/

#define RX_FLAG_USED    (1<<0)          // ��ռ��

#define TX_FLAG_USED    (1<<0)          // ��ռ��
#define TX_FLAG_SENDED  (1<<1)          // �ѷ���
#define TX_FLAG_RT      (1<<2)          // ��Ҫ�������·���
#define TX_FLAG_TIMEOUT (1<<3)          // �Ƿ�ʱ��־λ
#define TX_FLAG_MC      (1<<7)          // �ֶ����


/* Public typedef ------------------------------------------------------------*/
typedef enum
{
  TX_ONCE_AC = 0,                               // ��һ�Σ��Զ����
  TX_ONCE_MC = TX_FLAG_MC,                      // ��һ�Σ��ֶ����
  TX_MULTI_AC = TX_FLAG_RT,                     // ����Σ��Զ�������ط�������TxQueueStruct.maxCount
  TX_MULTI_MC = TX_FLAG_RT | TX_FLAG_MC,        // ����Σ��ֶ����
}TxModeEnum;

typedef struct
{
  uint8_t buffer[BUFFER_LENGTH];
  uint16_t count;
}RxBufferStruct;       //���ջ��壬����N�ֽڵĻ�����Լ�������

//*************************�����鵥λ*******************
typedef struct
{
  uint8_t flag;
  uint8_t *message;
  uint16_t length;
}RxBaseBlockStruct;        //���տ�

typedef struct
{
  uint8_t *message;
  uint16_t length;
  uint16_t flag;
  uint16_t retransCounter;
  TX_ID_SIZE id;
#ifdef TX_BLOCK_TIMEOUT
    uint32_t time;
#endif
}TxBaseBlockStruct;        // ���Ϳ�     

//***********************�߼��黺�嵥λ************************
typedef struct
{
  uint32_t time;                                // �Բ�ͬ�ķ��ͺ�����˵�����Ų�ͬ�ķ��ͼ������Ҫ���е�������
  uint16_t usedBlockQuantity;                   // ��ʹ�õĿ�����
  uint16_t maxTxCount;                          // ����ط�����
  uint16_t interval;                            // ���ͼ��
  uint16_t indexCache;                          // �������棬���������ʹ��
  BoolEnum isTxUnordered;                       // �Ƿ�������
  TxBaseBlockStruct txBlocks[BLOCK_COUNT];
}TxQueueStruct;             // ���Ϳ黺�����

typedef struct
{
  uint32_t time; 
  uint16_t usedBlockQuantity;
  RxBaseBlockStruct rxBlocks[BLOCK_COUNT];
}RxQueueStruct;             //���տ黺�����

//*********************����ö��**************************
typedef enum
{
  TxBlockError_TimeOut = 0
}TxBlockError;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/

/* ������Ҫ�ĺ��� */
void ReceiveSingleByte(uint8_t rxByte, RxBufferStruct *rxBuffer);                                                       //���յ��ֽ����ݣ������������
uint16_t RxQueue_Add(RxQueueStruct *rxQueue, uint8_t *packet, uint16_t Len);                                            //�����ջ����е�������䵽���ձ��Ķ�����
void RxQueue_Handle(RxQueueStruct *rxQueue, void (*RxPacketHandle)(uint8_t*, uint16_t));                                //���ձ��Ķ��д���

/* ������Ҫ�ĺ��� */
uint16_t TxQueue_Add(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, TxModeEnum mode);                       //��䷢�Ͷ��У���������ط��Լ�δʹ�ñ�־λΪ1
uint16_t TxQueue_AddWithId(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, TxModeEnum mode, TX_ID_SIZE id);
void TxQueue_Handle(TxQueueStruct *txQueue, void (*Transmit)(uint8_t*, uint16_t));                                      //���ͱ��Ķ��д���

void TxQueue_FreeByFunc(TxQueueStruct *txQueue, BoolEnum (*func)(uint8_t*, uint16_t, void*), void *para);               //ͨ��ָ���������ͷ�ָ�����Ϳ�
void TxQueue_FreeById(TxQueueStruct *txQueue,  TX_ID_SIZE id);                                                          //ͨ��ID���ͷ�ָ�����Ϳ�

#endif

