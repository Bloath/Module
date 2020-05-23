#ifndef _BUFFERQUEUE_H_
#define _BUFFERQUEUE_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"
#include "BufferQueue_Conf.h"

/* Public define -------------------------------------------------------------*/

#define RX_FLAG_USED        (1 << 0) // 被占用

#define TX_FLAG_USED        (1 << 0)        // 被占用
#define TX_FLAG_SENDED      (1 << 1)        // 已发送
#define TX_FLAG_RT          (1 << 2)        // 需要进行重新发送
#define TX_FLAG_TIMEOUT     (1 << 3)        // 是否超时标志位
#define TX_FLAG_PACKAGE     (1 << 4)        // 需要打包
#define TX_FLAG_IN_BATCHES  (1 << 5)
#define TX_FLAG_MC          (1 << 6)        // 手动清除
#define TX_FLAG_IS_MALLOC   (1 << 7)        // 已经为MALLOC，不需要复制，使用该位时，如果为字符串，则长度要包含结束位

#define TX_ONCE_AC  0                           // 发一次，自动清除
#define TX_ONCE_MC  TX_FLAG_MC                  // 发一次，手动清除
#define TX_MULTI_AC TX_FLAG_RT                  // 发多次，自动清除，重发次数在TxQueueStruct.maxCount
#define TX_MULTI_MC (TX_FLAG_RT | TX_FLAG_MC)   // 发多次，手动清除

/* Public typedef ------------------------------------------------------------*/

//*********************枚举**************************

/* 块清除方式枚举 */
enum BlockFreeMethodEnum
{
    BlockFree_OverTime = 0,
    BlockFree_OverRetransmit,
    BlockFree_Manual,
    BlockFree_TransmitReturn,
    BlockFree_OnceAuto, // 发送一次自动清除
};

struct RxBufferStruct
{
    uint8_t *_buffer;
    uint16_t length;
    uint16_t count;
} ; //接收缓冲，包含N字节的缓冲池以及计数器

//*************************基础块单位*******************
struct PacketStruct
{
    uint8_t *data;
    uint16_t length;
};

struct RxBaseBlockStruct
{
    uint8_t flag;
    uint8_t *message;
    uint16_t length;
#ifdef BLOCK_WITH_PARAM
    void *param;
#endif
}; //接收块

struct TxBaseBlockStruct
{
    uint8_t *message;
    uint16_t length;
    uint16_t flag;
    uint16_t retransCounter;
    uint16_t currentIndex;
    TX_ID_SIZE id;
    uint8_t seqId;
#ifdef BLOCK_WITH_PARAM
    void *param;
#endif
#ifdef TX_BLOCK_TIMEOUT
    uint32_t time;
#endif
}; // 发送块

//***********************高级块缓冲单位************************
struct TxQueueStruct
{
    struct TxBaseBlockStruct __txBlocks[BLOCK_COUNT];       // 数据块
    uint32_t __time;                                        // 对不同的发送函数来说，有着不同的发送间隔，需要进行单独设置
    uint8_t seqId;                                          // 序列ID，为有序发送准备
    
    uint8_t _usedBlockQuantity;                     // 已使用的块数量
    uint8_t _lastIndex;                             // 上一次发送的索引
    bool isTxUnordered;                             // 是否无序发送
    uint16_t maxTxCount;                            // 最大重发次数
    uint16_t interval;                              // 发送间隔
    
    bool (*CallBack_Transmit)(uint8_t *, uint16_t);                                             // 发送函数
    bool (*CallBack_TransmitUseBlock)(struct TxBaseBlockStruct *txBlock);                       // 通过块发送
    void (*CallBack_AutoClearBlock)(struct TxBaseBlockStruct *, enum BlockFreeMethodEnum );     // 自动清除回调
    int (*CallBack_PackagBeforeTransmit)(struct TxBaseBlockStruct *, struct PacketStruct *);    // 在发送前的组包处理，返回组完的包，需要通过Free释放
};// 发送块缓冲队列

struct RxQueueStruct
{
    struct RxBaseBlockStruct __rxBlocks[BLOCK_COUNT];
    uint32_t __time;
    uint16_t _usedBlockQuantity;  
    
    void (*CallBack_RxPacketHandle)(struct RxBaseBlockStruct *);                // 接收数据包处理
    bool (*CallBack_BeforeFree)(struct RxBaseBlockStruct *);                    // 在清除之前的回调，防止某些debug应用多次申请空间复制。  
}; //接收块缓冲队列

struct DmaBufferStruct
{
    struct RxQueueStruct _rxQueue;              // 发送缓冲队列
    uint8_t *_buffer;                           // DMA环形缓冲池，指针
    uint16_t __bufferLength;                    // 缓冲池大小
    uint16_t __start;                           // 头位置标记
    uint16_t __end;                             // 尾位置标记
    
    void (*CallBack_MallocFail)();              // 接收数据过长，缓冲没地方
};

/* Public macro --------------------------------------------------------------*/
#define  STM32_DMA_INIT(uart, buffer)                                   \
        {                                                               \
            HAL_UART_Receive_DMA(&uart, buffer, sizeof(buffer));        \
            SET_BIT(uart.Instance->CR1, USART_CR1_IDLEIE);              \
        }
        
#define STM32_RX_HANDLE(uart, dma)                                      \
        {                                                               \
            DmaBuffer_IdleHandle(&dma, uart.hdmarx->Instance->CNDTR);   \
            __HAL_UART_CLEAR_IT(&uart, UART_CLEAR_IDLEF);               \
            __HAL_UART_CLEAR_IT(&uart, UART_CLEAR_FEF);                 \
            __HAL_UART_CLEAR_IT(&uart, UART_CLEAR_NEF);                 \
            return;                                                     \
        }
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/

/* 接收需要的函数 */
int ReceiveSingleByte(uint8_t rxByte, struct RxBufferStruct *rxBuffer);                                             // 接收单字节数据，填充至缓冲中
int RxQueue_Add(struct RxQueueStruct *rxQueue, uint8_t *packet, uint16_t Len, bool isMalloc);                       // 将接收缓冲中的数据填充到接收报文队列中
void RxQueue_Handle(struct RxQueueStruct *rxQueue);                                                                 // 接收报文队列处理

/* 发送需要的函数 */
int TxQueue_Add(struct TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t mode);                        // 填充发送队列，包含清除重发以及未使用标志位为1
int TxQueue_AddWithId(struct TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t mode, TX_ID_SIZE id);   // 填充发送队列，通过Id对发送快进行标记
int TxQueue_Handle(struct TxQueueStruct *txQueue);                                                                      // 发送报文队列处理
void TxQueue_TimeSync(struct TxQueueStruct *txQueue, uint32_t time);

void TxQueue_FreeByFunc(struct TxQueueStruct *txQueue, bool (*func)(struct TxBaseBlockStruct*, void *), void *para);  // 通过指定函数，释放指定发送块
void TxQueue_FreeById(struct TxQueueStruct *txQueue, TX_ID_SIZE id);                                           // 通过ID，释放指定发送块
void TxQueue_FreeByIndex(struct TxQueueStruct *txQueue, uint8_t index);                                        // 通过Index，释放制定发送快
void TxQueue_FreeAll(struct TxQueueStruct *txQueue);
void TxQueue_FreeBatch(struct TxQueueStruct *txQueue);

void DmaBuffer_Init(struct DmaBufferStruct *dmaBuffer, uint8_t *buffer, uint16_t bufferSize);                  // 
void DmaBuffer_IdleHandle(struct DmaBufferStruct *dmaBuffer, uint16_t remainCount);
#endif
