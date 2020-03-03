/* Includes ------------------------------------------------------------------*/
#include "Module/Module.h"
#include "SoftI2C.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define I2C_Delay() Delay_us(2)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name   : SoftI2c_Start
* Description     : I2C总线起始信号
* Parameter       : 
* Return          : 0：总线正常，产生起始信号
                    -1：经过9个时钟后总线依然有问题
                    -2：总线在主机发出起始信号之后无应答
*******************************************************************************/
int SoftI2c_Start(struct SoftI2cStruct *i2c)
{
    uint8_t i = 0;
    //拉高SCL,SDA准备
    SCL_SET(i2c);
    SDA_SET(i2c);
    I2C_Delay();

    //SDA未被拉高，说明总线忙
    if (!SDA_STATUS(i2c))
    {
        //产生9个时钟解除死锁状态
        for (i = 0; i < 9; i++)
        {
            SCL_CLR(i2c);
            I2C_Delay();
            SCL_SET(i2c);
            I2C_Delay();
        }
        //9个时钟后依然低，则说明总线有问题
        if (!SDA_STATUS(i2c))
        {   return -1;   }
    }

    SDA_CLR(i2c); //先拉低SDA
    I2C_Delay();
    SCL_CLR(i2c); //后拉低SCL，产生起始信号
    I2C_Delay();

    //SDA未被拉底，说明总线错误
    if (SDA_STATUS(i2c))
    {   return -2;   }

    return 0;
}

/*******************************************************************************
* Function Name   : SoftI2c_Stop
* Description     : I2C总线停止信号
* Parameter       : 
* Return          : None
*******************************************************************************/
void SoftI2c_Stop(struct SoftI2cStruct *i2c)
{
    //拉低SCL,sda
    SCL_CLR(i2c);
    SDA_CLR(i2c);
    I2C_Delay();

    SCL_SET(i2c); //先拉高SCL
    I2C_Delay();
    SDA_SET(i2c); //再拉高SDA，送出停止
    I2C_Delay();
}

/*******************************************************************************
* Function Name   : SoftI2c_Ack
* Description     : I2C总线的应答，在第九位的SCL的上升沿，判断SDA为高还是低
                    先设置高低再拉高SCL并恢复
* Parameter       : status：ENABLE：送出应答
                            DISENABLE：不送出应答
* Return          : None
*******************************************************************************/
void SoftI2c_Ack(struct SoftI2cStruct *i2c, uint8_t Status)
{

    if (Status) //在需要产生应答时，将SDA拉低
    {   SDA_CLR(i2c);  }
    else //不需要则拉高(接收最后一个字节)
    {   SDA_SET(i2c);  }
    I2C_Delay();

    SCL_SET(i2c); //先拉高SCL
    I2C_Delay();

    SCL_CLR(i2c); //拉低SCL
    I2C_Delay();

    SDA_SET(i2c); // PS：非常重要，在发送ACK之后要把总线抬高，因为是开漏，所以必须通过输出抬高后，芯片才能控制总线
    I2C_Delay();
}

/*******************************************************************************
* Function Name   : SoftI2c_WaitAck
* Description     : I2C总线读取时等待从设备应答
* Parameter       : 
* Return          : 0：成功接收应答信号
                    -1：接收应答信号失败
*******************************************************************************/
int SoftI2c_WaitAck(struct SoftI2cStruct *i2c)
{
    int Status;
    SDA_SET(i2c); //拉高SDA，等待输入
    I2C_Delay();
    SCL_SET(i2c); //先拉高SCL
    I2C_Delay();

    if (!SDA_STATUS(i2c))
    {   Status = 0; }
    else
    {   Status = -1; }

    SCL_CLR(i2c);
    I2C_Delay();
    return Status;
}

/*******************************************************************************
* Function Name   : SoftI2c_SendByte
* Description     : I2C总线传输单字节数据
* Parameter       : data：8位数据
* Return          : None
*******************************************************************************/
void SoftI2c_SendByte(struct SoftI2cStruct *i2c, uint8_t data)
{
    uint8_t i = 0;

    for (i = 0; i < 8; i++)
    {
        SCL_CLR(i2c);
        I2C_Delay();

        if (data & (1 << 7))
        {   SDA_SET(i2c);  }
        else
        {   SDA_CLR(i2c);  }
        I2C_Delay();
        data <<= 1;

        SCL_SET(i2c);
        I2C_Delay();
    }
    SCL_CLR(i2c);
    I2C_Delay();
}

