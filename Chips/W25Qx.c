/* Includes ----------------------------------------------------------*/
#include "W25Qx.h"
#include "Module/Module.h"
/* define ------------------------------------------------------------*/
/* typedef -----------------------------------------------------------*/
/* macro -------------------------------------------------------------*/
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/
int W25Q_SendCmd(struct W25QStruct *w25q, uint8_t cmd);
int W25Q_SendCmdWithContent(struct W25QStruct *w25q, uint8_t cmd, uint8_t *content, uint16_t length);
int W25Q_ReceiveCmd(struct W25QStruct *w25q, uint8_t cmd, uint8_t *rx, uint16_t length);
int W25Q_ReceiveCmdArray(struct W25QStruct *w25q, uint8_t *cmd, uint16_t cmdLength, uint8_t *rx, uint16_t length);
/*********************************************************************************************
 
    W25Q 初始化, deviceId验证,
 
  ********************************************************************************************/
int W25Q_Init(struct W25QStruct *w25q)
{
    uint8_t  cmd[4]={0}, i=0;
    int result = 0;
    struct W25QIndexStruct indexTemp = {0};
  
    /* 读取并验证设备编号 */
    cmd[0] = 0x90;
    if((result = W25Q_ReceiveCmdArray(w25q, cmd, 4, w25q->deviceId, 2)) != 0)
    {   return -1;  }
    
    if(w25q->deviceId[0] != 0xEF)
    {   return -2;  }
    
    /* 获取索引个数 */
    for(i=0; i<16; i++)
    {
        if(W25Q_GetIndex(w25q, i, &indexTemp))
        {   break;  }
    }
    
    w25q->indexCount = i;
    
    return 0;
}

/*********************************************************************************************
 
    读取索引信息, 正确获取后要进行free
 
  ********************************************************************************************/
struct W25QIndexStruct* W25Q_GetIndexWithMalloc(struct W25QStruct *w25q, uint16_t pageIndex)
{
    struct W25QIndexStruct* index = (struct W25QIndexStruct*)Malloc(sizeof(struct W25QIndexStruct));
    
    W25Q_Read(w25q, pageIndex * W25Q_PAGE_SIZE, (uint8_t *)index, sizeof(struct W25QIndexStruct));
    
    /* 查看固定的数字是否为0 */
    if(index->fixedZero != 0)
    {   goto end;   }
    
    /* 查看crc是否准确 */
    if(index->crc32 != Crc32(index, sizeof(struct W25QIndexStruct) - 4, 0))
    {   goto end;   }
    
    return index;
end:
    Free(index);
    return NULL;
}
/*********************************************************************************************
 
    读取索引信息, -1: 读取错误, -2: 固定为不为0, -3: crc32错误
 
  ********************************************************************************************/
int W25Q_GetIndex(struct W25QStruct *w25q, uint16_t pageIndex, struct W25QIndexStruct *index)
{
    int result = 0;
    
    if(W25Q_Read(w25q, pageIndex * W25Q_PAGE_SIZE, (uint8_t *)index, sizeof(struct W25QIndexStruct)) != 0)
    {   return -1;  }
    
    /* 查看固定的数字是否为0 */
    if(index->fixedZero != 0)
    {   return -2;   }
    
    /* 查看crc是否准确 */
    if(index->crc32 != Crc32(index, sizeof(struct W25QIndexStruct) - 4, 0))
    {   return -3;   }
    
    return 0;
}

/*********************************************************************************************
 
    写入索引信息, 不需要提前处理crc
 
  ********************************************************************************************/
int W25Q_WriteIndex(struct W25QStruct *w25q, uint16_t pageIndex, struct W25QIndexStruct *index)
{
    // 计算crc
    index->crc32 = Crc32(index, sizeof(struct W25QIndexStruct) - 4, 0);
    index->fixedZero = 0;
    
    return W25Q_WritePage(w25q, pageIndex * W25Q_PAGE_SIZE, (uint8_t *)index, sizeof(struct W25QIndexStruct));
}


/*********************************************************************************************
 
    写flash, 按照每页写(256字节)
 
  ********************************************************************************************/
int W25Q_WritePage(struct W25QStruct *w25q, uint32_t address, uint8_t *data, uint16_t length)
{
    uint8_t cmd[4], *p = (uint8_t*)&address, status;
    int result = 0;
    
    W25Q_SendCmd(w25q, 0x06);               // Write Enable
    
    cmd[0] = 0x02;
    cmd[1] = p[2];
    cmd[2] = p[1];
    cmd[3] = p[0];
  
    w25q->CallBack_CSControl(true);
    result = w25q->CallBack_SpiWrite(cmd, 4);
    result = w25q->CallBack_SpiWrite(data, length);
    w25q->CallBack_CSControl(false);
    
    while(true)
    {
        result = W25Q_ReceiveCmd(w25q, 0x05, &status, 1);
        if(FLAG_IS_CLR(status, W25Q_STATUS_BUSY))
        {   break;  }
    }
    
    W25Q_SendCmd(w25q, 0x04);                // Write Disable
    
    return result;
}
/*********************************************************************************************
 
    写flash, 自动对数据宽度进行处理(地址与长度最好都是1页的倍数(256字节))
 
  ********************************************************************************************/
