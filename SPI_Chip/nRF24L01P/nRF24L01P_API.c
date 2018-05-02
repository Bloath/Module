/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../../Sys_Conf.h"
#include "../../Common/Delay.h"
#include "../../BufferQueue/BufferQueue.h"

#include "nRF24L01P_API.h"
#include "nRF24L01P_HAL.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*********************************************************************************************
  * @brief  读取接收缓冲数据
  * @param  pBuff：接收指针，将接收到的数据按个数写到接收指针中
  * @retval 接收数据长度，如果出错，则返回0
  * @remark 
  ********************************************************************************************/
uint8_t ReadRXPayload(uint8_t *packet)
{
    uint8_t width, i;
    width = ReadTopFIFOWidth();

    /* 如果长度大于32，说明长度有错误
       则清除接收缓冲，长度返回为0 */
    if(width > 32)
    { 
      FlushRX();
      return 0; 
    }
    
    CSN_LOW( );
    SPI_RW( RD_RX_PLOAD );
    for( i = 0; i < width; i ++ )
    {
        *( packet + i ) = SPI_RW( 0xFF );
    }
    CSN_HIGH( );
    FlushRX( );
    nRF24L01P_Write_Reg(W_REG + STATUS, 0xff);  				//清除所有的中断标志位

    return width;
}

/*********************************************************************************************
  * @brief  写入发送缓冲
  * @param  pBuff：接收指针，将接收到的数据按个数写到接收指针中
  * @retval 接收数据长度
  * @remark 
  ********************************************************************************************/
void WriteTXPayload_NoAck( uint8_t *packet, uint8_t length)
{
    CSN_LOW();
    
    SPI_RW( W_TX_PAYLOAD_NOACK );
    
    while( length-- )
    { SPI_RW( *packet++ ); }
    CSN_HIGH();
}
/*********************************************************************************************
  * @brief  读取顶层FIFO长度
  * @param  
  * @retval 宽度
  * @remark 
  ********************************************************************************************/
uint8_t ReadTopFIFOWidth()
{
    uint8_t btmp;
    CSN_LOW( );
    SPI_RW( R_RX_PL_WID );
    btmp = SPI_RW( 0xFF );
    CSN_HIGH( );
    return btmp;
}
/*********************************************************************************************
  * @brief  清除发送缓冲
  * @param  
  * @retval 
  * @remark 
  ********************************************************************************************/
void FlushTX()
{
    CSN_LOW( );
    SPI_RW( FLUSH_TX );
    CSN_HIGH( );
}
/*********************************************************************************************
  * @brief  清除接收缓冲
  * @param  
  * @retval 
  * @remark 
  ********************************************************************************************/
void FlushRX()
{
    CSN_LOW( );
    SPI_RW( FLUSH_RX );
    CSN_HIGH( );
}
/*********************************************************************************************
  * @brief  寄存器 单字节 写入
  * @param  reg：寄存器地址，要包含读OR写位
  * @param  value：写入寄存器的值
  * @retval 是否写入成功
  * @remark 
  ********************************************************************************************/
void nRF24L01P_Write_Reg(uint8_t reg, uint8_t value)
{
    CSN_LOW();                 
    SPI_RW(reg);				
    SPI_RW(value);
    
    CSN_HIGH();  
}

/*********************************************************************************************
  * @brief  寄存器 多字节 写入
  * @param  reg：寄存器地址，要包含读OR写位
  * @param  packet：数据包指针
  * @param  length：数据包长度
  * @retval 返回该寄存器的值( 原有值 )
  * @remark 
  ********************************************************************************************/
uint8_t nRF24L01P_Write_Buf(uint8_t reg, uint8_t *packet, uint8_t length)
{
    uint8_t status,i;

    CSN_LOW();                                  			
    status = SPI_RW(reg);                          
    for(i=0; i<length; i++)     
    SPI_RW(*packet++);
    CSN_HIGH();                                      	

    return(status);       
}							  					   
/*********************************************************************************************
  * @brief  寄存器 单字节 读取
  * @param  reg：寄存器地址，要包含读OR写位
  * @retval 寄存器值
  * @remark 
  ********************************************************************************************/
uint8_t nRF24L01P_Read_Reg(uint8_t reg)
{
    uint8_t value;

    CSN_LOW();    
    SPI_RW(reg);			
    value = SPI_RW(0);
    CSN_HIGH();              

    return(value);
}
/*********************************************************************************************
  * @brief  寄存器 多字节 写入
  * @param  reg：寄存器地址，要包含读OR写位
  * @param  packet：数据包指针
  * @param  length：数据包长度
  * @retval 返回该寄存器的值( 原有值 )
  * @remark 
  ********************************************************************************************/
uint8_t nRF24L01P_Read_Buf(uint8_t reg, uint8_t *packet, uint8_t length)
{
    uint8_t status,i;

    CSN_LOW();                                        
    status = SPI_RW(reg);                           
    for(i=0; i<length; i++)
    { packet[i] = SPI_RW(0);  }                //读取数据，低字节在前
    CSN_HIGH();                                        

    return(status);    
}

