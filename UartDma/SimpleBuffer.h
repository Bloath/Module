#ifndef _SIMPLE_BUFFER_H_
#define _SIMPLE_BUFFER_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"


#define BUFFER_LENGTH  600

//#define DYNAMIC_MEMORY  1       //是否使用 动态申请内存， 使用后会导致代码量增加

#ifndef DYNAMIC_MEMORY
  #define STATIC_BUFFER_LEN 32  //不使用动态内存的情况下，静态内存的大小
#endif


#define RX_BLOCK_COUNT 5
#define TX_BLOCK_COUNT 5

#define RX_FLAG_USED    (1<<0)          //被占用

#define TX_FLAG_USED    (1<<0)          //被占用
#define TX_FLAG_SENDED  (1<<1)          //已发送
#define TX_FLAG_RT      (1<<2)          //需要进行重新发送
#define TX_FLAG_TIMEOUT (1<<3)          // 
#define TX_FLAG_MC      (1<<7)          //手动清除

#define TX_TIME_OUT     3000             //超时时间

#include "stm32f0xx_hal.h"
#define sysTime HAL_GetTick()


/* Public typedef ------------------------------------------------------------*/
typedef struct
{
  uint8_t buffer[BUFFER_LENGTH];
  uint16_t count;
}RxBufferTypeDef;       //接收缓冲，包含N字节的缓冲池以及计数器

typedef struct
{
  uint8_t flag;
  
#ifdef DYNAMIC_MEMORY
  uint8_t *message;
#else
  uint8_t message[STATIC_BUFFER_LEN];
#endif
  
  uint16_t length;
}RxBlockTypeDef;        //接收块


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
}TxBlockTypeDef;        // 发送块     


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

/* 接收需要的函数 */
void ReceiveSingleByte(uint8_t rxByte, RxBufferTypeDef *rxBuffer);                   //接收单字节数据，填充至缓冲中
void FillRxBlock( RxBlockTypeDef *rxBlock, uint8_t *packet, uint16_t Len);           //将接收缓冲中的数据填充到接收报文队列中
void RxBlockListHandle(RxBlockTypeDef *rxBlock, void (*f)(uint8_t*, uint16_t));      //接收报文队列处理


/* 发送需要的函数 */
uint8_t FillTxBlock(TxBlockTypeDef *txBlock, uint8_t *message, uint16_t length, uint8_t ManualClear);           //填充发送队列，包含清除重发以及未使用标志位为1
void TxBlockListHandle(TxBlockTypeDef *txBlock, void (*Transmit)(uint8_t*, uint16_t), uint32_t timeout);
void FreeTxBlock(TxBlockTypeDef *txBlock);                                                              //释放发送块，释放内存清除标志位
void ClearSpecifyBlock(TxBlockTypeDef *txBlock, uint8_t (*func)(uint8_t*, uint16_t, void*), void *p);   //通过指定函数，清除

/* 小功能 */
void CopyPacket(uint8_t *srcPacket, uint8_t *desPacket, uint16_t length);               //复制数据包
uint8_t isPacketSame(uint8_t *srcPacket, uint8_t *desPacket, uint16_t length);

#endif

