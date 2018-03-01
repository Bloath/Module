#ifndef _SIMPLE_BUFFER_H_
#define _SIMPLE_BUFFER_H_

/* Includes ------------------------------------------------------------------*/
#include "../Sys_Conf.h"
#include "SimpleBuffer_Conf.h"

/* Public define -------------------------------------------------------------*/

#define RX_FLAG_USED    (1<<0)          //被占用

#define TX_FLAG_USED    (1<<0)          //被占用
#define TX_FLAG_SENDED  (1<<1)          //已发送
#define TX_FLAG_RT      (1<<2)          //需要进行重新发送
#define TX_FLAG_TIMEOUT (1<<3)          // 
#define TX_FLAG_MC      (1<<7)          //手动清除

/* Public typedef ------------------------------------------------------------*/
typedef struct
{
  uint8_t buffer[BUFFER_LENGTH];
  uint16_t count;
}RxBufferStruct;       //接收缓冲，包含N字节的缓冲池以及计数器

//*************************基础块单位*******************
typedef struct
{
  uint8_t flag;
  uint8_t *message;
  uint16_t length;
}RxBaseBlockStruct;        //接收块

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
}TxBaseBlockStruct;        // 发送块     

//***********************高级块缓冲单位************************
typedef struct
{
  uint32_t time;                                //对不同的发送函数来说，有着不同的发送间隔，需要进行单独设置
  TxBaseBlockStruct txBlocks[BLOCK_COUNT];
}TxQueueStruct;             // 发送块缓冲队列

typedef struct
{
  uint32_t time;        
  RxBaseBlockStruct rxBlocks[BLOCK_COUNT];
}RxQueueStruct;             //接收块缓冲队列

//*********************错误枚举**************************
typedef enum
{
  TxBlockError_TimeOut = 0
}TxBlockError;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/

/* 接收需要的函数 */
void ReceiveSingleByte(uint8_t rxByte, RxBufferStruct *rxBuffer);                                      //接收单字节数据，填充至缓冲中
uint16_t RxQueue_Add(RxQueueStruct *rxQueue, uint8_t *packet, uint16_t Len);                          //将接收缓冲中的数据填充到接收报文队列中
void RxQueue_Handle(RxQueueStruct *rxQueue, void (*f)(uint8_t*, uint16_t));                         //接收报文队列处理

/* 发送需要的函数 */
uint16_t TxQueue_Add(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t custom);               //填充发送队列，包含清除重发以及未使用标志位为1
uint16_t TxQueue_AddWithId(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t custom, TX_ID_SIZE id);
void TxQueue_Handle(TxQueueStruct *txQueue, void (*Transmit)(uint8_t*, uint16_t), uint32_t timeout);        //发送报文队列处理
void TxQueue_FreeBlock(TxBaseBlockStruct *txBlock);                                                                      //释放发送块，释放内存清除标志位
void TxQueue_FreeByFunc(TxQueueStruct *txQueue, uint8_t (*func)(uint8_t*, uint16_t, void*), void *p);           //通过指定函数，释放指定发送块
void TxQueue_FreeById(TxQueueStruct *txQueue,  TX_ID_SIZE id);                                                  //通过ID，释放指定发送块

/* 小功能 */
uint8_t isPacketSame(uint8_t *srcPacket, uint8_t *desPacket, uint16_t length);

#endif

