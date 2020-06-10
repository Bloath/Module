#ifndef _NB_API_H_
#define _NB_API_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"
#include "Module/DataStruct/DataStruct.h"

/* Public macro --------------------------------------------------------------*/
/* Public define --------------------------------------------------------------*/
#define NBSOCKET_FLAG_CREATED   (1<<0)
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

struct OrderATCmdStruct
{
    uint8_t index;
    bool isGetOk;
    bool isGetError;
    uint8_t errorCounter;
};

struct HttpParamStruct
{
    char *url;
    char *header;
    char **config;
    uint8_t _counterConfig;
};

struct NBSocketStruct
{
    struct ProcessStruct _process;
    struct NBSocketStruct *next;                       // 同级, 不加入previous
    char *host;                                         // 主机地址
    char *port;                                         // 端口
    uint8_t _counterError;
    int8_t _socketId;
    uint8_t _flag;
    uint8_t methodId;                                   // 方法索引
    void *param;                                        // 参数
                   
    struct TxQueueStruct txQueueApp;                    // 业务层 发送队列
    struct RxQueueStruct rxQueueApp;                    // 业务层 接收队列，拆包后填充至该队列    
    
    void (*CallBack_SocketRxATCommandHandle)(struct RxUnitStruct *, void *);    // socket接收AT指令处理
    void (*CallBack_ErrorHandle)(int id);               // 错误处理
};

struct NBSotaStruct
{
    struct ProcessStruct _process;
    
    char *_token;
    char *authorization;
    char *deviceId;
    void *param;
    char version[32];
    uint32_t _size;
    uint32_t _currentIndex;
    uint16_t unitSize;
    struct HttpParamStruct httpParam;
    struct NBSocketStruct *socket;
    struct MD5Struct md5;
    char md5Package[16];
    char md5Calculate[16];
    
    void (*CallBack_ErrorHandle)(struct NBSotaStruct *sota, int32_t errorCode , char *errorInfo);
    void (*CallBack_UpdatePackage)(struct NBSotaStruct *sota, uint8_t *package, int32_t size);
    void (*CallBack_Finish)(struct NBSotaStruct *sota);
};

struct NBStruct
{
    struct ProcessStruct _process;                      // 流程
    uint32_t _startConnectTime;                        // 开始连接时间
    struct OrderATCmdStruct _orderAT;                  // AT指令索引
    
    
    uint8_t _errorCounter;                              // 在发送状态下产生的错误次数
    bool _isTransmitting;                               // 正在发送
    uint8_t _signal;                                    // 信号强度
    enum SimTypeEnum _sim;
    char **cmdList;
    char _imei[16];                                      // nb模组sn码

    struct NBSocketStruct *sockets;                     // nb连接链表
    struct NBSocketStruct *socketCurrent;               // 当前socket
    struct RxQueueStruct *rxQueueHal;                   // 硬件层 接收队列
    
    void (*CallBack_Transmit)(uint8_t *message, uint16_t length, void *param);  // 发送回调, 一问一答, 无需队列
    void (*CallBack_TxError)(enum NbErrorEnum);                                 // 错误处理
    void (*CallBack_TimeUpdate)(uint32_t timeStamp);                            // 硬件层 获取新时间
    void (*CallBack_GetImei)();                                                 // 获取IMEI
};

/* Public variables ----------------------------------------------------------*/
extern struct NBStruct nb;
/* Public function prototypes ------------------------------------------------*/
void NB_Handle();
bool NB_IsIdle();
void NB_RxHandle(struct RxUnitStruct *rxBlock, void *param);     
void NB_HalTransmit(uint8_t *message, uint16_t length, void *param);
void NB_ErrorHandle(enum NbErrorEnum error);
void NB_PowerOn();
void NB_PowerOff();
void NB_SendATCommandList(char **list);
void NB_AddNewSocket(struct NBSocketStruct *socket);
struct NBSocketStruct* NB_FindSocketById(int8_t id);
bool NB_SocketsIsIdle(struct NBSocketStruct **socket);

#endif
