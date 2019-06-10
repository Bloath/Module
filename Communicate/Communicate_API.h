#ifndef _COMMUNICATE_API_H_
#define _COMMUNICATE_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Module_Conf.h"
#include "../DataStruct/BufferQueue.h"

/* Public typedef -----------------------------------------------------------*/
typedef enum
{
    COM_NET = 0,
    COM_485,
    COM_24G,
    COM_NBIOT
} CommunicateMediumEnum;

typedef struct __communicate
{
    /* 默认项参数 */
    uint32_t __time;                  // 记录时间
    ProcessEnum __process;            // 流程

    /* 手动设置项 */
    CommunicateMediumEnum medium; // 通讯介质
    uint8_t attribute;            // 属性，默认为全双工
    uint8_t flag;                 // 状态
    TxQueueStruct *txQueue;       // 发送缓冲，业务层 在进行业务处理后，将需要处理的业务直接填充到该缓冲中(不断循环)
    RxQueueStruct *rxQueue;       // 接收缓冲，业务层 RxQueue_Handle(communicate->rxQueue, Communicate_RxHandle, (void*)communicate);
    uint16_t loopInterval;        // 轮询时间（半双工客户端有效，COM_ATTR_POLL_MC无效）
    uint32_t *refTime;            // 参考时间，为系统时间(sysTime)还是真实时间(realTime) ==（半双工客户端有效，COM_ATTR_POLL_MC无效）

    /* 回调 */
    bool (*CallBack_TxFunc)(uint8_t *, uint16_t);                   // 业务层发送处理函数
    void *txFuncParam;                                              // 发送函数回调参数（用于封包）
    void (*CallBack_RxHandleFunc)(uint8_t *, uint16_t, void*);      // 业务层接收处理函数， param为communicate本身
    void (*CallBack_FillHoldMsg)(struct __communicate *);           // 填充空闲报文函数，半双工客户端使用，其他无视
} CommunicateStruct;

/* Public define ------------------------------------------------------------*/
#define COM_ATTR_FD         (1<<0)      // 全双工
#define COM_ATTR_HD_CLINET  (1<<1)      // 半双工客户端，默认为半双工服务器
#define COM_ATTR_POLL_MNL   (1<<2)      // 手动处理轮询（only for 半双工客户端）

#define COM_FLAG_IS_LOCK    (1<<3)      // 已锁定

#define Communicate_IsLock(com)     (((com)->flag & COM_FLAG_IS_LOCK) != 0)

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void Communicate_Handle(CommunicateStruct *communicate);
void Communicate_Lock(CommunicateStruct *communicate);

#endif
