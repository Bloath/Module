#ifndef _NB_API_H_
#define _NB_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Module.h"
#include "../DataStruct/DataStruct.h"

/* Public macro --------------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/
typedef enum
{
    Unknown = 0,
    ChinaMobile,
    ChinaTelecom,
} SimTypeEnum;

typedef enum
{
    NbError_WeakSignal = 0,
    NbError_ReadSimError,           // 读取SIM卡错误，等待下次连接
    NbError_AttTimeout,             // 网络附着超时，等待下次连接
    NBError_ConnectError,           // 连接错误，例如打开HTTP/TCP/UDP
    NbError_TxFail,
    NbError_NeedReset,
    NbError_AtError,
    NbError_PowerDown
} NbErrorEnum;

typedef struct
{
    uint8_t index;
    bool isGetOk;
    bool isGetError;
    uint8_t errorCounter;
} OrderAtCmdStruct;

typedef struct
{
    uint32_t __time;                                    // 内部时间
    uint32_t __startConnectTime;                        // 开始连接时间
    OrderAtCmdStruct __orderAt;                         // AT指令索引

    ProcessEnum _process;                               // 运行流程
    ProcessEnum _lastProcess;                           // 上次流程
    uint8_t _errorCounter;                              // 在发送状态下产生的错误次数
    bool _isTransmitting;                               // 正在发送
    bool isSleep;                                       // 是否休眠
    uint8_t socketId;                                   // 套接字编号
    uint8_t _signal;                                    // 信号强度
    SimTypeEnum sim;
    char **cmdList;
    char *host;
    char *port;

    TxQueueStruct *txQueueHal;                          // 硬件层 发送队列
    RxQueueStruct *rxQueueHal;                          // 硬件层 接收队列
    TxQueueStruct *txQueueApp;                          // 业务层 发送队列
    RxQueueStruct *rxQueueApp;                          // 业务层 接收队列，拆包后填充至该队列
    
    
    void (*CallBack_HandleBeforeNetting)();             // 联网之前的处理
    void (*CallBack_StartConenct)();                    // 启动连接
    void (*CallBack_MessagePackage)(TxBaseBlockStruct *block, void *param, PacketStruct *packet);    // 数据打包
    void (*CallBack_ReceiveHandle)(char *message, uint16_t len);     // 接收处理部分
    void (*CallBack_TxError)(NbErrorEnum);              // 错误处理
    bool (*CallBack_HalTxFunc)(uint8_t *, uint16_t);    // 硬件层 发送处理函数
    void (*CallBack_TimeUpdate)();                      // 硬件层 获取新时间
    void *halTxParam;                                   // 硬件层 发送处理函数参数 
} NBStruct;

/* Public variables ----------------------------------------------------------*/
extern NBStruct nb;
/* Public function prototypes ------------------------------------------------*/
void NB_Handle();
bool NB_IsIdle();
void NB_RxHandle(uint8_t *packet, uint16_t len, void *param);
int NB_DataPackage(TxBaseBlockStruct *block, void *param, PacketStruct *packet);       //需要处理Malloc
void NB_StringTrans(const char *string);
void NB_ErrorHandle(NbErrorEnum error);
void NB_PowerOn();
void NB_SetProcess(ProcessEnum process);
void NB_SendATCommandList(char **list);
#endif
