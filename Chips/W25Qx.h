#ifndef _W25QX_H_
#define _W25QX_H_

/* Includes ----------------------------------------------------------*/
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
/* define ------------------------------------------------------------*/
#define W25Q_STATUS_BUSY    (1<<0)


#define W25Q_PAGE_SIZE  256
/* typedef -----------------------------------------------------------*/

enum W25QEarseType
{
    W25QEarse_Sector4K = 0,
    W25QEarse_Block32k,
    W25QEarse_Block64k,
    W25QEarse_Chip
};

struct W25QIndexStruct
{
    uint32_t fixed;                 // 固定为0, 判断当前头部信息是否正常
    uint16_t sectorIndex[2];        // 扇区索引
    uint32_t totalSize;
    uint32_t timeStamp;             // 时间戳
    char name[64];                  // 最多支持63个字符
    char md5[16];                   // MD5
    uint32_t crc32;                 // 对前面的所有数据的crc校验
};

struct W25QStruct               
{
    uint16_t indexCount;            // 索引数量, 第一个32k扇区用于存放索引
    uint8_t deviceId[2];            // 设备型号
    
    void (*CallBack_CSControl)(bool enable);
    int (*CallBack_SpiWrite)(uint8_t *packet, uint16_t length);
    int (*CallBack_SpiRead)(uint8_t *packet, uint16_t length);    
};

/* macro -------------------------------------------------------------*/
#define W25Q_PowerDown(w25q)   W25Q_SendCmd(w25q, 0xB9) 
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/
int W25Q_WritePage(struct W25QStruct *w25q, uint32_t address, uint8_t *data, uint16_t length);
int W25Q_Write(struct W25QStruct *w25q, uint32_t address, uint8_t *data, uint16_t length);
int W25Q_Erase(struct W25QStruct *w25q, enum W25QEarseType earseType, uint32_t address);
int W25Q_Read(struct W25QStruct *w25q, uint32_t address, uint8_t *data, uint16_t length);

struct W25QIndexStruct* W25Q_GetIndexWithMalloc(struct W25QStruct *w25q, uint16_t pageIndex);
int W25Q_WriteIndex(struct W25QStruct *w25q, uint16_t pageIndex, struct W25QIndexStruct *index);
int W25Q_GetIndex(struct W25QStruct *w25q, uint16_t pageIndex, struct W25QIndexStruct *index);
int W25Q_FileCheck(struct W25QStruct *w25q, struct W25QIndexStruct *index);
int W25Q_Init(struct W25QStruct *w25q);

int W25Q_SendCmd(struct W25QStruct *w25q, uint8_t cmd);
int W25Q_SendCmdWithContent(struct W25QStruct *w25q, uint8_t cmd, uint8_t *content, uint16_t length);
int W25Q_ReceiveCmd(struct W25QStruct *w25q, uint8_t cmd, uint8_t *rx, uint16_t length);
int W25Q_ReceiveCmdArray(struct W25QStruct *w25q, uint8_t *cmd, uint16_t cmdLength, uint8_t *rx, uint16_t length);

#endif