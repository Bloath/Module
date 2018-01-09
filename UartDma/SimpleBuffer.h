#ifndef _SIMPLE_BUFFER_H_
#define _SIMPLE_BUFFER_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"


#define BUFFER_LENGTH  600

//#define DYNAMIC_MEMORY  1       //�Ƿ�ʹ�� ��̬�����ڴ棬 ʹ�ú�ᵼ�´���������

#ifndef DYNAMIC_MEMORY
  #define STATIC_BUFFER_LEN 32  //��ʹ�ö�̬�ڴ������£���̬�ڴ�Ĵ�С
#endif


#define RX_BLOCK_COUNT 5
#define TX_BLOCK_COUNT 5

#define RX_FLAG_USED    (1<<0)          //��ռ��

#define TX_FLAG_USED    (1<<0)          //��ռ��
#define TX_FLAG_SENDED  (1<<1)          //�ѷ���
#define TX_FLAG_RT      (1<<2)          //��Ҫ�������·���
#define TX_FLAG_TIMEOUT (1<<3)          // 
#define TX_FLAG_MC      (1<<7)          //�ֶ����

#define TX_TIME_OUT     3000             //��ʱʱ��

#include "stm32f0xx_hal.h"
#define sysTime HAL_GetTick()


/* Public typedef ------------------------------------------------------------*/
typedef struct
{
  uint8_t buffer[BUFFER_LENGTH];
  uint16_t count;
}RxBufferTypeDef;       //���ջ��壬����N�ֽڵĻ�����Լ�������

typedef struct
{
  uint8_t flag;
  
#ifdef DYNAMIC_MEMORY
  uint8_t *message;
#else
  uint8_t message[STATIC_BUFFER_LEN];
#endif
  
  uint16_t length;
}RxBlockTypeDef;        //���տ�


typedef struct
{
#ifdef DYNAMIC_MEMORY
  uint8_t *message;
#else
  uint8_t message[STATIC_BUFFER_LEN];
#endif
  
    uint16_t length;
    uint16_t flag;
    uint16_t retransCounter;
    uint32_t time;
}TxBlockTypeDef;        // ���Ϳ�     


typedef struct
{
  RxBufferTypeDef rxBuffer;
  RxBlockTypeDef rxBlockList[RX_BLOCK_COUNT];
}RxBufferStruct;

/* Public define -------------------------------------------------------------*/

typedef enum
{
  TxBlockError_TimeOut = 0
}TxBlockError;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/

/* ������Ҫ�ĺ��� */
void ReceiveSingleByte(uint8_t rxByte, RxBufferTypeDef *rxBuffer);                   //���յ��ֽ����ݣ������������
void FillRxBlock( RxBlockTypeDef *rxBlock, uint8_t *packet, uint16_t Len);           //�����ջ����е�������䵽���ձ��Ķ�����
void RxBlockListHandle(RxBlockTypeDef *rxBlock, void (*f)(uint8_t*, uint16_t));      //���ձ��Ķ��д���


/* ������Ҫ�ĺ��� */
uint8_t FillTxBlock(TxBlockTypeDef *txBlock, uint8_t *message, uint16_t length, uint8_t ManualClear);           //��䷢�Ͷ��У���������ط��Լ�δʹ�ñ�־λΪ1
void TxBlockListHandle(TxBlockTypeDef *txBlock, void (*Transmit)(uint8_t*, uint16_t), uint32_t timeout);
void FreeTxBlock(TxBlockTypeDef *txBlock);                                                              //�ͷŷ��Ϳ飬�ͷ��ڴ������־λ
void ClearSpecifyBlock(TxBlockTypeDef *txBlock, uint8_t (*func)(uint8_t*, uint16_t, void*), void *p);   //ͨ��ָ�����������

/* С���� */
void CopyPacket(uint8_t *srcPacket, uint8_t *desPacket, uint16_t length);               //�������ݰ�
uint8_t isPacketSame(uint8_t *srcPacket, uint8_t *desPacket, uint16_t length);

#endif

