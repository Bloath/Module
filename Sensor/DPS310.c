/*
 * @file       DPS310.c
 * @brief      DPS310ѹ��������
 * @author     dong
 * @version    v0.1
 * @date       2016-11-03
 */

#include "MKL27Z4.h" 
#include "KL27_Config.h"
#include "delay.h" 
#include "IIC.h" 
#include "DPS310.h" 

/******************************************************************************************
 *  @����      DPS310_ReadRegister 
 *  @����      ��ȡ�����Ĵ�����ֵ��
 *  @����      registerAddress - ��Ҫ��ȡ�ļĴ�����ַ, dataSave - ��ȡ���ļĴ���ֵ��Ҫ��������.
 *  @����      �����ȡ�ɹ������ء�POSITIVE���� ���򷵻ء�NEGATIVE��
 *  @�汾      v0.1
 *  @��ע      
 *  @����      2016-11-02 
 *  @����      
******************************************************************************************/

U8 DPS310_ReadRegister( U8 registerAddress, U8 *dataSave)
{
  start();
  write_byte(DPS310_DeviceAddress);
  if(!check()) return NEGATIVE;
  write_byte(registerAddress); 
  if(!check()) return NEGATIVE;
  start();
  write_byte(DPS310_DeviceAddress | 0x01);
  if(!check()) return NEGATIVE;
  *(dataSave+0) = read_byte();
  return POSITIVE;	
}

/******************************************************************************************
 *  @����      DPS310_ReadRegisters 
 *  @����      ��ȡ����Ĵ�����ֵ��
 *  @����      dataSave - ��ȡ�ļĴ���ֵ�������飬 registerAddress - ��Ҫ��ȡ�ļĴ�����ַ
 *  @����      �����ȡ�ɹ������ء�POSITIVE���� ���򷵻ء�NEGATIVE��
 *  @�汾      v0.1
 *  @��ע      
 *  @����      2016-11-02 
 *  @����      
******************************************************************************************/

U8 DPS310_ReadRegisters(U8 startAddress, U8 dataCount, U8 * dataBuff)
{
  start();
  write_byte(DPS310_DeviceAddress);
  if(!check()) return NEGATIVE;
  write_byte(startAddress); 
  if(!check()) return NEGATIVE;
  start(); 
  write_byte(DPS310_DeviceAddress | 0x01);
  if(!check()) return NEGATIVE;
  read_bytes(dataBuff, dataCount); 
  return POSITIVE;		
}
/******************************************************************************************
 *  @����      DPS310_WriteRegister 
 *  @����      д�����Ĵ�����ַ
 *  @����      registerAddress - ��Ҫд��ļĴ�����ַ, dataSave - ��Ҫд�����ֵ.
 *  @����      �����ȡ�ɹ������ء�POSITIVE���� ���򷵻ء�NEGATIVE��
 *  @�汾      v0.1
 *  @��ע      
 *  @����      2016-11-02 
 *  @����      
******************************************************************************************/

U8 DPS310_WriteRegister(U8 registerAddress, U8 dataBuff)
{
  start();
  write_byte(DPS310_DeviceAddress);
  if(!check()) return NEGATIVE;
  write_byte(registerAddress); 
  if(!check()) return NEGATIVE; 
  write_byte(dataBuff); 
  if(!check()) return NEGATIVE; 
  stop();
  return POSITIVE;		
}

/******************************************************************************************
 *  @����      DPS310_Config 
 *  @����      DPS310�ڲ�����
 *  @����      configNumber - ������ţ� �����������ѡ����Ӧ������ 
 *  @����      ��
 *  @�汾      v0.1
 *  @��ע      
 *  @����      2016-11-02 
 *  @����      
******************************************************************************************/

void DPS310_Config(DPS310_config configNumber)
{
  U8 temp[3];
  DPS310_ReadRegisters(DPS310_StartAddress, 3, temp);  // Ϊʲô��Ҫ����һ������������ò���ȷ��
  
  if(configNumber == CONFIG_1)
  {
    DPS310_WriteRegister(PRS_CFG, 0x35);   
    delay_ms(100);  
    DPS310_WriteRegister(TMP_CFG, 0xA0);   
    delay_ms(100);  
    DPS310_WriteRegister(CFG_REG, 0x04); 
    delay_ms(100); 
  }
  else if(configNumber == CONFIG_2)
  {
    DPS310_WriteRegister(PRS_CFG, 0x26);   
    delay_ms(100);  
    DPS310_WriteRegister(TMP_CFG, 0xA0);   
    delay_ms(100);  
    DPS310_WriteRegister(CFG_REG, 0x04); 
    delay_ms(100); 
  }
}

/******************************************************************************************
 *  @����      DPS310_Get_Coef
 *  @����      ��ȡDPS310�ڲ����ϵ��
 *  @����      ��
 *  @����      �Խṹ�巽ʽ����ϵ��
 *  @�汾      v0.1
 *  @��ע      
 *  @����      2016-11-02 
 *  @����       
******************************************************************************************/
struct_DPS310_coef DPS310_Get_Coef()
{
  U8 coefBuff[COEF_COUNT];
  struct_DPS310_coef dataSave;
  DPS310_ReadRegisters(COEF, COEF_COUNT, coefBuff); 
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
 *  @����      DPS310_Get_Pressure
 *  @����      ��ȡѹ��
 *  @����      coef - ������ϵ�����ṹ�壩��dataSave - ��ȡ����ѹ����������
 *  @����      ��
 *  @�汾      v0.1
 *  @��ע      
 *  @����      2016-11-02 
 *  @����      
******************************************************************************************/
void DPS310_Get_Pressure(struct_DPS310_coef coef, U8* dataSave )
{
  U8 dataReady[1] = {0};
  U8 PRS_buff[3] = {0, 0, 0};
 
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
    delay_ms(50);
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