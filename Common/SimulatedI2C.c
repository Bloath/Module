/* Includes ------------------------------------------------------------------*/
#include "SimulatedI2C.h"
#include "Delay.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SCL_Set() HAL_GPIO_WritePin(i2cPinStruct.sclPort, i2cPinStruct.scl, GPIO_PIN_SET)
#define SCL_Clr() HAL_GPIO_WritePin(i2cPinStruct.sclPort, i2cPinStruct.scl, GPIO_PIN_RESET)
#define SDA_Set() HAL_GPIO_WritePin(i2cPinStruct.sdaPort, i2cPinStruct.sda, GPIO_PIN_SET)
#define SDA_Clr() HAL_GPIO_WritePin(i2cPinStruct.sdaPort, i2cPinStruct.sda, GPIO_PIN_RESET)

#define SDA_Status  (HAL_GPIO_ReadPin(i2cPinStruct.sdaPort,i2cPinStruct.sda) == GPIO_PIN_SET)
#define I2C_Delay()  Delay_us(1);

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name   : SimulatedI2C_Configuration
* Description     : 模拟I2C的端口结构体以及IO设置
* Parameter       : None
* Return          : 
*******************************************************************************/
void SimulatedI2C_Pin_Init(I2C_PinStruct i2cPinStruct, GPIO_TypeDef * sclPort, uint16_t sclPin, GPIO_TypeDef * sdaPort, uint16_t sdaPin)
{
  i2cPinStruct.sclPort = sclPort;
  i2cPinStruct.scl = sclPin;
  i2cPinStruct.sdaPort = sdaPort;
  i2cPinStruct.sda = sdaPin;
  
  SCL_Set(); //SCL拉高
  SDA_Set(); //SDA拉高
}


/*******************************************************************************
* Function Name   : SimulatedI2C_Start
* Description     : 模拟I2C起始信号
* Parameter       : i2cPinStruct：模拟I2C的IO结构体
* Return          : 0：总线正常，产生起始信号
                    1：经过9个时钟后总线依然有问题
                    2：总线在主机发出起始信号之后无应答
*******************************************************************************/
uint8_t SimulatedI2C_Start(I2C_PinStruct i2cPinStruct)
{
  uint8_t i = 0;
  //拉高SCL,SDA准备
  SCL_Set();
  SDA_Set();          
  I2C_Delay();
  
  //SDA未被拉高，说明总线忙
  if(!SDA_Status)
  { 
    //产生9个时钟解除死锁状态
    for(i=0; i<9; i++)  
    {
      SCL_Clr();
      I2C_Delay();
      SCL_Set();
      I2C_Delay();
    }
    //9个时钟后依然低，则说明总线有问题
    if(!SDA_Status)     
    { return 1; }
  }
  
  SDA_Clr();      //先拉低SDA
  I2C_Delay();
  SCL_Clr();      //后拉低SCL，产生起始信号
  I2C_Delay();
  
  //SDA未被拉底，说明总线错误
  if(SDA_Status)
  { return 2; }
  
  return 0;
}

/*******************************************************************************
* Function Name   : SimulatedI2C_Stop
* Description     : 模拟I2C停止信号
* Parameter       : i2cPinStruct：模拟I2C的IO结构体
* Return          : None
*******************************************************************************/
void SimulatedI2C_Stop(I2C_PinStruct i2cPinStruct)
{
  //拉低SCL,sda
  SCL_Clr();
  SDA_Clr();          
  I2C_Delay();
  
  SCL_Set();    //先拉高SCL
  I2C_Delay();
  SDA_Set();    //再拉高SDA，送出停止
  I2C_Delay();
}

/*******************************************************************************
* Function Name   : SimulatedI2C_Ack
* Description     : 模拟I2C的应答
* Parameter       : i2cPinStruct：模拟I2C的IO结构体
                    status：0：在第九个时钟时不产生停止信号
                            1：在第九个时钟时产生停止信号
* Return          : None
*******************************************************************************/
void SimulatedI2C_Ack(I2C_PinStruct i2cPinStruct, uint8_t Status)
{  
  SCL_Set();              //先拉高SCL
  I2C_Delay();
  
  if(Status)              //在需要产生应答时，将SDA拉低
  { SDA_Clr();  }
  else                    //不需要则拉高(接收最后一个字节)
  { SDA_Set();  }
  I2C_Delay();
  
  SCL_Clr();              //拉低SCL 
  I2C_Delay();
}

/*******************************************************************************
* Function Name   : SimulatedI2C_WaitAck
* Description     : 模拟I2C读取时等待从设备应答
* Parameter       : i2cPinStruct：模拟I2C的IO结构体
* Return          : 0：成功接收应答信号
                    1：接收应答信号失败
*******************************************************************************/
uint8_t SimulatedI2C_WaitAck(I2C_PinStruct i2cPinStruct)
{  
  int Status;
  SDA_Set();      //拉高SDA，等待输入
  I2C_Delay();
  SCL_Set();      //先拉高SCL
  I2C_Delay();
  
  if(!SDA_Status)
  { Status = 0;}
  else
  { Status = 1; }
  
  SCL_Clr();
  I2C_Delay();
  return Status;
}

