#ifndef _NB_API_H_
#define _NB_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Module_Conf.h"
#include "../DataStruct/DataStruct.h"

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
typedef enum
{
    NbSignal_Undetected,
    NbSignal_Weak,
    NbSignal_Normal,
} NbSignalStrengthEnum;

typedef enum
{
    NbError_WeakSignal = 0,
    NbError_Undetected,
} NbErrorEnum;

typedef struct
{
    uint8_t index;
    bool isGetOk;
    bool isGetError;
} OrderAtCmdStruct;

typedef struct
{
    uint32_t __time;                                    // 内部时间
    uint32_t __startConnectTime;                        // 开始连接时间
    OrderAtCmdStruct __orderAt;                         // AT指令索引

    ProcessEnum _process;                               // 运行流程
    uint8_t _errorCounter;                              // 在发送状态下产生的错误次数
    bool _isTransmitting;                               // 正在发送
    NbSignalStrengthEnum _signal;                       // 信号强度

    TxQueueStruct *txQueueHal;                          // 硬件层 发送队列
    TxQueueStruct *txQueueService;                      // 业务层 发送队列
    RxQueueStruct *rxQueueService;                      // 业务层 接收队列，拆包后填充至该队列
    
    void (*CallBack_TxError)();                         // 错误处理
    bool (*CallBack_HalTxFunc)(uint8_t *, uint16_t);    // 硬件层 发送处理函数
    void *halTxParam;                                   // 硬件层 发送处理函数参数 
    void (*CallBack_HalRxGetMsg)(uint8_t *, uint16_t, void *param);     // 硬件层 接收到底层发来的数据，在解包处理之前的回调
} NBStruct;

/* Public variables ----------------------------------------------------------*/
extern NBStruct nb;

/* Public function prototypes ------------------------------------------------*/
void NB_Handle();
void NB_RxHandle(uint8_t *packet, uint16_t len, void *param);

void NB_StringTrans(const char *string);
#endif