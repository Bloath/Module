/* Includes ----------------------------------------------------------*/
#include "Module/Module.h"
#include "DPS310.h"

/* typedef -----------------------------------------------------------*/
/* define ------------------------------------------------------------*/
/* macro -------------------------------------------------------------*/
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/
int32_t Dps310_GetCoef(struct Dps310Struct *dps310);
int DPS310WriteAndCheck(struct Dps310Struct *dps310, uint8_t registerAddress, uint8_t data);
void Dps310_ClearFIFO(struct Dps310Struct *dps310);
uint32_t Dps310_SelectOverSampleFactor(uint8_t config);
uint32_t Dps310_GetOrigin(struct Dps310Struct *dps310, uint8_t startAddress);
void Dps310_DataConvert(struct Dps310Struct *dps310, uint32_t pressureOrigin, uint32_t temperatureOrigin, uint32_t *pressure, int16_t *temperature);
int Dps310_StartSingleConvert(struct Dps310Struct *dps310, uint8_t measureSelection);
int Dps310_IsSingleReadFinish(struct Dps310Struct *dps310, uint8_t measureSelection);

/*********************************************************************************************
 
 * name: 	Dps310_Init
 * param: 	dps310：dps310实例
            *coef：the coef of dsp310
            config: h->l pres_cfg  tmp_cfg meas_cfg, cfg_reg
 * return: 	0: success
 
  ********************************************************************************************/
int Dps310_Init(struct Dps310Struct *dps310, uint32_t config)
{
    uint8_t coefsource = 0,meafConfig = 0;
	int result = 0;

    result = SoftI2c_Read(dps310->i2c, dps310->devAddress, PRODUCT_ID, &coefsource, 1);
    result = SoftI2c_Read(dps310->i2c, dps310->devAddress, 0x1C, &coefsource, 1);
    
    /* 复位模块 */
    SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  0x0C, 0x09);
    Delay_ms(10);
    
    if((result = SoftI2c_Read(dps310->i2c, dps310->devAddress, TMP_COEF_SRCE, &coefsource, 1)) < 0) // TMP_COEF_SRCE位为1（使用MEMS），否则(使用ASIC）.
	{	return result;	}
    coefsource &= 0x80;

    uint8_t pressConfig = (uint8_t)(config >> 24);
    uint8_t tempConfig = (uint8_t)(config >> 16);
    
    /*** 2017年12月份新采购的芯片，需要添加下面几句操作，添加该操作之后对以前的芯片无影响 BEGIN***/
    SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  0x0E, 0xA5);
    SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  0x0F, 0x96);
    SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  0x62, 0x02);
    SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  0x0E, 0x00);
    SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  0x0F, 0x00);
    /*** 2017年12月份新采购的芯片，需要添加下面几句操作，添加该操作之后对以前的芯片无影响 END***/

    if ((pressConfig & 0x0F) > 3)
    {	meafConfig |= (1 << 2);	}
    if ((tempConfig & 0x0F) > 3)
    {	meafConfig |= (1 << 3);	}
    
    if((result = Dps310_GetCoef(dps310)) < 0)
    {   return result;  }
    
    result += SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  PRS_CFG, pressConfig);
    result += SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  TMP_CFG, tempConfig | coefsource);
	result += SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  CFG_REG, (uint8_t)config | meafConfig);        
    result += SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  MEAS_CFG, (uint8_t)(config >> 8));    
    if(result < 0)
	{	return result;	}  
    
    /* 如果为后台模式，则清除fifo */
    if(((uint8_t)(config >> 8) & 0x07) >= 5)
    {   Dps310_ClearFIFO(dps310);   }

    dps310->coef.pressFactor = Dps310_SelectOverSampleFactor(pressConfig & 0x0F);
    dps310->coef.tempFactor = Dps310_SelectOverSampleFactor(tempConfig & 0x0F);

    return 0;
}

/*********************************************************************************************

  * @brief  Dps310_GetCoef
  * @dcpt   获取芯片系数
  * @param  dps310：dps310实例
  * @param  
  * @retval -1：读超时
            0：成功
  * @remark 

  ********************************************************************************************/
