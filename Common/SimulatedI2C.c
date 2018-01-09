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
* Description     : ģ��I2C�Ķ˿ڽṹ���Լ�IO����
* Parameter       : None
* Return          : 
*******************************************************************************/
void SimulatedI2C_Pin_Init(I2C_PinStruct i2cPinStruct, GPIO_TypeDef * sclPort, uint16_t sclPin, GPIO_TypeDef * sdaPort, uint16_t sdaPin)
{
  i2cPinStruct.sclPort = sclPort;
  i2cPinStruct.scl = sclPin;
  i2cPinStruct.sdaPort = sdaPort;
  i2cPinStruct.sda = sdaPin;
  
  SCL_Set(); //SCL����
  SDA_Set(); //SDA����
}


/*******************************************************************************
* Function Name   : SimulatedI2C_Start
* Description     : ģ��I2C��ʼ�ź�
* Parameter       : i2cPinStruct��ģ��I2C��IO�ṹ��
* Return          : 0������������������ʼ�ź�
                    1������9��ʱ�Ӻ�������Ȼ������
                    2������������������ʼ�ź�֮����Ӧ��
*******************************************************************************/
uint8_t SimulatedI2C_Start(I2C_PinStruct i2cPinStruct)
{
  uint8_t i = 0;
  //����SCL,SDA׼��
  SCL_Set();
  SDA_Set();          
  I2C_Delay();
  
  //SDAδ�����ߣ�˵������æ
  if(!SDA_Status)
  { 
    //����9��ʱ�ӽ������״̬
    for(i=0; i<9; i++)  
    {
      SCL_Clr();
      I2C_Delay();
      SCL_Set();
      I2C_Delay();
    }
    //9��ʱ�Ӻ���Ȼ�ͣ���˵������������
    if(!SDA_Status)     
    { return 1; }
  }
  
  SDA_Clr();      //������SDA
  I2C_Delay();
  SCL_Clr();      //������SCL��������ʼ�ź�
  I2C_Delay();
  
  //SDAδ�����ף�˵�����ߴ���
  if(SDA_Status)
  { return 2; }
  
  return 0;
}

/*******************************************************************************
* Function Name   : SimulatedI2C_Stop
* Description     : ģ��I2Cֹͣ�ź�
* Parameter       : i2cPinStruct��ģ��I2C��IO�ṹ��
* Return          : None
*******************************************************************************/
void SimulatedI2C_Stop(I2C_PinStruct i2cPinStruct)
{
  //����SCL,sda
  SCL_Clr();
  SDA_Clr();          
  I2C_Delay();
  
  SCL_Set();    //������SCL
  I2C_Delay();
  SDA_Set();    //������SDA���ͳ�ֹͣ
  I2C_Delay();
}

/*******************************************************************************
* Function Name   : SimulatedI2C_Ack
* Description     : ģ��I2C��Ӧ��
* Parameter       : i2cPinStruct��ģ��I2C��IO�ṹ��
                    status��0���ڵھŸ�ʱ��ʱ������ֹͣ�ź�
                            1���ڵھŸ�ʱ��ʱ����ֹͣ�ź�
* Return          : None
*******************************************************************************/
void SimulatedI2C_Ack(I2C_PinStruct i2cPinStruct, uint8_t Status)
{  
  SCL_Set();              //������SCL
  I2C_Delay();
  
  if(Status)              //����Ҫ����Ӧ��ʱ����SDA����
  { SDA_Clr();  }
  else                    //����Ҫ������(�������һ���ֽ�)
  { SDA_Set();  }
  I2C_Delay();
  
  SCL_Clr();              //����SCL 
  I2C_Delay();
}

