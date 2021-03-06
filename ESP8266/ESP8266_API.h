#ifndef _ESP8266_API_H_
#define _ESP8266_API_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"
#include "Module/DataStruct/DataStruct.h"
#include "ESP8266_Conf.h"
#include "ESP8266_HAL.h"

/* Public typedef ------------------------------------------------------------*/
enum ESP8266_Error
{
    Error_Timeout,
    Error_CipSendError,
    Error_ResponseError,
    Error_TcpTimeout,
    Error_AirKissError,
    Error_NoAP,
    Error_ActiveClose,
    Error_Rst3Times,
    Error_GetResetInfo,
    Error_Lock,
};

enum Esp8266AirkissEnum
{
    Esp8266Airkiss_IsWriteDefault = 0,
    Esp8266Airkiss_WrittingDefault
};

enum ESP8266_ConnectProcessEnum
{
    ConnectStatus_Init = 0,
    ConnectStatus_Idle,
    ConnectStatus_Reset,
    ConnectStatus_ResetWait,
    ConnectStatus_ResetFinish,
    ConnectStatus_AirKiss,
    ConnectStatus_AirKissWait,
    ConnectStatus_Connected,
    ConnectStatus_WaitAck,
} ;

enum ESP8266_UTProcessEnum
{
    UTStatus_Init,
    UTStatus_Connected,
    UTStatus_StartTrans,
    UTStatus_WaitAck,
    UTStatus_SendOk
};

struct Esp8266HttpStruct
{
    char *host;
    char *port;
    int (*CallBack_PacketLength)(uint16_t length, char *host, char *port);
    char* (*CallBack_HttpPackage)(uint8_t *message, uint16_t length, char *host, char *port);
};

struct Esp8266Struct
{
    uint32_t __time;                                    // 流程时间，单位为s
    enum ESP8266_ConnectProcessEnum _conProcess;        // 连接流程，包括airkiss、查询是否连接等
    enum ESP8266_UTProcessEnum  UTProcess;              // TCP UDP连接流程，建立TCP连接，发送数据
    uint8_t _tcpFailCounter;                            // 发送错误计数器，有可能网络不稳或延迟导致的
    uint8_t _timeOutCounter;                            // 复位次数计数器
    uint8_t _flag;                                      // 标志位，表示当前模块状态
    uint8_t illegalResetCounter;                        // 非正常复位计数器（在正常复位中，串口是关闭的，是无法收到复位信息的）
    
    struct Esp8266HttpStruct http;                             // Http相关
    
    struct TxQueueStruct *txQueueHal;                          // 硬件层 发送队列
    struct RxQueueStruct *rxQueueHal;                          // 硬件层 接收队列
    struct TxQueueStruct *txQueueApp;                          // 业务层 发送队列
    struct RxQueueStruct *rxQueueApp;                          // 业务层 接收队列，拆包后填充至该队列
    
    bool (*CallBack_HalTxFunc)(uint8_t *, uint16_t);            // 硬件层 发送处理函数
    void *halTxParam;                                           // 硬件层 发送处理函数参数  
    
    void (*CallBack_ErrorHandle)(enum ESP8266_Error);               // 业务层
    bool (*CallBack_Airkiss)(enum Esp8266AirkissEnum operation);    // airkiss处理
};

/* Public define -------------------------------------------------------------*/
#define ESP8266_WIFI_CONNECTED (1 << 0)
#define ESP8266_TCP_CONNECTED (1 << 1)
#define ESP8266_AIRKISS (1 << 2)            // 有airkiss的操作
#define ESP8266_AIRKISSING (1 << 3)         // 正在airkiss的期间
#define ESP8266_AIRKISSED (1 << 4)          // airkiss完成
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern struct Esp8266Struct esp8266;
/* Public function prototypes ------------------------------------------------*/

void ESP8266_Handle();
bool ESP8266_IsIdle();
void ESP8266_Reset();
void ESP8266_PowerOn();
void ESP8266_EnableAirkiss();
void ESP8266_RxMsgHandle(struct RxUnitStruct *rxBlock, void *param);
void ESP8266_SendAtString(const char *cmd);
void ESP8266_ErrorHandle(enum ESP8266_Error errorType);

#endif
