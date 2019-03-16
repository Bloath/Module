#ifndef _BUFFERQUEUE_H_
#define _BUFFERQUEUE_H_

/* Includes ------------------------------------------------------------------*/
#include "../Module_Conf.h"
#include "BufferQueue_Conf.h"

/* Public define -------------------------------------------------------------*/

#define RX_FLAG_USED        (1 << 0) // 被占用

#define TX_FLAG_USED        (1 << 0)        // 被占用
#define TX_FLAG_SENDED      (1 << 1)        // 已发送
#define TX_FLAG_RT          (1 << 2)        // 需要进行重新发送
#define TX_FLAG_TIMEOUT     (1 << 3)        // 是否超时标志位
#define TX_FLAG_PACKAGE     (1 << 4)        // 需要打包
#define TX_FLAG_MC          (1 << 6)        // 手动清除
#define TX_FLAG_IS_MALLOC   (1 << 7)        // 已经为MALLOC，不需要复制，使用该位时，如果为字符串，则长度要包含结束位

#define TX_ONCE_AC  0                           // 发一次，自动清除
#define TX_ONCE_MC  TX_FLAG_MC                  // 发一次，手动清除
#define TX_MULTI_AC TX_FLAG_RT                  // 发多次，自动清除，重发次数在TxQueueStruct.maxCount
#define TX_MULTI_MC (TX_FLAG_RT | TX_FLAG_MC)   // 发多次，手动清除

/* Public typedef ------------------------------------------------------------*/

//*********************枚举**************************

/* 块清除方式枚举 */
typedef enum
{
    BlockFree_OverTime = 0,
    BlockFree_OverRetransmit,
    BlockFree_Manual,
    BlockFree_TransmitReturn,
    BlockFree_OnceAuto, // 发送一次自动清除
} BlockFreeMethodEnum;

typedef struct
{
    uint8_t _buffer[BUFFER_LENGTH];
    uint16_t count;
} RxBufferStruct; //接收缓冲，包含N字节的缓冲池以及计数器

//*************************基础块单位*******************
typedef struct
{
    uint8_t *data;
    uint8_t length;
} PacketStruct;

typedef struct
{
    uint8_t flag;
    uint8_t *message;
    uint16_t length;
} RxBaseBlockStruct; //接收块

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
} TxBaseBlockStruct; // 发送块

//***********************高级块缓冲单位************************
typedef struct
{
    TxBaseBlockStruct __txBlocks[BLOCK_COUNT];      // 数据块
    uint32_t __time;                                // 对不同的发送函数来说，有着不同的发送间隔，需要进行单独设置
    uint8_t __indexCache;                           // 索引缓存，配合无序发送使用
    
    uint8_t _usedBlockQuantity;                     // 已使用的块数量
    uint8_t _lastIndex;                             // 上一次发送的索引
    bool isTxUnordered;                             // 是否无序发送
    uint16_t maxTxCount;                            // 最大重发次数
    uint16_t interval;                              // 发送间隔
    
    void (*CallBack_AutoClearBlock)(TxBaseBlockStruct *, BlockFreeMethodEnum );             // 自动清除回调
    int (*CallBack_PackagBeforeTransmit)(TxBaseBlockStruct *, void *, PacketStruct *);      // 在发送前的组包处理，返回组完的包，需要通过Free释放
} TxQueueStruct;                                                                            // 发送块缓冲队列

typedef struct
{
    RxBaseBlockStruct __rxBlocks[BLOCK_COUNT];
    uint32_t __time;
    uint16_t _usedBlockQuantity;        
    
    bool (*CallBack_BeforeFree)(uint8_t*, uint16_t);           // 在清除之前的回调，防止某些debug应用多次申请空间复制。
} RxQueueStruct; //接收块缓冲队列

typedef struct
{
    RxQueueStruct _rxQueue;                     // 发送缓冲队列
    uint8_t _buffer[BUFFER_LENGTH];             // DMA环形缓冲池
    uint16_t __bufferLength;                    // 缓冲池大小
    uint16_t __start;                           // 头位置标记
    uint16_t __end;                             // 尾位置标记
} DmaBufferStruct;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/

/* 接收需要的函数 */
int ReceiveSingleByte(uint8_t rxByte, RxBufferStruct *rxBuffer);                                              // 接收单字节数据，填充至缓冲中
int RxQueue_Add(RxQueueStruct *rxQueue, uint8_t *packet, uint16_t Len, bool isMalloc);                        // 将接收缓冲中的数据填充到接收报文队列中
void RxQueue_Handle(RxQueueStruct *rxQueue, void (*RxPacketHandle)(uint8_t *, uint16_t, void *), void *para); // 接收报文队列处理

/* 发送需要的函数 */
int TxQueue_Add(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t mode);                      // 填充发送队列，包含清除重发以及未使用标志位为1
int TxQueue_AddWithId(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t mode, TX_ID_SIZE id); // 填充发送队列，通过Id对发送快进行标记
void TxQueue_Handle(TxQueueStruct *txQueue, bool (*Transmit)(uint8_t *, uint16_t), void *packageParam);           // 发送报文队列处理

void TxQueue_FreeByFunc(TxQueueStruct *txQueue, bool (*func)(TxBaseBlockStruct*, void *), void *para);  // 通过指定函数，释放指定发送块
void TxQueue_FreeById(TxQueueStruct *txQueue, TX_ID_SIZE id);                                           // 通过ID，释放指定发送块
void TxQueue_FreeByIndex(TxQueueStruct *txQueue, uint8_t index);                                        // 通过Index，释放制定发送快

void DmaBuffer_IdleHandle(DmaBufferStruct *dmaBuffer, uint16_t remainCount);
#endif
