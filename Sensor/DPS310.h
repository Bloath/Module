/*
 * @file       DPS310.h
 * @brief      DPS310压力传感器头文件
 * @author     dong
 * @version    v0.1
 * @date       2016-11-03
 */

#define DPS310_DeviceAddress  0xEE      // 最后一位“0”代表写操作，最后一位“1”代表读操作

#define DPS310_StartAddress    0x00      // 寄存器起始地址
#define DPS310_RegisterCount   41        // 寄存器个数

#define PRODUCT_ID            0x0D      // 产品版本号

#define COEF                  0x10      // 计算系数
#define COEF_COUNT            18        // 计算系数的个数

#define FIFO_STS              0x0B      // FIFO状态  
                                        /* FIFO_FULL  - 1 or 0(1代表满，0代表不满),  
                                           FIFO_EMPTY - 1 or 0(1代表空，0代表不空)*/

#define INT_SYS               0x0A      // 中断状态   
                                        /* INT_FIFO_FULL - 1 or 0(1代表有中断)，
                                           INT_TMP       - 1 or 0(1代表有中断),
                                           INT_PRS       - 1 or 0(1代表有中断) */

#define CFG_REG               0x09      // 中断和FIFO设置 
                                        /* INT_HL      SDO中断使能
                                           INT_FIFO    FIFO满中断
                                           INT_TMP     温度测量准备好中断
                                           INT_PRS     压力测量准备好中断
                                           T_SHIFT     温度测量结果是否移位
                                           P_SHIFT     压力测量结果是否移位
                                           FIFO_EN     使能FIFO
                                           SPI_MODE    设置SPI模式 */ 

#define MEAS_CFG               0x08      // 工作模式及状态
                                         /* COEF_RDY   1bit 系数是否可用
                                            SENSOR_RDY 1bit 传感器初始化是否成功
                                            TMP_RDY    1bit 新的温度准备好
                                            PRS_RDY    1bit 新的压力准备好
                                            reserve    1bit 预留
                                            MEAS_CTRL  3bit 设置工作模式
                                                  standby mode
                                                    000 - 空闲 / 停止
                                                  command mode
                                                    001 - 压力测量
                                                    010 - 温度测量
                                                    011 - none
                                                    100 - none
                                                  background mode
                                                    101 - 持续压力测试
                                                    110 - 持续温度测试
                                                    111 - 持续压力和温度测试 */       

#define TMP_CFG                 0x07     // 温度测量设置
                                         /* TMP_EXT  设置使用内部传感器还是外部传感器
                                            TMP_RATE[2:0]  设置每秒采样次数
                                                  000 - 每秒采样1次
                                                  001 - 每秒采样2次
                                                  010 - 每秒采样4次
                                                  011 - 每秒采样8次
                                                  100 - 每秒采样16次
                                                  101 - 每秒采样32次
                                                  110 - 每秒采样64次
                                                  111 - 每秒采样128次
                                            TMP_PRC[3:0]  设置每次采样中采集几次求平均值
                                                  0000 - 1次
                                                  0001 - 2次
                                                  0010 - 4次
                                                  0011 - 8次
                                                  0100 - 16次
                                                  0101 - 32次
                                                  0110 - 64次
                                                  0111 - 128次
                                                  1××× - 预留 */

#define PRS_CFG                 0x06     // 压力测量设置
                                         /* Reserved  预留
                                            PM_RATE[2:0]  设置每秒采样次数
                                                  000 - 每秒采样1次
                                                  001 - 每秒采样2次
                                                  010 - 每秒采样4次
                                                  011 - 每秒采样8次
                                                  100 - 每秒采样16次
                                                  101 - 每秒采样32次
                                                  110 - 每秒采样64次
                                                  111 - 每秒采样128次
                                            PM_PRC[3:0]  设置每次采样中采集几次求平均值
                                                  0000 - 1次
                                                  0001 - 2次
                                                  0010 - 4次
                                                  0011 - 8次
                                                  0100 - 16次
                                                  0101 - 32次
                                                  0110 - 64次
                                                  0111 - 128次
                                                  1××× - 预留 */
                                             
#define PRS_B0              0x02
#define PRS_B1              0x01
#define PRS_B2              0x00
#define TMP_B0              0x05
#define TMP_B1              0x04
#define TMP_B2              0x03

#define PRS_STARTADDRESS    0x00
#define PRS_COUNT           3
#define TMP_STARTADDRESS    0x03
#define TMP_COUNT           3

#define DPS310_CONFIG_1     1

typedef enum DPS310_cfg
{
    CONFIG_1,
    CONFIG_2,
    CONFIG_3,
    CONFIG_4,
} DPS310_config;
     
typedef struct 
{
  int C0;
  int C1;
  int C00;
  int C10;
  int C01;
  int C11;
  int C20;
  int C21;
  int C30;
} struct_DPS310_coef;

U8 DPS310_ReadRegister( U8 registerAddress, U8 *dataSave);
U8 DPS310_ReadRegisters(U8 startAddress, U8 dataCount, U8 * dataBuff);
U8 DPS310_WriteRegister(U8 registerAddress, U8 dataBuff);
void DPS310_Config(DPS310_config configNumber);
struct_DPS310_coef DPS310_Get_Coef();
void DPS310_Get_Pressure(struct_DPS310_coef coef, U8* dataSave );