/*******************************************************************************
* Function Name   : SoftI2c_ReceiveByte
* Description     : I2C总线接收单字节数据
* Parameter       : 
* Return          : None读取的8位数据
*******************************************************************************/
uint8_t SoftI2c_ReceiveByte(struct SoftI2cStruct *i2c)
{
    uint8_t i = 0, Temp;

    for (i = 0; i < 8; i++)
    {
        SCL_CLR(i2c);
        I2C_Delay();
        SCL_SET(i2c);
        I2C_Delay();

        Temp <<= 1;
        if (SDA_STATUS(i2c))
        {   Temp |= 1;  }
        I2C_Delay();
    }
    SCL_CLR(i2c);
    I2C_Delay();

    return Temp;
}
/*******************************************************************************
* Function Name   : SoftI2c_WriteByte
* Description     : I2C总线发送数据
* Parameter       : data：   要发送的字节数据
* Return          : 0：操作成功
                    -1：总线死锁无法解决
*******************************************************************************/
int SoftI2c_Send(struct SoftI2cStruct *i2c, uint8_t data)
{
    uint8_t status = 0;

    // 如果只发低字节的话，则只发一次就结束
    SoftI2c_SendByte(i2c, (uint8_t)data);   

    status = SoftI2c_WaitAck(i2c);
    if (status)
    {
        SoftI2c_Stop(i2c);
        return status;
    }
    
    return 0;
}
/*******************************************************************************
* Function Name   : SoftI2c_Revice
* Description     : I2C总线接收数据
* Parameter       : isLast：是否为最后一个数据（最后一个字节不需要发ack）
* Return          : 
*******************************************************************************/
uint8_t SoftI2c_Receive(struct SoftI2cStruct *i2c, bool isLast)
{
    uint8_t data = SoftI2c_ReceiveByte(i2c);

    if (isLast == true)
    {   SoftI2c_Ack(i2c, 0); }
    else
    {   SoftI2c_Ack(i2c, 1); }

    return data;
}
/*******************************************************************************

* Description     : 软件单独写
* Parameter       : i2cPinStruct：软件的IO结构体
                    deviceAddr：设备地址，7位地址，无需左移，函数内部已经左移并加入了读写位
                    registerAddr：寄存器地址
                    data：   要发送的字节数据
* Return          : 0：操作成功
                    1：失败
*******************************************************************************/
int SoftI2c_SingleWrite(struct SoftI2cStruct *i2c, uint8_t deviceAddr, uint8_t registerAddr, uint8_t data)
{
    // 1：起始信号
    if (SoftI2c_Start(i2c) != 0)
    {   return -1;   }

    // 2：写入设备地址
    if (SoftI2c_Send(i2c, (uint8_t)(deviceAddr << 1)) != 0)
    {   return -2;   }

    // 3：写入寄存器地址
    if (SoftI2c_Send(i2c, registerAddr) != 0)
    {   return -3;   }

    // 4：写入数据
    if (SoftI2c_Send(i2c, data) != 0)
    {   return -4;   }

    // 5：结束信号
    SoftI2c_Stop(i2c);

    return 0;
}

/*******************************************************************************
* Function Name   : SoftI2c_Write
* Description     : 软件写入数据
* Parameter       : deviceAddr：设备地址，7位地址，无需左移，函数内部已经左移并加入了读写位
                    registerAddr：寄存器地址
                    *data：  要发送的数据指针
                    len：    要发送的数据个数
* Return          : 0：操作成功
                    -1：失败
*******************************************************************************/
int SoftI2c_MultiWrite(struct SoftI2cStruct *i2c, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t Len)
{
    uint8_t i = 0;

    // 1：起始信号
    if (SoftI2c_Start(i2c) != 0)
    {   return -1;   }

    // 2：写入设备地址
    if (SoftI2c_Send(i2c, (uint8_t)(deviceAddr << 1)) != 0)
    {   return -1;   }

    // 3：写入寄存器地址
    if (SoftI2c_Send(i2c, registerAddr) != 0)
    {   return -2;   }

    // 4：写入数据
    for (i = 0; i < Len; i++)
    {
        if (SoftI2c_Send(i2c, data[i]) != 0)
        {   return -1;   }
    }

    // 5：结束信号
    SoftI2c_Stop(i2c);

    return 0;
}
/*******************************************************************************
* Function Name   : SoftI2c_Read
* Description     : 软件读取寄存器
* Parameter       : deviceAddr：设备地址，7位地址，无需左移，函数内部已经左移并加入了读写位
                    registerAddr：寄存器地址
                    *data：  存放读取出来数据的字节指针
                    len：个数
* Return          : 0：操作成功
                    1：总线死锁无法解决
                    2：总线硬件连接问题
*******************************************************************************/
int SoftI2c_Read(struct SoftI2cStruct *i2c, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t len)
{
    // 1：起始信号
    if (SoftI2c_Start(i2c) != 0)
    {   return -1;   }

    // 2：写入设备地址
    if (SoftI2c_Send(i2c, (uint8_t)(deviceAddr << 1)) != 0)
    {   return -5;   }

    // 3：写入寄存器地址
    if (SoftI2c_Send(i2c, registerAddr) != 0)
    {   return -2;   }

    // 4：起始信号
    if (SoftI2c_Start(i2c) != 0)
    {   return -3;   }

    // 5：写入设备地址，加入读标志位
    if (SoftI2c_Send(i2c, (uint8_t)((deviceAddr << 1) | (1 << 0))) != 0)
    {   return -4;   }

    // 6：读取数据
    for (uint8_t i = 0; i < len; i++)
    {
        bool isLast = (i == (len - 1)) ? true : false;
        data[i] = SoftI2c_Receive(i2c, isLast);
    }

    // 7：结束信号
    SoftI2c_Stop(i2c);

    return 0;
}