int32_t Dps310_GetCoef(struct Dps310Struct *dps310)
{
    uint8_t coefBuff[COEF_COUNT];

    uint8_t SensorStatus = 0;
    SoftI2c_Read(dps310->i2c, dps310->devAddress, MEAS_CFG, &SensorStatus, 1);
    uint8_t ReadCoefTimes = 0;
    SoftI2c_Read(dps310->i2c, dps310->devAddress, MEAS_CFG, &SensorStatus, 1);

    while (((SensorStatus & COEF_AVALIABLE) != COEF_AVALIABLE) && (ReadCoefTimes < 100)) // 判断COEF值是否可读
    {
        Delay_ms(5);
        ReadCoefTimes++;
        SoftI2c_Read(dps310->i2c, dps310->devAddress, MEAS_CFG, &SensorStatus, 1);
    }
    if (ReadCoefTimes == 100)
    {
        return -1;
    }

    SoftI2c_Read(dps310->i2c, dps310->devAddress, COEF, coefBuff, COEF_COUNT);

    dps310->coef.C0 = ((int32_t)coefBuff[0] << 4) | ((int32_t)coefBuff[1] >> 4);                                    // 12位
    dps310->coef.C1 = (((int32_t)coefBuff[1] & 0x0F) << 8) | coefBuff[2];                                           // 12位
    dps310->coef.C00 = ((int32_t)coefBuff[3] << 12) | ((int32_t)coefBuff[4] << 4) | ((int32_t)coefBuff[5] >> 4);    // 20位
    dps310->coef.C10 = (((int32_t)coefBuff[5] & 0x0FL) << 16) | ((int32_t)coefBuff[6] << 8) | (int32_t)coefBuff[7]; // 20位
    dps310->coef.C01 = ((int32_t)coefBuff[8] << 8) | coefBuff[9];                                                   // 16位
    dps310->coef.C11 = ((int32_t)coefBuff[10] << 8) | coefBuff[11];                                                 // 16位
    dps310->coef.C20 = ((int32_t)coefBuff[12] << 8) | coefBuff[13];                                                 // 16位
    dps310->coef.C21 = ((int32_t)coefBuff[14] << 8) | coefBuff[15];                                                 // 16位
    dps310->coef.C30 = ((int32_t)coefBuff[16] << 8) | coefBuff[17];                                                 // 16位

    if (dps310->coef.C0 & 0xFFFFF800)
        dps310->coef.C0 |= 0xFFFFF000;
    if (dps310->coef.C1 & 0xFFFFF800)
        dps310->coef.C1 |= 0xFFFFF000;
    if (dps310->coef.C00 & 0xFFF80000)
        dps310->coef.C00 |= 0xFFF00000;
    if (dps310->coef.C10 & 0xFFF80000)
        dps310->coef.C10 |= 0xFFF00000;
    if (dps310->coef.C01 & 0xFFFF8000)
        dps310->coef.C01 |= 0xFFFF0000;
    if (dps310->coef.C11 & 0xFFFF8000)
        dps310->coef.C11 |= 0xFFFF0000;
    if (dps310->coef.C20 & 0xFFFF8000)
        dps310->coef.C20 |= 0xFFFF0000;
    if (dps310->coef.C21 & 0xFFFF8000)
        dps310->coef.C21 |= 0xFFFF0000;
    if (dps310->coef.C30 & 0xFFFF8000)
        dps310->coef.C30 |= 0xFFFF0000;
    return 0;
}
/*********************************************************************************************

  * @brief  Dps310_DataConvert 数据换算
  * @param  dps310：dps310实例
            pressureOrigin 压力原始
            temperatureOrigin 温度原始
            pressure：压力转换结果填充地址
            temperature：压力转换结果填充地址
  * @param  
  * @retval 

  ********************************************************************************************/
