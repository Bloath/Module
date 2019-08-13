/* Includes ------------------------------------------------------------------*/
#include "SoftI2C.h"
#include "../Common/Common.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name   : SoftI2C_Start
* Description     : I2C总线起始信号
* Parameter       : 
* Return          : 0：总线正常，产生起始信号
                    -1：经过9个时钟后总线依然有问题
                    -2：总线在主机发出起始信号之后无应答
*******************************************************************************/
int SoftI2C_Start()
{
    uint8_t i = 0;
    //拉高SCL,SDA准备
    SCL_Set();
    SDA_Set();
    I2C_Delay();

    //SDA未被拉高，说明总线忙
    if (!SDA_Status)
    {
        //产生9个时钟解除死锁状态
        for (i = 0; i < 9; i++)
        {
            SCL_Clr();
            I2C_Delay();
            SCL_Set();
            I2C_Delay();
        }
        //9个时钟后依然低，则说明总线有问题
        if (!SDA_Status)
        {   return -1;   }
    }

    SDA_Clr(); //先拉低SDA
    I2C_Delay();
    SCL_Clr(); //后拉低SCL，产生起始信号
    I2C_Delay();

    //SDA未被拉底，说明总线错误
    if (SDA_Status)
    {   return -2;   }

    return 0;
}

/*******************************************************************************
* Function Name   : SoftI2C_Stop
* Description     : I2C总线停止信号
* Parameter       : 
* Return          : None
*******************************************************************************/
void SoftI2C_Stop()
{
    //拉低SCL,sda
    SCL_Clr();
    SDA_Clr();
    I2C_Delay();

    SCL_Set(); //先拉高SCL
    I2C_Delay();
    SDA_Set(); //再拉高SDA，送出停止
    I2C_Delay();
}

/*******************************************************************************
* Function Name   : SoftI2C_Ack
* Description     : I2C总线的应答，在第九位的SCL的上升沿，判断SDA为高还是低
                    先设置高低再拉高SCL并恢复
* Parameter       : status：ENABLE：送出应答
                            DISENABLE：不送出应答
* Return          : None
*******************************************************************************/
void SoftI2C_Ack(uint8_t Status)
{

    if (Status) //在需要产生应答时，将SDA拉低
    {   SDA_Clr();  }
    else //不需要则拉高(接收最后一个字节)
    {   SDA_Set();  }
    I2C_Delay();

    SCL_Set(); //先拉高SCL
    I2C_Delay();

    SCL_Clr(); //拉低SCL
    I2C_Delay();

    SDA_Set(); // PS：非常重要，在发送ACK之后要把总线抬高，因为是开漏，所以必须通过输出抬高后，芯片才能控制总线
    I2C_Delay();
}

/*******************************************************************************
* Function Name   : SoftI2C_WaitAck
* Description     : I2C总线读取时等待从设备应答
* Parameter       : 
* Return          : 0：成功接收应答信号
                    -1：接收应答信号失败
*******************************************************************************/
int SoftI2C_WaitAck()
{
    int Status;
    SDA_Set(); //拉高SDA，等待输入
    I2C_Delay();
    SCL_Set(); //先拉高SCL
    I2C_Delay();

    if (!SDA_Status)
    {   Status = 0; }
    else
    {   Status = -1; }

    SCL_Clr();
    I2C_Delay();
    return Status;
}

/*******************************************************************************
* Function Name   : SoftI2C_SendByte
* Description     : I2C总线传输单字节数据
* Parameter       : data：8位数据
* Return          : None
*******************************************************************************/
void SoftI2C_SendByte(uint8_t data)
{
    uint8_t i = 0;

    for (i = 0; i < 8; i++)
    {
        SCL_Clr();
        I2C_Delay();

        if (data & (1 << 7))
        {   SDA_Set();  }
        else
        {   SDA_Clr();  }
        I2C_Delay();
        data <<= 1;

        SCL_Set();
        I2C_Delay();
    }
    SCL_Clr();
    I2C_Delay();
}

