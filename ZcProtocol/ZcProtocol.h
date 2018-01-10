#ifndef _ZCPROTOCOL_H_
#define _ZCPROTOCOL_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "Array.h"

/* Public typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t head;
  uint8_t control;
  uint16_t length;
  uint16_t info;
  uint8_t address[7];
  uint8_t afn;
  uint8_t id;
}ZcProtocolHead;

typedef struct
{
  uint8_t crc;
  uint8_t end;
}ZcProtocolEnd;

typedef struct
{
  ZcProtocolHead head;
  uint8_t *data;
}ZcProtocol;

/* Public define ------------------------------------------------------------*/
#define ZC_HEAD   0x68
#define ZC_END    0x16

#define ZC_HEAD_LEN     (sizeof(ZcProtocolHead) - 1)    //结构体长度按照 最宽类型 的整数倍，不符合则填充 
#define ZC_END_LEN      sizeof(ZcProtocolEnd)
#define ZC_UNDATA_LEN   (ZC_HEAD_LEN + ZC_END_LEN)
#define ZC_DATA_LEN

/* Public macro -------------------------------------------------------------*/
/* Public variables ---------------------------------------------------------*/
extern ZcProtocol zcPrtc;

/* Public function prototypes -----------------------------------------------*/
void ZcProtocol_Init();
ArrayStruct* ZcProtocol_ConvertMsg(ZcProtocol* zcProtocol, uint8_t *message, uint16_t length);
ZcProtocol* ZcProtocol_Check(uint8_t *message, uint16_t length);
uint8_t ZcProtocol_SameId(uint8_t *message, uint16_t length, void *p);

#endif