void Dps310_DataConvert(struct Dps310Struct *dps310, uint32_t pressureOrigin, uint32_t temperatureOrigin, uint32_t *pressure, int16_t *temperature)
{
    double Praw_sc = (double)((int32_t)pressureOrigin) / dps310->coef.pressFactor;   // 这个数值是根据oversampling rate来确定的
    double Traw_sc = (double)((int32_t)temperatureOrigin) / dps310->coef.tempFactor; // 这个数值是根据oversampling rate来确定的

    *temperature = (int16_t)((dps310->coef.C0 / 2 + Traw_sc * dps310->coef.C1) * 100);

    *pressure = (uint32_t)((dps310->coef.C00 + (((dps310->coef.C20 + Praw_sc * dps310->coef.C30) * Praw_sc + dps310->coef.C10) * Praw_sc) + ((Traw_sc * dps310->coef.C01)) + ((dps310->coef.C11 + Praw_sc * dps310->coef.C21) * Traw_sc * Praw_sc)) * 10);
}
/*********************************************************************************************

  * @brief  Dps310_SyncRead
  * @dcpt   读取单个数据,同步读法
  * @param  dps310：dps310实例
  * @param  
  * @retval -1：读取压力超时
            -2：读取温度超时
            0：成功
  * @remark 通过单次启动转换，读取一次温度与压力

  ********************************************************************************************/
int Dps310_SyncRead(struct Dps310Struct *dps310, uint32_t timeOut)
{
    uint32_t __time = SYSTIME;

    /* 启动压力转换 */
    Dps310_StartSingleConvert(dps310, DPS310_MEASURE_PRESSURE);
    do
    {
        /* 超时判断 */
        if((__time + timeOut) < SYSTIME)
        {   return -1;  }

        if (Dps310_IsSingleReadFinish(dps310, DPS310_MEASURE_PRESSURE))
        {
            dps310->data->pressureOrigin = Dps310_GetOrigin(dps310, PRS_STARTADDRESS);
            break;
        }
    } while (1);

    /* 启动温度转换 */
    Dps310_StartSingleConvert(dps310, DPS310_MEASURE_TEMPERATURE);
    do
    {
        /* 超时判断 */
        if((__time + timeOut) < SYSTIME)
        {   return -1;  }

        if (Dps310_IsSingleReadFinish(dps310, DPS310_MEASURE_TEMPERATURE))
        {
            dps310->data->temperatureOrigin = Dps310_GetOrigin(dps310, TMP_STARTADDRESS);
            break;
        }
    } while (1);

    Dps310_DataConvert(dps310, dps310->data->pressureOrigin, dps310->data->temperatureOrigin, &(dps310->data->pressureEx1), &(dps310->data->temperatureEx2));
    
    if(dps310->CallBack_ReadData != NULL)
    {   dps310->CallBack_ReadData(dps310);    }
    
    return 0;
}
/*********************************************************************************************

  * @brief  Dps310_AsynReadStart 异步读取启动
  * @dcpt   对压力相关传感器的初始化
  * @param  dps310：dps310实例
  * @retval 
  * @remark 每秒采集一次数据

  ********************************************************************************************/
void Dps310_AsynReadStart(struct Dps310Struct *dps310)
{
    PROCESS_CHANGE(dps310->data->_process, Process_Start);
}
/*********************************************************************************************

  * @brief  Dps310_AsynRead 异步获取
  * @dcpt   对压力相关传感器的初始化
  * @param  dps310：dps310实例
            timeOut：超时时间
  * @retval -1：失败
            0： 成功
  * @remark 每秒采集一次数据

  ********************************************************************************************/
