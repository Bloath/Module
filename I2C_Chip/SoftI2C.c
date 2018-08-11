/* Includes ------------------------------------------------------------------*/
#include "../Common/Delay.h"
#include "../Sys_Conf.h"

#include "SoftI2C.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name   : SoftI2C_Start
* Description     : I2C������ʼ�ź�
* Parameter       : 
* Return          : 0������������������ʼ�ź�
                    1������9��ʱ�Ӻ�������Ȼ������
                    2������������������ʼ�ź�֮����Ӧ��
*******************************************************************************/
uint8_t SoftI2C_Start()
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
* Function Name   : SoftI2C_Stop
* Description     : I2C����ֹͣ�ź�
* Parameter       : 
* Return          : None
*******************************************************************************/
void SoftI2C_Stop()
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
* Function Name   : SoftI2C_Ack
* Description     : I2C���ߵ�Ӧ���ڵھ�λ��SCL�������أ��ж�SDAΪ�߻��ǵ�
                    �����øߵ�������SCL���ָ�
* Parameter       : status��ENABLE���ͳ�Ӧ��
                            DISENABLE�����ͳ�Ӧ��
* Return          : None
*******************************************************************************/
void SoftI2C_Ack(uint8_t Status)
{  
                                
  if(Status)              //����Ҫ����Ӧ��ʱ����SDA����
  { SDA_Clr();  }
  else                    //����Ҫ������(�������һ���ֽ�)
  { SDA_Set();  }
  I2C_Delay();

  SCL_Set();              //������SCL
  I2C_Delay();
  
  SCL_Clr();              //����SCL 
  I2C_Delay();
  
  SDA_Set();                    // PS���ǳ���Ҫ���ڷ���ACK֮��Ҫ������̧�ߣ���Ϊ�ǿ�©�����Ա���ͨ�����̧�ߺ�оƬ���ܿ�������
  I2C_Delay();
}

/*******************************************************************************
* Function Name   : SoftI2C_WaitAck
* Description     : I2C���߶�ȡʱ�ȴ����豸Ӧ��
* Parameter       : 
* Return          : 0���ɹ�����Ӧ���ź�
                    1������Ӧ���ź�ʧ��
*******************************************************************************/
uint8_t SoftI2C_WaitAck()
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
* Function Name   : SoftI2C_SendByte
* Description     : I2C���ߴ��䵥�ֽ�����
* Parameter       : data��8λ����
* Return          : None
*******************************************************************************/
void SoftI2C_SendByte(uint8_t data)
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
* Function Name   : SoftI2C_ReceiveByte
* Description     : I2C���߽��յ��ֽ�����
* Parameter       : 
* Return          : None��ȡ��8λ����
*******************************************************************************/
uint8_t SoftI2C_ReceiveByte()
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
* Function Name   : SoftI2C_WriteByte
* Description     : I2C���߷�������
* Parameter       : data��   Ҫ���͵��ֽ�����
                    isOnlyLSB��ֻ���͵�8�ֽ�
* Return          : 0�������ɹ�
                    1�����������޷����
                    2������Ӳ����������
*******************************************************************************/
uint8_t SoftI2C_Send(DATA_TYPE data, BoolEnum isOnlyLSB)
{
  uint8_t status = 0;
  
  for(uint8_t i=0; i<DATA_SIZE; i++)
  {
    // ���ֻ�����ֽڵĻ�����ֻ��һ�ξͽ���
    if(isOnlyLSB == FALSE)
    { SoftI2C_SendByte((uint8_t)(data >> (DATA_SIZE - 1 - i) * 8)); }
    else
    { SoftI2C_SendByte((uint8_t)data); }
    
    status = SoftI2C_WaitAck();        
    if(status)   
    {
      SoftI2C_Stop();                  
      return status;
    }
    
    if(isOnlyLSB == TRUE)
    { return 0; }
  }
  
  return 0;
}
/*******************************************************************************
* Function Name   : SoftI2C_Revice
* Description     : I2C���߽�������
* Parameter       : isLast���Ƿ�Ϊ���һ�����ݣ����һ���ֽڲ���Ҫ��ack��
* Return          : 0�������ɹ�
                    1�����������޷����
                    2������Ӳ����������
*******************************************************************************/
DATA_TYPE SoftI2C_Receive(BoolEnum isLast)
{
  DATA_TYPE data = 0;            // ����Ҫ����λ��������
  
  for(uint8_t i=0; i<DATA_SIZE; i++)
  {
    data <<= 8;
    data |= SoftI2C_ReceiveByte(); 
    
    if(isLast == TRUE && i == (DATA_SIZE - 1))
    { SoftI2C_Ack(0); }
    else
    { SoftI2C_Ack(1); }
  }
  
  return data;
}
/*******************************************************************************

* Description     : �������д
* Parameter       : i2cPinStruct�������IO�ṹ��
                    deviceAddr���豸��ַ��7λ��ַ���������ƣ������ڲ��Ѿ����Ʋ������˶�дλ
                    registerAddr���Ĵ�����ַ
                    data��   Ҫ���͵��ֽ�����
* Return          : 0�������ɹ�
                    1��ʧ��
*******************************************************************************/
uint8_t SoftI2C_SingleWrite(uint8_t deviceAddr, DATA_TYPE registerAddr, DATA_TYPE data)
{
  // 1����ʼ�ź�   
  if(SoftI2C_Start() != 0)     
  { return 1; }
  
  // 2��д���豸��ַ
  if(SoftI2C_Send((DATA_TYPE)deviceAddr, TRUE) != 0)
  { return 1; }
  
  // 3��д��Ĵ�����ַ
  if(SoftI2C_Send(registerAddr, FALSE) != 0)
  { return 1; }

  // 4��д������
  if(SoftI2C_Send(data, FALSE) != 0)
  { return 1; }
  
  // 5�������ź�
  SoftI2C_Stop();                    
  
  return 0;
}

