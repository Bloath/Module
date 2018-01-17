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
  uint8_t *data;
}ZcProtocol;

/* Public define ------------------------------------------------------------*/
#define ZC_HEAD   0x68
#define ZC_END    0x16

#define ZC_HEAD_LEN     sizeof(ZcProtocolHead)    //�ṹ�峤�Ȱ��� ������� ��������������������� 
#define ZC_END_LEN      sizeof(ZcProtocolEnd)
#define ZC_UNDATA_LEN   (ZC_HEAD_LEN + ZC_END_LEN)
#define ZC_DATA_LEN

#define ZC_CMD_QUERY_HOLD       0x00    // �ݴ�
#define ZC_CMD_ADDRESS          0x10    //��ַ��
#define ZC_CMD_DEVICE_ATTR      0x11    //�豸����
#define ZC_CMD_MEASURE_ATTR     0x21    //��������
#define ZC_CMD_LADIR_PRICE      0x22    //���ݷ���
#define ZC_CMD_RECHARGE         0x23    //��ֵ��¼
#define ZC_CMD_USE_HISTORY      0x24    //������ʷ��¼
#define ZC_CMD_ALARM            0x81    //�����¼�
#define ZC_CMD_ENVIROMENT       0x82    //��������
#define ZC_CMD_PIPE_STATUS      0x83    //�ܵ�״̬
#define ZC_CMD_VALVE_RECORD     0x84    //���ſ��ؼ�¼
#define ZC_CMD_VALVE_OPRT       0xE1    //���ſ��ط�
#define ZC_CMD_SERVER_CONFIRM   0xFF    //������ȷ�ϱ���

/* Public macro -------------------------------------------------------------*/
/* Public variables ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/
void ZcProtocol_Init();
ArrayStruct* ZcProtocol_ConvertMsg(ZcProtocol* zcProtocol, uint8_t *data, uint16_t dataLen);          //��ͷ����������ϣ�ת��Ϊ����
char* ZcProtocol_ConvertHttpString(ZcProtocol* zcProtocol, uint8_t *data, uint16_t dataLen);            //����ϵĻ����ϣ�ת��Ϊ�ַ�������������HTTPЭ����

ZcProtocol* ZcProtocol_Check(uint8_t *message, uint16_t length);
uint8_t ZcProtocol_SameId(uint8_t *message, uint16_t length, void *p);

#endif