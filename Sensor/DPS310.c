/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#include "DPS310.h" 
#include "SimulatedI2C.h"
#include "Delay.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
I2C_PinStruct dps310_i2c;       //创建一个I2C实例（修改引脚什么的）

/* Private function prototypes ------------------------------------------------*/


/******************************************************************************************
 *  @名称      DPS310_ReadRegister 
 *  @描述      读取单个寄存器的值，
 *  @参数      registerAddress - 需要读取的寄存器地址, dataSave - 读取到的寄存器值需要保存数组.
 *  @返回      如果读取成功，返回“POSITIVE”， 否则返回“NEGATIVE”
 *  @版本      v0.1
 *  @备注      
 *  @日期      2016-11-02 
 *  @举例      
******************************************************************************************/

uint8_t DPS310_ReadRegister( uint8_t registerAddress, uint8_t *dataSave)
{
  return SimulatedI2C_ByteRead(dps310_i2c, DPS310_DeviceAddress, registerAddress, dataSave);	
}

/******************************************************************************************
 *  @名称      DPS310_ReadRegisters 
 *  @描述      读取多个寄存器的值，
 *  @参数      dataSave - 读取的寄存器值保存数组， registerAddress - 需要读取的寄存器地址
 *  @返回      如果读取成功，返回“POSITIVE”， 否则返回“NEGATIVE”
 *  @版本      v0.1
 *  @备注      
 *  @日期      2016-11-02 
 *  @举例      
******************************************************************************************/

uint8_t DPS310_ReadRegisters(uint8_t startAddress, uint8_t dataCount, uint8_t * dataBuff)
{
  return SimulatedI2C_Read(dps310_i2c, DPS310_DeviceAddress, startAddress, dataBuff, dataCount);		
}
/******************************************************************************************
 *  @名称      DPS310_WriteRegister 
 *  @描述      写单个寄存器地址
 *  @参数      registerAddress - 需要写入的寄存器地址, dataSave - 需要写入的数值.
 *  @返回      如果读取成功，返回“POSITIVE”， 否则返回“NEGATIVE”
 *  @版本      v0.1
 *  @备注      
 *  @日期      2016-11-02 
 *  @举例      
******************************************************************************************/

uint8_t DPS310_WriteRegister(uint8_t registerAddress, uint8_t dataBuff)
{
  return SimulatedI2C_ByteWrite(dps310_i2c, DPS310_DeviceAddress, registerAddress, dataBuff);			
}

/******************************************************************************************
 *  @名称      DPS310_Config 
 *  @描述      DPS310内部设置
 *  @参数      configNumber - 设置序号， 根据设置序号选择相应的配置 
 *  @返回      无
 *  @版本      v0.1
 *  @备注      
 *  @日期      2016-11-02 
 *  @举例      
******************************************************************************************/

void DPS310_Config(DPS310_config configNumber)
{
  uint8_t temp[3];
  DPS310_ReadRegisters(DPS310_StartAddress, 3, temp);  // 为什么需要有这一步，下面的设置才正确。
  
  if(configNumber == CONFIG_1)
  {
    DPS310_WriteRegister(PRS_CFG, 0x35);   
    Delay_ms(100);  
    DPS310_WriteRegister(TMP_CFG, 0xA0);   
    Delay_ms(100);  
    DPS310_WriteRegister(CFG_REG, 0x04); 
    Delay_ms(100); 
  }
  else if(configNumber == CONFIG_2)
  {
    DPS310_WriteRegister(PRS_CFG, 0x26);   
    Delay_ms(100);  
    DPS310_WriteRegister(TMP_CFG, 0xA0);   
    Delay_ms(100);  
    DPS310_WriteRegister(CFG_REG, 0x04); 
    Delay_ms(100); 
  }
}