/*******************************************************************************
* Function Name   : SoftI2C_Write
* Description     : ���д������
* Parameter       : deviceAddr���豸��ַ��7λ��ַ���������ƣ������ڲ��Ѿ����Ʋ������˶�дλ
                    registerAddr���Ĵ�����ַ
                    *data��  Ҫ���͵�����ָ��
                    len��    Ҫ���͵����ݸ���
* Return          : 0�������ɹ�
                    1��ʧ��
*******************************************************************************/
uint8_t SoftI2C_MultiWrite(uint8_t deviceAddr, DATA_TYPE registerAddr, DATA_TYPE *data, uint8_t Len)
{
  uint8_t i = 0;
 
  // 1����ʼ�ź�   
  if(SoftI2C_Start() != 0)     
  { return 1; }
  
  // 2��д���豸��ַ
  if(SoftI2C_Send((DATA_TYPE)deviceAddr, TRUE) != 0)
  { return 1; }
  
  // 3��д��Ĵ�����ַ
  if(SoftI2C_Send(registerAddr, FALSE) != 0)
  { return 1; }
  
  // 4��д������
  for(i=0; i<Len; i++)
  {
    if(SoftI2C_Send(data[i], FALSE) != 0)
    { return 1; }
  }
  
  // 5�������ź�
  SoftI2C_Stop();                   
  
  return 0;
}
/*******************************************************************************
* Function Name   : SoftI2C_Read
* Description     : �����ȡ�Ĵ���
* Parameter       : deviceAddr���豸��ַ��7λ��ַ���������ƣ������ڲ��Ѿ����Ʋ������˶�дλ
                    registerAddr���Ĵ�����ַ
                    *data��  ��Ŷ�ȡ�������ݵ��ֽ�ָ��
                    len������
* Return          : 0�������ɹ�
                    1�����������޷����
                    2������Ӳ����������
*******************************************************************************/
uint8_t SoftI2C_Read(uint8_t deviceAddr, DATA_TYPE registerAddr, DATA_TYPE *data, uint8_t len)
{
  // 1����ʼ�ź�   
  if(SoftI2C_Start() != 0)     
  { return 1; }
  
  // 2��д���豸��ַ
  if(SoftI2C_Send((DATA_TYPE)deviceAddr, TRUE) != 0)
  { return 1; }
  
  // 3��д��Ĵ�����ַ
  if(SoftI2C_Send(registerAddr, FALSE) != 0)
  { return 1; }
  
  // 4����ʼ�ź�   
  if(SoftI2C_Start() != 0)     
  { return 1; }
  
  // 5��д���豸��ַ���������־λ
  if(SoftI2C_Send((DATA_TYPE)(deviceAddr | (1<<0)), TRUE) != 0)
  { return 1; }
 
  // 6����ȡ����
  for(uint8_t i=0; i<len; i++)                                         
  {
    BoolEnum isLast = (i == (len - 1))? TRUE:FALSE; 
    data[i] = SoftI2C_Receive(isLast);
  }
  
  // 7�������ź�
  SoftI2C_Stop(); 
  
  return 0;
}
