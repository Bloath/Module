#ifndef _SIMPLE_BUFFER_H_
#define _SIMPLE_BUFFER_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "SimpleBuffer_Conf.h"

/* Public define -------------------------------------------------------------*/

#define RX_FLAG_USED    (1<<0)          //��ռ��

#define TX_FLAG_USED    (1<<0)          //��ռ��
#define TX_FLAG_SENDED  (1<<1)          //�ѷ���
#define TX_FLAG_RT      (1<<2)          //��Ҫ�������·���
#define TX_FLAG_TIMEOUT (1<<3)          // 
#define TX_FLAG_MC      (1<<7)          //�ֶ����

/* Public typedef ------------------------------------------------------------*/
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
  uint32_t time;                                //�Բ�ͬ�ķ��ͺ�����˵�����Ų�ͬ�ķ��ͼ������Ҫ���е�������
  TxBaseBlockStruct txBlocks[BLOCK_COUNT];
}TxQueueStruct;             // ���Ϳ黺�����

typedef struct
{
  uint32_t time;        
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
void ReceiveSingleByte(uint8_t rxByte, RxBufferStruct *rxBuffer);                                      //���յ��ֽ����ݣ������������
uint16_t RxQueue_Add(RxQueueStruct *rxQueue, uint8_t *packet, uint16_t Len);                          //�����ջ����е�������䵽���ձ��Ķ�����
void RxQueue_Handle(RxQueueStruct *rxQueue, void (*f)(uint8_t*, uint16_t));                         //���ձ��Ķ��д���

/* ������Ҫ�ĺ��� */
uint16_t TxQueue_Add(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t custom);               //��䷢�Ͷ��У���������ط��Լ�δʹ�ñ�־λΪ1
uint16_t TxQueue_AddWithId(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t custom, TX_ID_SIZE id);
void TxQueue_Handle(TxQueueStruct *txQueue, void (*Transmit)(uint8_t*, uint16_t), uint32_t timeout);        //���ͱ��Ķ��д���
void TxQueue_FreeBlock(TxBaseBlockStruct *txBlock);                                                                      //�ͷŷ��Ϳ飬�ͷ��ڴ������־λ
void TxQueue_FreeByFunc(TxQueueStruct *txQueue, uint8_t (*func)(uint8_t*, uint16_t, void*), void *p);           //ͨ��ָ���������ͷ�ָ�����Ϳ�
void TxQueue_FreeById(TxQueueStruct *txQueue,  TX_ID_SIZE id);                                                  //ͨ��ID���ͷ�ָ�����Ϳ�

/* С���� */
uint8_t isPacketSame(uint8_t *srcPacket, uint8_t *desPacket, uint16_t length);

#endif

