/* Includes ------------------------------------------------------------------*/
#include "hsppad14xa.h"
#include "Module/Module.h"

/* typedef -------------------------------------------------------------------*/
/* define --------------------------------------------------------------------*/
/* macro ---------------------------------------------------------------------*/
/* variables -----------------------------------------------------------------*/
/* function prototypes -------------------------------------------------------*/
/* functions -----------------------------------------------------------------*/
int Hsppad14xa_GetPressure(struct Hsppad14xaStruct *hsppad14xa, int32_t *pressure);
int Hsppad14xa_GetTemperature(struct Hsppad14xaStruct *hsppad14xa, int32_t *temperature);

/*********************************************************************************************

  * @brief  Hsppad14xa_Init
  * @param  hsppad14xa：传感器结构体
            config：配置 NULL CTL1 CTL2 AVCL 
  * @return 
  * @remark 

  ********************************************************************************************/
int Hsppad14xa_Init(struct Hsppad14xaStruct *hsppad14xa, uint32_t config)
{
    uint8_t temp8u = 0;
    int result = SoftI2c_Read(hsppad14xa->i2c, hsppad14xa->devAddress, HSPPAD14X_WIA, &temp8u, 1);
    
    /* 查看传感器是否正常 */
    if(result < 0 || temp8u != 0x49)
    {   return -1;  }
    
    /* 重启传感器 */
    SoftI2c_SingleWrite(hsppad14xa->i2c, hsppad14xa->devAddress, HSPPAD14X_ACTL2, 0x80);
    
    /* 读取id，查看系数 */
    result = SoftI2c_Read(hsppad14xa->i2c, hsppad14xa->devAddress, HSPPAD14X_PNUM, &temp8u, 1);
    switch(temp8u)
    {
    case 0xE0:
        hsppad14xa->coef = 2;
        break;
    case 0x31:
        hsppad14xa->coef = 6;
        break;
    }      
    
    /* 初始化芯片 */
    SoftI2c_SingleWrite(hsppad14xa->i2c, hsppad14xa->devAddress, HSPPAD14X_CTL1, (uint8_t)(config >> 16));
    SoftI2c_SingleWrite(hsppad14xa->i2c, hsppad14xa->devAddress, HSPPAD14X_AVCL, (uint8_t)(config));
    
    /* 如果为连续模式，则开启fifo */
    if(((uint8_t)(config >> 8) & 0x03) == 0x01)
    {
        SoftI2c_SingleWrite(hsppad14xa->i2c, hsppad14xa->devAddress, HSPPAD14X_FCTL, 0x90);
    }
    
    /* 启动芯片 */
    SoftI2c_SingleWrite(hsppad14xa->i2c, hsppad14xa->devAddress, HSPPAD14X_CTL2, (uint8_t)(config >> 8));
    
    return 0;
}
/*********************************************************************************************

  * @brief  Hsppad14xa_BackgroudRead
  * @param  hsppad14xa：传感器结构体
  * @return 
  * @remark 

  ********************************************************************************************/
int Hsppad14xa_BackgroudRead(struct Hsppad14xaStruct *hsppad14xa)
{
    uint8_t temp8u = 0;
    int i = 0;
    
    if(SoftI2c_Read(hsppad14xa->i2c, hsppad14xa->devAddress, HSPPAD14X_FFST, &temp8u, 1) != 0)
    {   return -1;  }
    hsppad14xa->fifo->pCount = temp8u & 0x1F;
    for(i=0; i<hsppad14xa->fifo->pCount; i++)
    {   Hsppad14xa_GetPressure(hsppad14xa, hsppad14xa->fifo->pressure + i);    }
    
    Hsppad14xa_GetTemperature(hsppad14xa, &(hsppad14xa->fifo->temperature));

    if(hsppad14xa->CallBack_ReadFifo != NULL)
    {   hsppad14xa->CallBack_ReadFifo(hsppad14xa);  }
    
    return temp8u;
}
/*********************************************************************************************

  * @brief  Hsppad14x_GetPressure
  * @param  hsppad14xa：传感器结构体
            pressure：读取数据的指针
  * @return 
  * @remark 

  ********************************************************************************************/
int Hsppad14xa_GetPressure(struct Hsppad14xaStruct *hsppad14xa, int32_t *pressure)
{
    *pressure = 0;
    int result = SoftI2c_Read(hsppad14xa->i2c, hsppad14xa->devAddress, HSPPAD14X_POUTL, (uint8_t *)pressure, 3);
    if(result < 0)
    {   return result;  }
    *pressure *= hsppad14xa->coef;
    return 0;
}
/*********************************************************************************************

  * @brief  Hsppad14x_GetTemperature
  * @param  hsppad14xa：传感器结构体
            pressure：读取数据的指针
  * @return 
  * @remark 

  ********************************************************************************************/
int Hsppad14xa_GetTemperature(struct Hsppad14xaStruct *hsppad14xa, int32_t *temperature)
{
    *temperature = 0;
    int result = SoftI2c_Read(hsppad14xa->i2c, hsppad14xa->devAddress, HSPPAD14X_TOUTL, (uint8_t *)temperature, 2);
    *temperature >>= 8;
    return result;
}