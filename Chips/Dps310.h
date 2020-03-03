#ifndef _DPS310_H_
#define _DPS310_H_
/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"
/* typedef -----------------------------------------------------------*/
struct DPS310CoefStruct
{
  int32_t C0;
  int32_t C1;
  int32_t C00;
  int32_t C10;
  int32_t C01;
  int32_t C11;
  int32_t C20;
  int32_t C21;
  int32_t C30;
  uint32_t pressFactor;
  uint32_t tempFactor;
};
 
struct Dps310FifoStruct
{
    uint32_t origin[32];            // fifo原始数据
    uint32_t lastTemperatureOrigin; // 上一次的温度原始数据
    uint32_t pressureList[32];      // 压力数据转换
    int16_t temperature;            // 温度
    uint16_t oCount;                // 原始数据数量
    uint16_t pCount;                // 压力数据个数
};

struct Dps310SDataStruct
{
    uint32_t pressureOrigin;
    uint32_t temperatureOrigin;
    uint32_t pressure;
    int16_t temperature;
    struct ProcessStruct _process;  // 流程
};

struct Dps310Struct
{
    /* 硬件相关 */
    uint32_t __time;                // 用于记录超时
    struct SoftI2cStruct *i2c;
    uint8_t devAddress;             // 设备地址
    
    struct DPS310CoefStruct coef;   // 参数
    struct Dps310FifoStruct *fifo;  // fifo，为指针，如果需要则新建
    struct Dps310SDataStruct *data; // 数据
    
    void (*CallBack_ReadData)(struct Dps310Struct *);
    void (*CallBack_ReadFifo)(struct Dps310Struct *);
} ;

/* define ------------------------------------------------------------*/
#define DPS310_StartAddress    0x00      // 寄存器起始地址
#define DPS310_RegisterCount   41        // 寄存器个数
      
#define PRS_B2                  0x00
#define PRS_B1                  0x01
#define PRS_B0                  0x02
#define TMP_B2                  0x03
#define TMP_B1                  0x04
#define TMP_B0                  0x05
#define PRS_CFG                 0x06
#define TMP_CFG                 0x07
#define MEAS_CFG                0x08            // 工作模式及状态
#define CFG_REG                 0x09            // 中断和FIFO设置 
#define INT_SYS                 0x0A            // 中断状态  
#define FIFO_STS                0x0B            // 中断状态  
#define FIFO_FLUSH              0x0C            // Reset and FIFO flush 
#define PRODUCT_ID              0x0D            // 产品版本号
#define COEF                    0x10            // 计算系数
#define TMP_COEF_SRCE           0x28

#define COEF_COUNT              18              // 计算系数的个数
#define COEF_AVALIABLE          0x80            // coef 参数可用

#define PRS_STARTADDRESS        PRS_B2
#define TMP_STARTADDRESS        TMP_B2

//#define DPS310_CONFIG_1     1
                          
#define  DPS310_MODE_IDLE         0                                                   
#define  DPS310_MODE_COMMAND_P    1    
#define  DPS310_MODE_COMMAND_T    2  
#define  DPS310_MODE_BACKFROUND_P   5       
#define  DPS310_MODE_BACKFROUND_T   6   
#define  DPS310_MODE_BACKFROUND_P_T  7  


#define DPS310_MEASURE_PRESSURE     0x01
#define DPS310_MEASURE_TEMPERATURE  0x02

    
#define DPS310_CONFIG_BG_FIFO   0x00000702      


/* macro -------------------------------------------------------------*/
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/

int Dps310_Init(struct Dps310Struct *dps310, uint32_t config);
int32_t Dps310_GetCoef(struct Dps310Struct *dps310);

int Dps310_SyncRead(struct Dps310Struct *dps310, uint32_t timeOut);
void Dps310_AsynRead(struct Dps310Struct *dps310, uint32_t timeOut);
void Dps310_AsynReadStart(struct Dps310Struct *dps310);
int Dps310_BackgroudRead(struct Dps310Struct *dps310);
void Dps310_ClearFIFO(struct Dps310Struct *dps310);

#endif