void Dps310_AsynRead(struct Dps310Struct *dps310, uint32_t timeOut)
{
    int result = 0;
  
    switch (dps310->data->_process.current)
    {
    case Process_Idle:
        break;

    case Process_Start: // 启动压力转换
        Dps310_StartSingleConvert(dps310, DPS310_MEASURE_PRESSURE);
        PROCESS_CHANGE_WITH_TIME(dps310->data->_process, Process_Wait, SYSTIME);
        break;

    case Process_Wait:      // 等待压力转换完成
        result = Dps310_IsSingleReadFinish(dps310, DPS310_MEASURE_PRESSURE);
        if (result == 1)
        {
            dps310->data->pressureOrigin = Dps310_GetOrigin(dps310, PRS_STARTADDRESS);         // 获取压力初始值
            Dps310_StartSingleConvert(dps310, DPS310_MEASURE_TEMPERATURE);                    // 启动温度转换
            PROCESS_CHANGE_WITH_TIME(dps310->data->_process, Process_LongWait, SYSTIME);      // 切换到等到温度转换等待
        }
        else if(result < 0 || (dps310->data->_process.__time + timeOut) < SYSTIME)
        {   PROCESS_CHANGE_WITH_TIME(dps310->data->_process, Process_Lock, SYSTIME); }
        break;

    case Process_LongWait:  // 等待温度转换完成
        result = Dps310_IsSingleReadFinish(dps310, DPS310_MEASURE_TEMPERATURE);
        if (result == 1)
        {
            dps310->data->temperatureOrigin = Dps310_GetOrigin(dps310, TMP_STARTADDRESS);         // 获取压力初始值
            PROCESS_CHANGE_WITH_TIME(dps310->data->_process, Process_Finish, SYSTIME);            // 切换到完成
        }
        else if(result < 0 || (dps310->data->_process.__time + timeOut) < SYSTIME)
        {   PROCESS_CHANGE_WITH_TIME(dps310->data->_process, Process_Lock, SYSTIME); }
        break;

    case Process_Finish:    // 完成操作
         Dps310_DataConvert(dps310, dps310->data->pressureOrigin, dps310->data->temperatureOrigin, &(dps310->data->pressureEx1), &(dps310->data->temperatureEx2));
        if(dps310->CallBack_ReadData != NULL)
        {   dps310->CallBack_ReadData(dps310);    }
        PROCESS_CHANGE_WITH_TIME(dps310->data->_process, Process_Idle, SYSTIME);
        break;
    }
}
/*********************************************************************************************

  * @brief  Dps310_MultiRead 多个获取
  * @dcpt   
  * @param  dps310：dps310实例
  * @retval -1：失败
            0： 成功
  * @remark 每秒采集一次数据

  ********************************************************************************************/
int Dps310_BackgroudRead(struct Dps310Struct *dps310)
{
    dps310->fifo->oCount = 0;
    uint8_t status = 0;
    uint32_t temp32u = 0;
    int result = 0;
    int32_t temp32;
    
    do
    {
        if((result = SoftI2c_Read(dps310->i2c, dps310->devAddress, FIFO_STS, &status, 1)) != 0)
         {   return -1;  }
        
        if(FLAG_IS_CLR(status, (1<<0)))
        {
            // 获取数据，最后一位为1，则为压力，为0为温度
            temp32u = Dps310_GetOrigin(dps310, PRS_STARTADDRESS);
            
            if(temp32u == 0)
            {   return -3;  }
            
            dps310->fifo->origin[dps310->fifo->oCount++] = temp32u;
            
            if(dps310->fifo->oCount == 32)
            {   break; }
        }
        else
        {   break;   }
    }while(1);
    
    dps310->fifo->pCount = 0;
    for(uint8_t i=0; i<dps310->fifo->oCount; i++)
    {   
        if((dps310->fifo->origin[i] & (1<<0)) != 0)
        {
            Dps310_DataConvert(dps310, 
                               dps310->fifo->origin[i], 
                               dps310->fifo->lastTemperatureOrigin, 
                               &temp32,
                               &(dps310->fifo->temperatureEx2));  // 数据转换
            if(temp32 < 12000000)
            {   dps310->fifo->pressureEx1List[dps310->fifo->pCount++] = temp32; }
        }
        else
        {   dps310->fifo->lastTemperatureOrigin = dps310->fifo->origin[i];  }
    }
    
    /* 数据处理回调，处理完成后清空oCount */
    if(dps310->CallBack_ReadFifo != NULL)
    {   dps310->CallBack_ReadFifo(dps310);  }
    
    return 0;
}
/******************************************************************************************
 *  @brief     DPS310WriteAndCheck
 *  @dcpt      写单个寄存器，然后检查是否写入成功
 *  @param     dps310：dps310实例
               registerAddress - 需要读取的寄存器地址, 
               data - 读取到的寄存器值需要保存数组.
 *  @retval    -1 失败
               0：成功
 *  @remark      
 *  @eg      
******************************************************************************************/
int DPS310WriteAndCheck(struct Dps310Struct *dps310, uint8_t registerAddress, uint8_t data)
{
    uint8_t valueCheck = 0;
    SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress, registerAddress, data);
    Delay_ms(1);
    SoftI2c_Read(dps310->i2c, dps310->devAddress, registerAddress, &valueCheck, 1);

    return (valueCheck == data) ? 0 : -1;
}
/******************************************************************************************
 *  @brief      Dps310_ClearFIFO 
 *  @dcpt       清空FIFO缓冲区，每次读取完FIFO后，需要执行该函数，将原来的数据清除掉。
 *  @param      dps310：dps310实例
 *  @retval    
 *  @remark     
 *  @eg      
******************************************************************************************/

