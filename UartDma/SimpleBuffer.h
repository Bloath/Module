#ifndef _SIMPLE_BUFFER_H_
#define _SIMPLE_BUFFER_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "SimpleBuffer_Conf.h"

/* Public define -------------------------------------------------------------*/


#define RX_BLOCK_COUNT BLOCK_COUNT
#define TX_BLOCK_COUNT BLOCK_COUNT

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
}RxBufferTypeDef;       //���ջ��壬����N�ֽڵĻ�����Լ�������

typedef struct
{
  uint8_t flag;
  uint8_t *message;
  uint16_t length;
}RxBlockTypeDef;        //���տ�


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
}TxBlockTypeDef;        // ���Ϳ�     


typedef struct
{
  RxBufferTypeDef rxBuffer;
  RxBlockTypeDef rxBlockList[RX_BLOCK_COUNT];
}RxBufferStruct;
typedef enum
{
  TxBlockError_TimeOut = 0
}TxBlockError;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/

/* ������Ҫ�ĺ��� */
void ReceiveSingleByte(uint8_t rxByte, RxBufferTypeDef *rxBuffer);                                      //���յ��ֽ����ݣ������������
uint16_t FillRxBlock( RxBlockTypeDef *rxBlock, uint8_t *packet, uint16_t Len);                          //�����ջ����е�������䵽���ձ��Ķ�����
void RxBlockListHandle(RxBlockTypeDef *rxBlock, void (*f)(uint8_t*, uint16_t));                         //���ձ��Ķ��д���

/* ������Ҫ�ĺ��� */
uint16_t FillTxBlock(TxBlockTypeDef *txBlock, uint8_t *message, uint16_t length, uint8_t custom);               //��䷢�Ͷ��У���������ط��Լ�δʹ�ñ�־λΪ1
uint16_t FillTxBlockWithId(TxBlockTypeDef *txBlock, uint8_t *message, uint16_t length, uint8_t custom, TX_ID_SIZE id);
void TxBlockListHandle(TxBlockTypeDef *txBlock, void (*Transmit)(uint8_t*, uint16_t), uint32_t timeout);        //���ͱ��Ķ��д���
void FreeTxBlock(TxBlockTypeDef *txBlock);                                                                      //�ͷŷ��Ϳ飬�ͷ��ڴ������־λ
void FreeTxBlockByFunc(TxBlockTypeDef *txBlock, uint8_t (*func)(uint8_t*, uint16_t, void*), void *p);           //ͨ��ָ���������ͷ�ָ�����Ϳ�
void FreeTxBlockById(TxBlockTypeDef *txBlock,  TX_ID_SIZE id);                                                  //ͨ��ID���ͷ�ָ�����Ϳ�

/* С���� */
uint8_t isPacketSame(uint8_t *srcPacket, uint8_t *desPacket, uint16_t length);

#endif

