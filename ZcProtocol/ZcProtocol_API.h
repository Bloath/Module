#ifndef _ZCPROTOCOL_API_H_
#define _ZCPROTOCOL_API_H_

/* Includes ------------------------------------------------------------------*/
#include "../Module_Conf.h"
#include "ZcProtocol_Conf.h"

/* typedef ------------------------------------------------------------*/
typedef struct
{
    uint8_t head;
    uint8_t id;
    uint8_t control;
    uint8_t length;
    uint32_t timestamp;
    uint8_t address[7];
    uint8_t cmd;
} ZcProtocolHead;

typedef struct
{
    uint8_t crc;
    uint8_t end;
} ZcProtocolEnd;

typedef struct
{
    ZcProtocolHead head;
    uint8_t data[1];
} ZcProtocol;

typedef enum
{
    ZcHandleStatus_Init = 0,
    ZcHandleStatus_PollInterval,
    ZcHandleStatus_Trans,
    ZcHandleStatus_Wait
} ZcProtocolStatus;

/* define -------------------------------------------------------------*/
#define ZC_HEAD 0x68
#define ZC_END 0x16

#define ZC_HEAD_LEN sizeof(ZcProtocolHead) //结构体长度按照 最宽类型 的整数倍，不符合则填充
#define ZC_END_LEN sizeof(ZcProtocolEnd)
#define ZC_UNDATA_LEN (ZC_HEAD_LEN + ZC_END_LEN)
#define ZC_DATA_LEN

#define ZC_CMD_QUERY_HOLD 0x00     // 暂存
#define ZC_CMD_FAIL 0xFE           // 失败报文
#define ZC_CMD_SERVER_CONFIRM 0xFF // 服务器确认报文
/* macro --------------------------------------------------------------*/
/* variables ----------------------------------------------------------*/
extern ZcProtocol zcPrtc;
extern uint8_t queryId, eventId;
/* function prototypes ------------------------------------------------*/
void ZcProtocol_Init();
uint8_t *ZcProtocol_ConvertMsg(ZcProtocol *zcProtocol, uint8_t *data, uint16_t dataLen);    // 将头部和数据组合，转换为报文
ZcProtocol *ZcProtocol_Check(uint8_t *message, uint16_t length);                            
void ZcProtocol_IdIncrement(bool isQueryId);                                                // 协议ID自增控制，分为请求和其他命令

void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t *address);                         //全局变量ZcPrtc初始化，用于协议头部一些常用数据的写入
uint8_t ZcProtocol_Request(CommunicateStruct *communicate, uint8_t cmd, uint8_t *data, uint16_t dataLen, uint8_t txMode);
void ZcProtocol_Response(CommunicateStruct *communicate, ZcProtocol *zcProtocol, uint8_t *data, uint16_t dataLen, uint8_t txMode);

#endif