/*******************************************************************************
* Function Name   : SimulatedI2C_WaitAck
* Description     : ģ��I2C��ȡʱ�ȴ����豸Ӧ��
* Parameter       : i2cPinStruct��ģ��I2C��IO�ṹ��
* Return          : 0���ɹ�����Ӧ���ź�
                    1������Ӧ���ź�ʧ��
*******************************************************************************/
uint8_t SimulatedI2C_WaitAck(I2C_PinStruct i2cPinStruct)
{  
  int Status;
  SDA_Set();      //����SDA���ȴ�����
  I2C_Delay();
  SCL_Set();      //������SCL
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
* Description     : ģ��I2C����8λ����
* Parameter       : i2cPinStruct��ģ��I2C��IO�ṹ��
                    data��8λ����
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
* Description     : ģ��I2C��ȡ8λ����
* Parameter       : i2cPinStruct��ģ��I2C��IO�ṹ��
* Return          : None��ȡ��8λ����
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
* Description     : ģ��I2Cд��һ���ֽ�����
* Parameter       : i2cPinStruct��ģ��I2C��IO�ṹ��
                    deviceAddr���豸��ַ��7λ��ַ���������ƣ������ڲ��Ѿ����Ʋ������˶�дλ
                    registerAddr���Ĵ�����ַ
                    data��   Ҫ���͵��ֽ�����
* Return          : 0�������ɹ�
                    1�����������޷����
                    2������Ӳ����������
*******************************************************************************/
uint8_t SimulatedI2C_ByteWrite(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t data)
{
  uint8_t Status = 0;
  
  /* д���豸��ַ����д��Writeλ */
  /* ��ʼ��д�룬�ȴ�Ӧ�� */
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
  
  /* д��Ĵ�����ַ */
  /* д�룬�ȴ�Ӧ�� */
  SimulatedI2C_SendByte(i2cPinStruct, registerAddr);       
  Status = SimulatedI2C_WaitAck(i2cPinStruct);        
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                  
    return Status;
  }               
  
  /* д��һ���ֽڲ��ȴ�Ӧ�� */
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
* Description     : ģ��I2Cд������
* Parameter       : i2cPinStruct��ģ��I2C��IO�ṹ��
                    deviceAddr���豸��ַ��7λ��ַ���������ƣ������ڲ��Ѿ����Ʋ������˶�дλ
                    registerAddr���Ĵ�����ַ
                    *data��  Ҫ���͵�����ָ��
                    len��    Ҫ���͵����ݸ���
* Return          : 0�������ɹ�
                    1�����������޷����
                    2������Ӳ����������
*******************************************************************************/
uint8_t SimulatedI2C_Write(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t Len)
{
  uint8_t i = 0;
  uint8_t Status = 0;
 
  /* д���豸��ַ����д��Writeλ */
  /* ��ʼ��д�룬�ȴ�Ӧ�� */
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
  
  /* д��Ĵ�����ַ */
  /* д�룬�ȴ�Ӧ�� */
  SimulatedI2C_SendByte(i2cPinStruct, registerAddr);       
  Status = SimulatedI2C_WaitAck(i2cPinStruct);        
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                  
    return Status;
  }
  
  /* ѭ��д�����ݣ�����ÿһ���ֽں�ȴ�Ӧ�� */
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
* Description     : ģ��I2C��ȡһ���ֽ�
* Parameter       : i2cPinStruct��ģ��I2C��IO�ṹ��
                    deviceAddr���豸��ַ��7λ��ַ���������ƣ������ڲ��Ѿ����Ʋ������˶�дλ
                    registerAddr���Ĵ�����ַ
                    *data��  ��Ŷ�ȡ�������ݵ��ֽ�ָ��
* Return          : 0�������ɹ�
                    1�����������޷����
                    2������Ӳ����������
*******************************************************************************/
uint8_t SimulatedI2C_ByteRead(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data)
{
  uint8_t Status = 0;
  
  /* д���豸��ַ����д��Writeλ */
  /* ��ʼ��д�룬�ȴ�Ӧ�� */
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
  
  /* д��Ĵ�����ַ */
  /* д�룬�ȴ�Ӧ�� */
  SimulatedI2C_SendByte(i2cPinStruct, registerAddr);  
  
  Status = SimulatedI2C_WaitAck(i2cPinStruct);                
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                          
    return Status;
  }
  
  /* д���豸��ַ����д��readλ */
  /* ��ʼ��д�룬�ȴ�Ӧ�� */
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
  
  /* ��ȡһ���ֽڣ�����Ӧ�� */
  *data = SimulatedI2C_ReceiveByte(i2cPinStruct);           
  SimulatedI2C_Ack(i2cPinStruct, DISABLE); 
  
  SimulatedI2C_Stop(i2cPinStruct);
  
  return 0;
}
/*******************************************************************************
* Function Name   : SimulatedI2C_Read
* Description     : ģ��I2C��ȡ����
* Parameter       : i2cPinStruct��ģ��I2C��IO�ṹ��
                    deviceAddr���豸��ַ��7λ��ַ���������ƣ������ڲ��Ѿ����Ʋ������˶�дλ
                    registerAddr���Ĵ�����ַ
                    *data��  ��Ŷ�ȡ�������ݵ�����ָ��
                    len��    Ҫ���͵����ݸ���
* Return          : 0�������ɹ�
                    1�����������޷����
                    2������Ӳ����������
*******************************************************************************/
uint8_t SimulatedI2C_Read(I2C_PinStruct i2cPinStruct, uint8_t deviceAddr, uint8_t registerAddr, uint8_t *data, uint8_t Len)
{
  uint8_t i=0;
  uint8_t Status = 0;
  
  /* д���豸��ַ����д��Writeλ */
  /* ��ʼ��д�룬�ȴ�Ӧ�� */
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
  
  /* д��Ĵ�����ַ */
  /* д�룬�ȴ�Ӧ�� */
  SimulatedI2C_SendByte(i2cPinStruct, registerAddr);  
  
  Status = SimulatedI2C_WaitAck(i2cPinStruct);                
  if(Status)   
  {
    SimulatedI2C_Stop(i2cPinStruct);                          
    return Status;
  }
  
  /* д���豸��ַ����д��readλ */
  /* ��ʼ��д�룬�ȴ�Ӧ�� */
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
  
  /* ѭ���������ݣ�����ÿһ���ֽں���Ӧ�� */
  /* ���һ���ֽڷ�����ɺ�����Ӧ�� */
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
