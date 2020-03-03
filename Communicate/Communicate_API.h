#ifndef _COMMUNICATE_API_H_
#define _COMMUNICATE_API_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"
#include "Module/DataStruct/BufferQueue.h"

/* Public typedef -----------------------------------------------------------*/
enum CommunicateMediumEnum
{
    COM_NET = 0,
    COM_SERIAL,
    COM_485,
    COM_24G,
    COM_NBIOT
};

struct CommunicateHalfDuplexStruct
{
    uint32_t __time;                // 时间
    uint16_t loopInterval;          // 轮询时间（半双工客户端有效，COM_ATTR_POLL_MC无效）
    uint32_t *refTime;              // 参考时间，为系统时间(sysTime)还是真实时间(realTime) ==（半双工客户端有效，COM_ATTR_POLL_MC无效）
    struct ProcessStruct __process;          // 流程
    
    void *parent;                   // 父com结构体
    
    void (*CallBack_FillHoldMsg)(void *com);           // 填充空闲报文函数，半双工客户端使用，其他无视
};


struct CommunicateStruct
{
    /* 默认项参数 */
    enum CommunicateMediumEnum medium;          // 通讯介质
    uint8_t flag;                               // 标志位

    struct CommunicateHalfDuplexStruct *halfDuplex;             // 半双工
    struct TxQueueStruct *txQueue;                             // 发送缓冲，业务层 在进行业务处理后，将需要处理的业务直接填充到该缓冲中(不断循环)
    struct RxQueueStruct *rxQueue;                             // 接收缓冲，业务层 RxQueue_Handle(communicate->rxQueue, Communicate_RxHandle, (void*)communicate);
    
    /* 回调 */
    bool (*CallBack_TxFunc)(uint8_t *, uint16_t);                   // 业务层发送处理函数
    void *txFuncParam;                                              // 发送函数回调参数（用于封包）
    void (*CallBack_RxHandleFunc)(uint8_t *, uint16_t, void*);      // 业务层接收处理函数， param为communicate本身
};

/* Public define ------------------------------------------------------------*/
#define COM_FLAG_IS_LOCK    (1<<7)      // 已锁定

#define Communicate_IsLock(com)     (((com)->flag & COM_FLAG_IS_LOCK) != 0)

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void Communicate_Handle(struct CommunicateStruct *communicate);
void Communicate_Lock(struct CommunicateStruct *communicate);

#endif