/*******************************************************************************
* Function Name   : SimulatedI2C_SendByte
* Description     : 模拟I2C发送8位数据
* Parameter       : i2cPinStruct：模拟I2C的IO结构体
                    data：8位数据
* Return          : None
*******************************************************************************/
void SimulatedI2C_SendByte(I2C_PinStruct i2cPinStruct, uint8_t data)
{
  uint8_t i = 0;
  
  for(i=0; i<8; i++)
  {
    SCL_Clr();         
    I2C_Delay();
    
    if(data & (1<<7))
    { SDA_Set();  }
    else
    { SDA_Clr();  }
    I2C_Delay();
    data <<= 1;
    
    SCL_Set();
    I2C_Delay();
  }
  SCL_Clr(); 
  I2C_Delay();
}

/*******************************************************************************
* Function Name   : SimulatedI2C_ReceiveByte
* Description     : 模拟I2C读取8位数据
* Parameter       : i2cPinStruct：模拟I2C的IO结构体
* Return          : None读取的8位数据
*******************************************************************************/
uint8_t SimulatedI2C_ReceiveByte(I2C_PinStruct i2cPinStruct)
{
  uint8_t i = 0,Temp;
  
  for(i=0; i<8; i++)
  {
    SCL_Clr();         
    I2C_Delay();
    SCL_Set();      
    I2C_Delay();
    
    Temp <<= 1;
    if(SDA_Status)
    { Temp |= 1;  }
    I2C_Delay();
  }
  SCL_Clr(); 
  I2C_Delay();
  
  return Temp;
}

/*******************************************************************************
* Function Name   : SimulatedI2C_ByteWrite
* Description     : 模拟I2C写入一个字节数据
* Parameter       : i2cPinStruct：模拟I2C的IO结构体
                    deviceAddr：设备地址，7位地址，无需左移，函数内部已经左移并加入了读写位
                    registerAddr：寄存器地址
                    data：   要发送的字节数据
* Return          : 0：操作成功
                    1：总线死锁无法解决
                    2：总线硬件连接问题
*******************************************************************************/
uint8_t SimulatedI2C_ByteWrite(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t data)
{
  uint8_t Status = 0;
  
  /* 写入设备地址，并写入Write位 */
  /* 起始，写入，等待应答 */
  Status = SimulatedI2C_Start(i2cPinStruct);          
  if(Status)     
  { return Status; }
  
  SimulatedI2C_SendByte(i2cPinStruct, deviceAddr << 1);  
  
  Status = SimulatedI2C_WaitAck(i2cPinStruct);        
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                  
    return Status;
  }
  
  /* 写入寄存器地址 */
  /* 写入，等待应答 */
  SimulatedI2C_SendByte(i2cPinStruct, registerAddr);       
  Status = SimulatedI2C_WaitAck(i2cPinStruct);        
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                  
    return Status;
  }               
  
  /* 写入一个字节并等待应答 */
  SimulatedI2C_SendByte(i2cPinStruct, data);          
  Status = SimulatedI2C_WaitAck(i2cPinStruct);        
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                  
    return Status;
  }  
  
  SimulatedI2C_Stop(i2cPinStruct);                    
  
  return 0;
}

