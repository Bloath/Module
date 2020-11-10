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
    
     void (*CallBack_ErrorHandle)(struct NBSocketStruct *socket, int code);               // 错误处理
};

struct NBSotaStruct
{
    struct ProcessStruct _process;
    
    char _token[32];
    char *authorization;
    void *param;
    char *versionLast;
    char versionNew[16];
    char deviceId[16];
    uint32_t _size;
    uint32_t _currentIndex;
    uint16_t unitSize;
    int16_t _result;
    struct HttpParamStruct httpParam;
    struct HttpParamStruct httpParamIdCheck;
    struct NBSocketStruct socket;
    struct NBSocketStruct socketIdCheck;
    char md5Package[16];
    
    void (*CallBack_UpdatePackage)(struct NBSotaStruct *sota, uint8_t *package, int32_t size);
    int (*CallBack_DownloadFinish)(struct NBSotaStruct *sota);
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
    void (*CallBack_ErrorHandle)(int errCode);                                 // 错误处理
    void (*CallBack_TimeUpdate)(uint32_t timeStamp);                            // 硬件层 获取新时间
    void (*CallBack_GetImei)();                                                 // 获取IMEI
};

/* Public variables ----------------------------------------------------------*/
extern struct NBStruct nb;
extern char *nbConfiguration[];
/* Public function prototypes ------------------------------------------------*/
void NB_Handle();
bool NB_IsIdle();
void NB_RxHandle(struct RxUnitStruct *rxBlock, void *param);     
void NB_HalTransmit(uint8_t *message, uint16_t length, void *param);
void NB_PowerOn();
void NB_PowerOff();
void NB_SendATCommandList(char **list);
void NB_AddNewSocket(struct NBSocketStruct *socket);
struct NBSocketStruct* NB_FindSocketById(int8_t id);
bool NB_SocketsIsIdle(struct NBSocketStruct **socket);
void NB_SocketLoopHandle(void (*handleFunc)(struct NBSocketStruct *, void *param), void *param);
#endif