/*******************************************************************************
* Function Name   : SoftI2C_ReceiveByte
* Description     : I2C总线接收单字节数据
* Parameter       : 
* Return          : None读取的8位数据
*******************************************************************************/
uint8_t SoftI2C_ReceiveByte()
{
    uint8_t i = 0, Temp;

    for (i = 0; i < 8; i++)
    {
        SCL_Clr();
        I2C_Delay();
        SCL_Set();
        I2C_Delay();

        Temp <<= 1;
        if (SDA_Status)
        {   Temp |= 1;  }
        I2C_Delay();
    }
    SCL_Clr();
    I2C_Delay();

    return Temp;
}
/*******************************************************************************
* Function Name   : SoftI2C_WriteByte
* Description     : I2C总线发送数据
* Parameter       : data：   要发送的字节数据
                    isOnlyLSB：只传送低8字节
* Return          : 0：操作成功
                    -1：总线死锁无法解决
*******************************************************************************/
int SoftI2C_Send(DATA_TYPE data, bool isOnlyLSB)
{
    uint8_t status = 0;

    for (uint8_t i = 0; i < DATA_SIZE; i++)
    {
        // 如果只发低字节的话，则只发一次就结束
        if (isOnlyLSB == false)
        {   SoftI2C_SendByte((uint8_t)(data >> (DATA_SIZE - 1 - i) * 8));   }
        else
        {   SoftI2C_SendByte((uint8_t)data);    }

        status = SoftI2C_WaitAck();
        if (status)
        {
            SoftI2C_Stop();
            return status;
        }

        if (isOnlyLSB == true)
        {   return 0;   }
    }

    return 0;
}
/*******************************************************************************
* Function Name   : SoftI2C_Revice
* Description     : I2C总线接收数据
* Parameter       : isLast：是否为最后一个数据（最后一个字节不需要发ack）
* Return          : 
*******************************************************************************/
DATA_TYPE SoftI2C_Receive(bool isLast)
{
    DATA_TYPE data = 0; // 将需要填充的位置先清零

    for (uint8_t i = 0; i < DATA_SIZE; i++)
    {
        data <<= 8;
        data |= SoftI2C_ReceiveByte();

        if (isLast == true && i == (DATA_SIZE - 1))
        {   SoftI2C_Ack(0); }
        else
        {   SoftI2C_Ack(1); }
    }

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
int SoftI2C_SingleWrite(uint8_t deviceAddr, DATA_TYPE registerAddr, DATA_TYPE data)
{
    // 1：起始信号
    if (SoftI2C_Start() != 0)
    {   return -1;   }

    // 2：写入设备地址
    if (SoftI2C_Send((DATA_TYPE)deviceAddr, true) != 0)
    {   return -2;   }

    // 3：写入寄存器地址
    if (SoftI2C_Send(registerAddr, false) != 0)
    {   return -3;   }

    // 4：写入数据
    if (SoftI2C_Send(data, false) != 0)
    {   return -4;   }

    // 5：结束信号
    SoftI2C_Stop();

    return 0;
}

/*******************************************************************************
* Function Name   : SoftI2C_Write
* Description     : 软件写入数据
* Parameter       : deviceAddr：设备地址，7位地址，无需左移，函数内部已经左移并加入了读写位
                    registerAddr：寄存器地址
                    *data：  要发送的数据指针
                    len：    要发送的数据个数
* Return          : 0：操作成功
                    -1：失败
*******************************************************************************/
int SoftI2C_MultiWrite(uint8_t deviceAddr, DATA_TYPE registerAddr, DATA_TYPE *data, uint8_t Len)
{
    uint8_t i = 0;

    // 1：起始信号
    if (SoftI2C_Start() != 0)
    {   return -1;   }

    // 2：写入设备地址
    if (SoftI2C_Send((DATA_TYPE)deviceAddr, true) != 0)
    {   return -1;   }

    // 3：写入寄存器地址
    if (SoftI2C_Send(registerAddr, false) != 0)
    {   return -1;   }

    // 4：写入数据
    for (i = 0; i < Len; i++)
    {
        if (SoftI2C_Send(data[i], false) != 0)
        {   return -1;   }
    }

    // 5：结束信号
    SoftI2C_Stop();

    return 0;
}
/*******************************************************************************
* Function Name   : SoftI2C_Read
* Description     : 软件读取寄存器
* Parameter       : deviceAddr：设备地址，7位地址，无需左移，函数内部已经左移并加入了读写位
                    registerAddr：寄存器地址
                    *data：  存放读取出来数据的字节指针
                    len：个数
* Return          : 0：操作成功
                    1：总线死锁无法解决
                    2：总线硬件连接问题
*******************************************************************************/
int SoftI2C_Read(uint8_t deviceAddr, DATA_TYPE registerAddr, DATA_TYPE *data, uint8_t len)
{
    // 1：起始信号
    if (SoftI2C_Start() != 0)
    {   return -1;   }

    // 2：写入设备地址
    if (SoftI2C_Send((DATA_TYPE)deviceAddr, true) != 0)
    {   return -1;   }

    // 3：写入寄存器地址
    if (SoftI2C_Send(registerAddr, false) != 0)
    {   return -1;   }

    // 4：起始信号
    if (SoftI2C_Start() != 0)
    {   return -1;   }

    // 5：写入设备地址，加入读标志位
    if (SoftI2C_Send((DATA_TYPE)(deviceAddr | (1 << 0)), true) != 0)
    {   return -1;   }

    // 6：读取数据
    for (uint8_t i = 0; i < len; i++)
    {
        bool isLast = (i == (len - 1)) ? true : false;
        data[i] = SoftI2C_Receive(isLast);
    }

    // 7：结束信号
    SoftI2C_Stop();

    return 0;
}
