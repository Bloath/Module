/* Includes ------------------------------------------------------------------*/
#include "ZcProtocol_API.h"

/* private typedef ------------------------------------------------------------*/
/* private define -------------------------------------------------------------*/
/* private macro --------------------------------------------------------------*/
/* private variables ----------------------------------------------------------*/
ZcProtocol zcPrtc; // 拙诚协议实例
uint8_t queryId = 0, eventId = 0;

/* private function prototypes ------------------------------------------------*/
uint8_t ZcProtocol_GetCrc(uint8_t *message, uint16_t length);

/*********************************************************************************************

  * @brief  协议转报文
  * @param  protocol:  协议结构体指针
            
  * @retval 报文数组结构体指针
  * @remark 

  ********************************************************************************************/
uint8_t *ZcProtocol_ConvertMsg(ZcProtocol *zcProtocol, uint8_t *data, uint16_t dataLen)
{
    /* 确定数据长度，并申请相应内存 */
    zcProtocol->head.length = ZC_UNDATA_LEN + dataLen;
    uint8_t *msg = (uint8_t *)Malloc(zcProtocol->head.length);
    
    if(msg == NULL)
    {   return NULL;    }

    /* 按照 头 、数据、结尾的顺序开始对报文进行赋值 */
    memcpy(msg, &zcProtocol->head, sizeof(zcProtocol->head));

    EndianExchange(&( ((ZcProtocol *)msg)->head.timestamp ), &(zcProtocol->head.timestamp), 4);
    
    memcpy(msg + ZC_HEAD_LEN, data, dataLen);   // 复制数据域

    msg[zcProtocol->head.length - 2] = ZcProtocol_GetCrc(msg, zcProtocol->head.length); // 填充CRC
    msg[zcProtocol->head.length - 1] = ZC_END;                                          //填充结束字符

    return msg;
}

/*********************************************************************************************

  * @brief  协议检查，
  * @param  
  * @retval 返回协议指针
  * @remark 检查协议头、尾以及CRC等

  ********************************************************************************************/
ZcProtocol *ZcProtocol_Check(uint8_t *message, uint16_t length)
{
    if (length < ZC_UNDATA_LEN)
    {   return NULL;    }

    ZcProtocol *protocol = (ZcProtocol *)message; // 获取协议结构指针
    ZcProtocolEnd *tail = (ZcProtocolEnd *)(message + protocol->head.length - 2);

    /* 首尾判断 */
    if (protocol->head.head != ZC_HEAD || tail->end != ZC_END)
    {   return NULL;    }

#ifndef DEBUG
    /* CRC判断 */
    if (tail->crc != ZcProtocol_GetCrc(message, protocol->head.length))
    {   return NULL;    }
#endif

    return protocol;
}

/*********************************************************************************************

  * @brief  协议获取CRC
  * @param  message:  报文指针
            length：报文长度
  * @retval 一个字节的crc
  * @remark 

  ********************************************************************************************/
uint8_t ZcProtocol_GetCrc(uint8_t *message, uint16_t length)
{
    uint8_t crc = 0;
    for (uint16_t i = 0; i < (length - 2); i++) //填充CRC
    {   crc += message[i];  }

    crc ^= 0xFF;

    return crc;
}
/*********************************************************************************************

  * @brief  协议初始化
  * @param  DeviceType：设备类型
            address：设备地址指针
  * @retval 
  * @remark 在程序初始化时，需要将协议实例进行初始化

  ********************************************************************************************/
void ZcProtocol_InstanceInit(uint8_t DeviceType, uint8_t *address)
{
    zcPrtc.head.head = 0x68;
    zcPrtc.head.control = DeviceType;
    zcPrtc.head.timestamp = 0;

    queryId = 1;
    eventId = 127;
    
    memcpy(zcPrtc.head.address, address, 7); //复制7字节地址，在产品实际运用后，地址是不会改变的（跟微信挂钩）
}
/*********************************************************************************************

  * @brief  拙诚协议=》协议实例ID自增
  * @param  isQueryId：是否为请求id
  * @retval 
  * @remark 0预留给其他设备

  ********************************************************************************************/
void ZcProtocol_IdIncrement(bool isQueryId)
{
    if (isQueryId == true)
    {
        queryId ++;
        if(queryId == 128)
        {   queryId = 1;    }
    }
    else
    {
        eventId ++;
        if(eventId == 0)
        {   eventId = 128;  }
    }
}

/*********************************************************************************************

  * @brief  拙诚协议 发送请求
  * @param  communicate：通讯结构体对象
            cmd: 命令字
            data:  报文指针
            dataLen：报文长度
            txMode：发送模式
  * @retval 该请求命令的id
  * @remark 通过输入命令以及数据，并填写到发送缓冲当中

  ********************************************************************************************/
int ZcProtocol_Request(CommunicateStruct *communicate, uint8_t cmd, uint8_t *data, uint16_t dataLen, uint8_t txMode)
{
    uint8_t *msg;
    
    zcPrtc.head.timestamp = timeStamp;  
    zcPrtc.head.cmd = cmd;
    
    /* 暂存命令-> Id使用queryId，（1-127）不自增，在接收到当前Id的确认报文后自增 
       其他命令-> Id使用eventId，（128-255）发送后自增*/
    if (cmd == 0x00)
    {   zcPrtc.head.id = queryId;   }
    else
    {
        zcPrtc.head.id = eventId;
        ZcProtocol_IdIncrement(false);      // 事件Id自增
    }

    msg = ZcProtocol_ConvertMsg(&zcPrtc, data, dataLen);
    
    if(msg == NULL)
    {   return -1;  }
    
    TxQueue_AddWithId(communicate->txQueue, msg, msg[3], txMode | TX_FLAG_IS_MALLOC, zcPrtc.head.id);

    return zcPrtc.head.id;
}
/*********************************************************************************************

  * @brief  拙诚协议 回复，在接收到命令后的回复
  * @param  communicate：通讯结构体对象
            zcProtocol：接收包中解析出来的数据结构指针
            data:  报文指针
            dataLen：报文长度
  * @retval 
  * @remark 与请求不同，回复一般是用请求的ID、CMD等，仅仅是数据部分有区别

  ********************************************************************************************/
void ZcProtocol_Response(CommunicateStruct *communicate, ZcProtocol *zcProtocol, uint8_t *data, uint16_t dataLen, uint8_t txMode)
{
    uint8_t *msg;

    /* 根据不同的源，进行不同的发送处理 */
    msg = ZcProtocol_ConvertMsg(zcProtocol, data, dataLen);
    
    if(msg == NULL)
    {   return; }
    
    TxQueue_AddWithId(communicate->txQueue, msg, msg[3], txMode | TX_FLAG_IS_MALLOC, zcProtocol->head.id);
}