/******************************************************************************************
 *  @名称      DPS310_Get_Coef
 *  @描述      读取DPS310内部相关系数
 *  @参数      无
 *  @返回      以结构体方式返回系数
 *  @版本      v0.1
 *  @备注      
 *  @日期      2016-11-02 
 *  @举例       
******************************************************************************************/
struct_DPS310_coef DPS310_Get_Coef()
{
  uint8_t coefBuff[COEF_COUNT];
  struct_DPS310_coef dataSave;  // 创建数据实例
  DPS310_ReadRegisters(COEF, COEF_COUNT, coefBuff);     // 从计算系数中读取N个数
  
  dataSave.C0 = coefBuff[0];
  dataSave.C0 = (dataSave.C0 << 4) + (coefBuff[1] >> 4); 
  if(dataSave.C0 > 2047) dataSave.C0 = dataSave.C0 - 4096; 
  
  dataSave.C1 = coefBuff[1];
  dataSave.C1 = ((dataSave.C1 & 0x0F) << 8) + coefBuff[2];  
  if(dataSave.C1 > 2047) dataSave.C1 = dataSave.C1 - 4096; 
  
  dataSave.C00 = coefBuff[3];
  dataSave.C00 = (dataSave.C00 << 8) + coefBuff[4];
  dataSave.C00 = (dataSave.C00 << 4) + (coefBuff[5] >> 4);
  if(dataSave.C00>524287) dataSave.C00 = dataSave.C00 - 1048576;
  
  dataSave.C10 = coefBuff[5] & 0x0F;
  dataSave.C10 = (dataSave.C10 << 8) + coefBuff[6];
  dataSave.C10 = (dataSave.C10 << 8) + coefBuff[7];
  if(dataSave.C10>524287) dataSave.C10 = dataSave.C10 -1048576;
  
  dataSave.C01 = coefBuff[8];
  dataSave.C01 = (dataSave.C01 << 8) + coefBuff[9];
  if(dataSave.C01>32767) dataSave.C01 = dataSave.C01- 65536;
  
  dataSave.C11 = coefBuff[10];
  dataSave.C11 = (dataSave.C11 << 8) + coefBuff[11];
  if(dataSave.C11>32767) dataSave.C11 =  dataSave.C11-65536;
  
  dataSave.C20 = coefBuff[12];
  dataSave.C20 = (dataSave.C20 << 8) + coefBuff[13];
  if(dataSave.C20>32767) dataSave.C20 = dataSave.C20-65536;
  
  dataSave.C21 = coefBuff[14];
  dataSave.C21 = (dataSave.C21 << 8) + coefBuff[15];
  if(dataSave.C21>32767) dataSave.C21 = dataSave.C21-65536;
  
  dataSave.C30 = coefBuff[16];
  dataSave.C30 = (dataSave.C30 << 8) + coefBuff[17];
  if(dataSave.C30>32767) dataSave.C30 = dataSave.C30-65536 ;
  
  return dataSave;
}

/******************************************************************************************
 *  @名称      DPS310_Get_Pressure
 *  @描述      读取压力
 *  @参数      coef - 读到的系数（结构体），dataSave - 读取到的压力保存数组
 *  @返回      无
 *  @版本      v0.1
 *  @备注      
 *  @日期      2016-11-02 
 *  @举例      
******************************************************************************************/
void DPS310_Get_Pressure(struct_DPS310_coef coef, uint8_t* dataSave )
{
  uint8_t dataReady[1] = {0};
  uint8_t PRS_buff[3] = {0, 0, 0};
 
  long long int Pressure = 0; 
  long long int Final_Pressure = 0;
  long long int Pressure_1 = 0; 
  long long int Pressure_2 = 0; 
  long long int Pressure_3 = 0; 
  long long int Pressure_4 = 0; 
  long long int Pressure_5 = 0; 
  long long int Pressure_6 = 0; 
  long long int Pressure_7 = 0; 
  long long int P_raw = 0;
  long long int T_raw = 0;
  
  DPS310_WriteRegister(MEAS_CFG, 0x01); 
  while( !(dataReady[0] & 0x10))
  {
    DPS310_ReadRegisters(MEAS_CFG,1,dataReady);
    Delay_ms(50);
  }
  DPS310_ReadRegisters(PRS_STARTADDRESS, PRS_COUNT, PRS_buff); 
    
  Pressure = PRS_buff[0];
  Pressure = (Pressure << 8) + PRS_buff[1];
  Pressure = (Pressure << 8) + PRS_buff[2];
  if(Pressure > 8388607) P_raw = 0 - (16777216 -Pressure) ;
  
  P_raw = P_raw * 1000000  / 516096;
  
  Pressure_1 =  coef.C00 * 100; 
  Pressure_2 =  P_raw * coef.C10 / 10000; 
  Pressure_3 = (P_raw  ) * (P_raw ) * coef.C20 / 10000000000; 
  Pressure_4 = (P_raw /100) * (P_raw /100) * (P_raw /100) * coef.C30 / 10000000000; 
  Pressure_5 = (T_raw) * coef.C01 / 10000; 
  Pressure_6 = (T_raw ) * (P_raw ) * coef.C11/10000000000; 
  Pressure_7 = (T_raw ) * (P_raw ) * (P_raw ) * coef.C21/10000000000000000; 
  Final_Pressure = Pressure_1 + Pressure_2 +Pressure_3 +Pressure_4 +Pressure_5 +Pressure_6 +Pressure_7; 
  
  *(dataSave + 0) = ((Final_Pressure / 10000000) << 4) + Final_Pressure % 10000000 / 1000000;
  *(dataSave + 1) = ((Final_Pressure % 10000000 % 1000000 / 100000 )<<4) + Final_Pressure % 10000000 % 1000000% 100000 / 10000;
  *(dataSave + 2) = ((Final_Pressure % 10000000 % 1000000 % 100000 % 10000 / 1000 )<<4) + Final_Pressure % 1000 / 100;
}