void Dps310_ClearFIFO(struct Dps310Struct *dps310)
{
    uint8_t fifo_status = 0;
    SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  FIFO_FLUSH, 0x80);      // 清空FIFO缓冲区
    SoftI2c_Read(dps310->i2c, dps310->devAddress, FIFO_STS, &fifo_status, 1); // 读取缓冲区状态，确认缓冲区是否为空，进而判断是否成功清空
}
/******************************************************************************************
 *  @brief      Dps310_SelectOverSampleFactor
 *  @dcpt       根据param选择过采样率系数
 *  @param      config：配置
 *  @retval     过采样率系数
                0：参数不正确
 *  @remark     
 *  @eg         
******************************************************************************************/
uint32_t Dps310_SelectOverSampleFactor(uint8_t config)
{
    switch (config)
    {
    case 0x00:
        return 524288;
    case 0x01:
        return 1572864;
    case 0x02:
        return 3670016;
    case 0x03:
        return 7864320;
    case 0x04:
        return 253952;
    case 0x05:
        return 516096;
    case 0x06:
        return 1040384;
    case 0x07:
        return 2088960;
    }

    return 0;
}
/******************************************************************************************
 *  @brief     Dps310_GetOrigin
 *  @dcpt      读取原始值用于后面的转换
 *  @param     dps310：dps310实例
               startAddress：起始地址，可选PRS_STARTADDRESS， TMP_STARTADDRESS
 *  @retval    4字节初始值
 *  @remark     
 *  @eg      
******************************************************************************************/
uint32_t Dps310_GetOrigin(struct Dps310Struct *dps310, uint8_t startAddress)
{
    uint32_t temp32u = 0;
    uint8_t *p = (uint8_t *)(&temp32u);

    SoftI2c_Read(dps310->i2c, dps310->devAddress, startAddress, p, 3); // 读取数据

    // 转换位置，从 2 1 0 空 => 0 1 2 空
    p[3] = p[0];
    p[0] = p[2];
    p[2] = p[3];
    p[3] = 0;

    return ((temp32u & 0x00800000) != 0) ? (temp32u | 0xFF000000) : temp32u;
}
/*********************************************************************************************

  * @brief  Dps310_StartSingleConvert
  * @dcpt  读取单个数据模式下，用于启动单次转换
  * @param  dps310：dps310实例
  * @param  measureSelection: 
                DPS310_MEASURE_TEMPERATURE
                DPS310_MEASURE_PRESSURE
  * @retval 
  * @remark 多个压力传感器同时使用，则通过该函数同时启动，减少整体时间（取数据为阻塞）

  ********************************************************************************************/
int Dps310_StartSingleConvert(struct Dps310Struct *dps310, uint8_t measureSelection)
{
   return SoftI2c_SingleWrite(dps310->i2c, dps310->devAddress,  MEAS_CFG, measureSelection);
}
/*********************************************************************************************

  * @brief  Dps310_IsSingleReadFinish
  * @dcpt   判断是否转换完成
  * @param  dps310：dps310实例
  * @param  measureSelection: 
                DPS310_MEASURE_TEMPERATURE
                DPS310_MEASURE_PRESSURE
  * @retval true：完成
            
  * @remark 判断结果是否完成

  ********************************************************************************************/
int Dps310_IsSingleReadFinish(struct Dps310Struct *dps310, uint8_t measureSelection)
{
    uint8_t temp8u;
    if(SoftI2c_Read(dps310->i2c, dps310->devAddress, MEAS_CFG, &temp8u, 1) < 0)
    {   return -1;  }
    return ((temp8u & (measureSelection << 4)) != 0)? 1:0;
}