/*******************************************************************************
* Function Name   : SimulatedI2C_Write
* Description     : 模拟I2C写入数据
* Parameter       : i2cPinStruct：模拟I2C的IO结构体
                    deviceAddr：设备地址，7位地址，无需左移，函数内部已经左移并加入了读写位
                    registerAddr：寄存器地址
                    *data：  要发送的数据指针
                    len：    要发送的数据个数
* Return          : 0：操作成功
                    1：总线死锁无法解决
                    2：总线硬件连接问题
*******************************************************************************/
uint8_t SimulatedI2C_Write(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t Len)
{
  uint8_t i = 0;
  uint8_t Status = 0;
 
  /* 写入设备地址，并写入Write位 */
  /* 起始，写入，等待应答 */
  Status = SimulatedI2C_Start(i2cPinStruct);          
  if(Status)     
  { return Status; }
  
  SimulatedI2C_SendByte(i2cPinStruct, deviceAddr << 1);  
  
  Status = SimulatedI2C_WaitAck(i2cPinStruct);        
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                  
    return Status;
  }
  
  /* 写入寄存器地址 */
  /* 写入，等待应答 */
  SimulatedI2C_SendByte(i2cPinStruct, registerAddr);       
  Status = SimulatedI2C_WaitAck(i2cPinStruct);        
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                  
    return Status;
  }
  
  /* 循环写入数据，并在每一个字节后等待应答 */
  for(i=0; i<Len; i++)
  {
    Delay_ms(10);
    SimulatedI2C_SendByte(i2cPinStruct, data[i]);      
    Status = SimulatedI2C_WaitAck(i2cPinStruct);      
    if(Status)   
    {
      SimulatedI2C_Stop(i2cPinStruct);                
      return Status;
    }
  }
  
  SimulatedI2C_Stop(i2cPinStruct);                   
  
  return 0;
}
/*******************************************************************************
* Function Name   : SimulatedI2C_ByteRead
* Description     : 模拟I2C读取一个字节
* Parameter       : i2cPinStruct：模拟I2C的IO结构体
                    deviceAddr：设备地址，7位地址，无需左移，函数内部已经左移并加入了读写位
                    registerAddr：寄存器地址
                    *data：  存放读取出来数据的字节指针
* Return          : 0：操作成功
                    1：总线死锁无法解决
                    2：总线硬件连接问题
*******************************************************************************/
uint8_t SimulatedI2C_ByteRead(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data)
{
  uint8_t Status = 0;
  
  /* 写入设备地址，并写入Write位 */
  /* 起始，写入，等待应答 */
  Status = SimulatedI2C_Start(i2cPinStruct);          
  if(Status)     
  { return Status; }
  
  SimulatedI2C_SendByte(i2cPinStruct, deviceAddr << 1);  
  
  Status = SimulatedI2C_WaitAck(i2cPinStruct);        
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                  
    return Status;
  }
  
  /* 写入寄存器地址 */
  /* 写入，等待应答 */
  SimulatedI2C_SendByte(i2cPinStruct, registerAddr);  
  
  Status = SimulatedI2C_WaitAck(i2cPinStruct);                
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                          
    return Status;
  }
  
  /* 写入设备地址，并写入read位 */
  /* 起始，写入，等待应答 */
  Status = SimulatedI2C_Start(i2cPinStruct);                  
  if(Status)     
  { return Status; }
  
  SimulatedI2C_SendByte(i2cPinStruct, deviceAddr | (1<<0)); 
  
  Status = SimulatedI2C_WaitAck(i2cPinStruct);                
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                         
    return Status;
  }
  
  /* 读取一个字节，无需应答 */
  *data = SimulatedI2C_ReceiveByte(i2cPinStruct);           
  SimulatedI2C_Ack(i2cPinStruct, DISABLE); 
  
  SimulatedI2C_Stop(i2cPinStruct);
  
  return 0;
}
/*******************************************************************************
* Function Name   : SimulatedI2C_Read
* Description     : 模拟I2C读取数据
* Parameter       : i2cPinStruct：模拟I2C的IO结构体
                    deviceAddr：设备地址，7位地址，无需左移，函数内部已经左移并加入了读写位
                    registerAddr：寄存器地址
                    *data：  存放读取出来数据的数组指针
                    len：    要发送的数据个数
* Return          : 0：操作成功
                    1：总线死锁无法解决
                    2：总线硬件连接问题
*******************************************************************************/
uint8_t SimulatedI2C_Read(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t Len)
{
  uint8_t i=0;
  uint8_t Status = 0;
  
  /* 写入设备地址，并写入Write位 */
  /* 起始，写入，等待应答 */
  Status = SimulatedI2C_Start(i2cPinStruct);                  
  if(Status)     
  { return Status; }
  
  SimulatedI2C_SendByte(i2cPinStruct, deviceAddr << 1);      
  
  Status = SimulatedI2C_WaitAck(i2cPinStruct);               
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                          
    return Status;
  }
  
  /* 写入寄存器地址 */
  /* 写入，等待应答 */
  SimulatedI2C_SendByte(i2cPinStruct, registerAddr);  
  
  Status = SimulatedI2C_WaitAck(i2cPinStruct);                
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                          
    return Status;
  }
  
  /* 写入设备地址，并写入read位 */
  /* 起始，写入，等待应答 */
  Status = SimulatedI2C_Start(i2cPinStruct);                  
  if(Status)     
  { return Status; }
  
  SimulatedI2C_SendByte(i2cPinStruct, deviceAddr | (1<<0)); 
  
  Status = SimulatedI2C_WaitAck(i2cPinStruct);                
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                         
    return Status;
  }
  
  /* 循环接收数据，并在每一个字节后发送应答 */
  /* 最后一个字节发送完成后无需应答 */
  for(i=0; i<Len; i++)                                         
  {
    data[i] = SimulatedI2C_ReceiveByte(i2cPinStruct); 
    if(i = (Len - 1))                                           
    { SimulatedI2C_Ack(i2cPinStruct, DISABLE); }
    else
    { SimulatedI2C_Ack(i2cPinStruct, ENABLE);  }
  }
  
  SimulatedI2C_Stop(i2cPinStruct);
  
  return 0;
}