int W25Q_Write(struct W25QStruct *w25q, uint32_t address, uint8_t *data, uint16_t length)
{
    uint32_t pageCount = 0, i=0 ,temp32 = length % 256;
    
    if(temp32 == 0)
    {   pageCount = length >> 8;    }
    else
    {   pageCount = length >> 8 + 1;    }
    
    for(i=0; i<pageCount; i++)
    {   W25Q_WritePage(w25q, address + i * 256, data + i * 256, (i == (pageCount - 1))? temp32:256);    } 
    
    return 0;
}

/*********************************************************************************************
 
    擦写flash, 4种模式, 4k 32k 64k chip
 
  ********************************************************************************************/
int W25Q_Erase(struct W25QStruct *w25q, enum W25QEarseType earseType, uint32_t address)
{
    uint8_t cmd[4], *p = (uint8_t*)&address, status;
    int result = 0;
    
    W25Q_SendCmd(w25q, 0x06);               // Write Enable
    
    switch(earseType)
    {
    case W25QEarse_Sector4K:
        cmd[0] = 0x20;
        break;
    case W25QEarse_Block32k:
        cmd[0] = 0x52;
        break;
    case W25QEarse_Block64k:
        cmd[0] = 0xD8;
        break;
    case W25QEarse_Chip: 
        cmd[0] = 0xC7;
        break;
    }
    cmd[1] = p[2];
    cmd[2] = p[1];
    cmd[3] = p[0];
    
    w25q->CallBack_CSControl(true);
    result = w25q->CallBack_SpiWrite(cmd, 1);
    if(earseType != W25QEarse_Chip)
    {   result = w25q->CallBack_SpiWrite(cmd + 1, 3);  }
    w25q->CallBack_CSControl(false);
    
    while(true)
    {
        result = W25Q_ReceiveCmd(w25q, 0x05, &status, 1);
        if(FLAG_IS_CLR(status, W25Q_STATUS_BUSY))
        {   break;  }
    }
    
    W25Q_SendCmd(w25q, 0x04);                // Write Disable
    
    return result;
}

/*********************************************************************************************
 
    flash 读
 
  ********************************************************************************************/
int W25Q_Read(struct W25QStruct *w25q, uint32_t address, uint8_t *data, uint16_t length)
{
    uint8_t cmd[4], *p = (uint8_t*)&address;
    int result = 0;
    
    cmd[0] = 0x03;
    cmd[1] = p[2];
    cmd[2] = p[1];
    cmd[3] = p[0];

    w25q->CallBack_CSControl(true);
    result = w25q->CallBack_SpiWrite(cmd, 4);
    result = w25q->CallBack_SpiRead(data, length);
    w25q->CallBack_CSControl(false);
    
    return result;
}

/*********************************************************************************************
 
    发送命令处理
 
  ********************************************************************************************/
int W25Q_SendCmd(struct W25QStruct *w25q, uint8_t cmd)
{
    int result = 0;
    w25q->CallBack_CSControl(true);
    result = w25q->CallBack_SpiWrite(&cmd, 1);
    w25q->CallBack_CSControl(false);
    
    return result;
}
int W25Q_SendCmdWithContent(struct W25QStruct *w25q, uint8_t cmd, uint8_t *content, uint16_t length)
{
    int result=0;
    w25q->CallBack_CSControl(true);
    result = w25q->CallBack_SpiWrite(&cmd, 1);
    result = w25q->CallBack_SpiWrite(content, length);
    w25q->CallBack_CSControl(false);
    
    return result;
}
/*********************************************************************************************
 
    读取命令
 
  ********************************************************************************************/
int W25Q_ReceiveCmd(struct W25QStruct *w25q, uint8_t cmd, uint8_t *rx, uint16_t length)
{
    int result=0;
    w25q->CallBack_CSControl(true);
    result = w25q->CallBack_SpiWrite(&cmd, 1);
    result = w25q->CallBack_SpiRead(rx, length);
    w25q->CallBack_CSControl(false);
    return result;
}
int W25Q_ReceiveCmdArray(struct W25QStruct *w25q, uint8_t *cmd, uint16_t cmdLength, uint8_t *rx, uint16_t length)
{
    int result=0;
    w25q->CallBack_CSControl(true);
    result = w25q->CallBack_SpiWrite(cmd, cmdLength);
    result = w25q->CallBack_SpiRead(rx, length);
    w25q->CallBack_CSControl(false);
    return result;
}