#ifndef _NB_API_H_
#define _NB_API_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"
#include "Module/DataStruct/DataStruct.h"

/* Public macro --------------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/
enum SimTypeEnum
{
    Unknown = 0,
    ChinaMobile,
    ChinaTelecom,
};

enum NbErrorEnum
{
    NbError_WeakSignal = 0,
    NbError_ReadSimError,           // 读取SIM卡错误，等待下次连接
    NbError_AttTimeout,             // 网络附着超时，等待下次连接
    NBError_ConnectError,           // 连接错误，例如打开HTTP/TCP/UDP
    NbError_NoRegister,
    NbError_TxFail,
    NbError_NeedReset,
    NbError_AtError,
    NbError_PowerDown
};

struct OrderAtCmdStruct
{
    uint8_t index;
    bool isGetOk;
    bool isGetError;
    uint8_t errorCounter;
};

struct NBHttpStruct
{
    struct ProcessStruct __process;
    char *url;
    char *header;
};

struct NBStruct
{
    uint32_t __time;                                    // 内部时间
    uint32_t __startConnectTime;                        // 开始连接时间
    struct OrderAtCmdStruct __orderAt;                  // AT指令索引

    struct ProcessStruct _process;                      // 流程
    uint8_t _errorCounter;                              // 在发送状态下产生的错误次数
    bool _isTransmitting;                               // 正在发送
    bool isSleep;                                       // 是否休眠
    uint8_t socketId;                                   // 套接字编号
    uint8_t _signal;                                    // 信号强度
    int _lastId;                                         // 上次发送成功的ID
    enum SimTypeEnum sim;
    char **cmdList;
    char *host;
    char *port;
    
    struct NBHttpStruct http;

    struct TxQueueStruct *txQueueHal;                          // 硬件层 发送队列
    struct RxQueueStruct *rxQueueHal;                          // 硬件层 接收队列
    struct TxQueueStruct *txQueueApp;                          // 业务层 发送队列
    struct RxQueueStruct *rxQueueApp;                          // 业务层 接收队列，拆包后填充至该队列
    
    void (*CallBack_HandleBeforeNetting)();                         // 联网之前的处理
    void (*CallBack_StartConenct)();                                // 启动连接
    void (*CallBack_RxSecondaryHandle)(char *message, uint16_t len);// 接收处理部分
    void (*CallBack_TxError)(enum NbErrorEnum);                     // 错误处理
    void (*CallBack_TimeUpdate)();                                  // 硬件层 获取新时间
};

/* Public variables ----------------------------------------------------------*/
extern struct NBStruct nb;
/* Public function prototypes ------------------------------------------------*/
void NB_Handle();
bool NB_IsIdle();
void NB_RxHandle(struct RxBaseBlockStruct *rxBlock);
int NB_DataPackage(struct TxBaseBlockStruct *block, void *param, struct PacketStruct *packet);       //需要处理Malloc
void NB_StringTrans(const char *string);
void NB_ErrorHandle(enum NbErrorEnum error);
void NB_PowerOn();
void NB_SetProcess(enum ProcessEnum process);
void NB_SendATCommandList(char **list);
#endif
