#ifndef _ZCPROTOCOL_H_
#define _ZCPROTOCOL_H_

/* Includes ------------------------------------------------------------------*/

#include "stdlib.h"
#include "string.h"
#include "../Sys_Conf.h"
#include "../Common/Array.h"


/* Public typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t head;
  uint8_t id;
  uint8_t control;
  uint8_t length;
  uint32_t timestamp;
  uint8_t address[7];
  uint8_t cmd;
}ZcProtocolHead;

typedef struct
{
  uint8_t crc;
  uint8_t end;
}ZcProtocolEnd;

typedef struct
{
  ZcProtocolHead head;
  uint8_t data;
}ZcProtocol;

typedef struct
{
  uint32_t flag;
  uint32_t flagCache;
}ZcErrorStruct;

/* Public define ------------------------------------------------------------*/
#define ZC_HEAD   0x68
#define ZC_END    0x16

#define ZC_HEAD_LEN     sizeof(ZcProtocolHead)    //结构体长度按照 最宽类型 的整数倍，不符合则填充 
#define ZC_END_LEN      sizeof(ZcProtocolEnd)
#define ZC_UNDATA_LEN   (ZC_HEAD_LEN + ZC_END_LEN)
#define ZC_DATA_LEN

#define ZC_CMD_QUERY_HOLD       0x00    // 暂存
#define ZC_CMD_ADDRESS          0x10    // 地址域
#define ZC_CMD_DEVICE_ATTR      0x11    // 设备属性
#define ZC_CMD_MEASURE_ATTR     0x21    // 计量属性
#define ZC_CMD_LADIR_PRICE      0x22    // 阶梯费用
#define ZC_CMD_RECHARGE         0x23    // 充值记录
#define ZC_CMD_USE_HISTORY      0x24    // 用气历史记录
#define ZC_CMD_ALARM            0x81    // 报警事件
#define ZC_CMD_ENVIROMENT       0x82    // 环境参数
#define ZC_CMD_PIPE_STATUS      0x83    // 管道状态
#define ZC_CMD_VALVE_RECORD     0x84    // 阀门开关记录
#define ZC_CMD_ANALYSE_INFO     0x85    // 算法信息
#define ZC_CMD_VALVE_OPRT       0xE1    // 阀门开关阀
#define ZC_CMD_FAIL             0xFE    // 失败报文
#define ZC_CMD_SERVER_CONFIRM   0xFF    // 服务器确认报文

#define ZC_ERROR_FLOWOVERLOAD           0x00000080      // 流量过载
#define ZC_ERROR_GASMETERBREAK          0x00000040
#define ZC_ERROR_VALVEBREAK             0x00000020        
#define ZC_ERROR_BOARDBREAK             0x00000010
#define ZC_ERROR_OWE                    0x00000008
#define ZC_ERROR_UNBALANCE              0x00000004
#define ZC_ERROR_VALVECLOSE             0x00000002
#define ZC_ERROR_VALVEBAN               0x00000001
#define ZC_ERROR_SEI                    0x00008000
#define ZC_ERROR_PIPEBREAK              0x00004000
#define ZC_ERROR_GASLEAK                0x00002000
#define ZC_ERROR_OUTOFLEL               0x00001000
#define ZC_ERROR_FLOWOVER90             0x00000800
#define ZC_ERROR_FLOWOVER05             0x00000400
#define ZC_ERROR_LONGTIMECLOSE          0x00000200
#define ZC_ERROR_BATTERYLOW             0x00000100
//#define ZC_ERROR_RESERVE                0x00800000
//#define ZC_ERROR_RESERVE                0x00400000
//#define ZC_ERROR_RESERVE                0x00200000
//#define ZC_ERROR_RESERVE                0x00100000
#define ZC_ERROR_SHOCK                  0x00080000
#define ZC_ERROR_FIRE                   0x00040000
#define ZC_ERROR_UNDERPRESSURE          0x00020000
#define ZC_ERROR_OVERPRESSURE           0x00010000
//#define ZC_ERROR_RESERVE                0x80000000
//#define ZC_ERROR_RESERVE                0x40000000
//#define ZC_ERROR_RESERVE                0x20000000
//#define ZC_ERROR_RESERVE                0x10000000
//#define ZC_ERROR_RESERVE                0x08000000
//#define ZC_ERROR_RESERVE                0x04000000
//#define ZC_ERROR_RESERVE                0x02000000
//#define ZC_ERROR_RESERVE                0x01000000

/* Public macro -------------------------------------------------------------*/
/* Public variables ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/
void ZcProtocol_Init();
ArrayStruct* ZcProtocol_ConvertMsg(ZcProtocol* zcProtocol, uint8_t *data, uint16_t dataLen);          //将头部和数据组合，转换为报文
char* ZcProtocol_ConvertHttpString(ZcProtocol* zcProtocol, uint8_t *data, uint16_t dataLen);            //在组合的基础上，转换为字符串，并包含在HTTP协议内

ZcProtocol* ZcProtocol_Check(uint8_t *message, uint16_t length);

#endif