/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "SimpleBuffer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void TxBlockErrorHandle(TxBlockError error);

/* Private functions ---------------------------------------------------------*/
/*********************************************************************************************

  * @brief  接收单个字节
  * @param  data:       接收的单字节
  * @param  rxBuffer   接收缓冲结构体
  * @retval 无
  * @remark 

  ********************************************************************************************/
void ReceiveSingleByte(uint8_t data, RxBufferTypeDef *rxBuffer)
{
  rxBuffer->buffer[rxBuffer->count] = data;         //填入缓冲 
  rxBuffer->count ++;                               //计数器递增
}

/*********************************************************************************************

  * @brief  将缓冲内的数据填写到报文队列中
  * @param  rxBlockList：   接收缓冲结构体
            packet：   要填入接收块的数据包指针
            len；长度
  * @retval 无
  * @remark 

  ********************************************************************************************/
void FillRxBlock( RxBlockTypeDef *rxBlock, uint8_t *packet, uint16_t Len)
{
  if(Len == 0)
  { return; }
  
  /* 找到空闲缓冲，填入 */
  for(uint16_t i=0; i<RX_BLOCK_COUNT; i++)
  {
    if(!(rxBlock[i].flag & RX_FLAG_USED))                                      //查找空闲报文队列
    {
      rxBlock[i].flag |= RX_FLAG_USED;                                              //报文块使用标志位置位

#ifdef DYNAMIC_MEMORY      
      rxBlock[i].message = (uint8_t*)malloc((Len + 1) * sizeof(uint8_t));         //根据缓冲长度申请内存，多一个字节，用于填写字符串停止符
#endif
      
      memcpy(rxBlock[i].message, packet, Len);  
      
      rxBlock[i].message[Len] = 0;              // 添加结束符，该缓冲块可以用作字符串处理 
      
      rxBlock[i].length = Len; 
      break;
    }
  }
}

/*********************************************************************************************

  * @brief  将缓冲内的数据填写到报文队列中
  * @param  rxBlockList：           接收缓冲结构体
  * @param  rxBlock：       报文接收块
  * @param  *f：         接收缓冲结构体
  * @retval 无
  * @remark 

  ********************************************************************************************/
void RxBlockListHandle(RxBlockTypeDef *rxBlock, void (*f)(uint8_t*, uint16_t))
{
  for(uint16_t i=0; i<RX_BLOCK_COUNT; i++)
  {
    if(rxBlock[i].flag & RX_FLAG_USED)                     //查找需要处理的报文
    {
      (*f)(rxBlock[i].message, rxBlock[i].length);
      
#ifdef DYNAMIC_MEMORY  
      free(rxBlock[i].message);                             //释放申请的内存,定长
#endif      
    
      rxBlock[i].flag &= ~RX_FLAG_USED;                    //清空已使用标志位
    }
  }
}

/*********************************************************************************************
  * @brief  无线发送缓冲处理
  * @param  txBlock：发送缓冲块，发送缓冲队列头
  * @param  Transmit：发送函数指针（调用底层发送函数）
  * @param  interval：发送缓冲函数调用间隔时间
  * @retval 无
  * @remark 
  ********************************************************************************************/
void TxBlockListHandle(TxBlockTypeDef *txBlock, void (*Transmit)(uint8_t*, uint16_t), uint32_t interval)
{
  uint16_t i;
  static uint32_t time = 0;
  
  if((time + interval) > sysTime)
  { return; }
  else
  {  time = sysTime; }
  
  for(i=0; i<TX_BLOCK_COUNT; i++)
  {
      if(txBlock[i].flag & TX_FLAG_USED)            
      {
          /* 发送超时，进入错误处理，并释放发送缓冲块 */
          if((txBlock[i].time + TX_TIME_OUT) < sysTime && txBlock[i].flag & TX_FLAG_TIMEOUT)
          {
            TxBlockErrorHandle(TxBlockError_TimeOut);
            FreeTxBlock(&txBlock[i]);
            continue;
          }
        
          /*在已发送标志位为0，或者重复发送为真时
            进行数据的发送，并置位已发送标志位*/
          if(!(txBlock[i].flag & TX_FLAG_SENDED) || txBlock[i].flag & TX_FLAG_RT)
          {
            Transmit(txBlock[i].message, txBlock[i].length);             //发送数据
            txBlock[i].flag |= TX_FLAG_SENDED;
            //break;              //注意这个位置
          }
        
          txBlock[i].retransCounter ++;                            //重发次数递增
          
          /* 非手动清除 且 (重发超过200次 或者 不需要重发) 的情况下
             清除缓存释放模块 */
          if(!(txBlock[i].flag & TX_FLAG_MC) 
             && (txBlock[i].retransCounter > 200 || !(txBlock[i].flag & TX_FLAG_RT)))
          {
            FreeTxBlock(&txBlock[i]);
          }  
      }
  }
}

/*********************************************************************************************

  * @brief  填充发送结构体
  * @param  txBlock：发送模块结构体指针
  * @param  message：报文指针
  * @param  length：报文长度
  * @return 
  * @remark 

  ********************************************************************************************/
void TxBlockErrorHandle(TxBlockError error)
{

  
}

/*********************************************************************************************

  * @brief  填充发送结构体
  * @param  txBlock：发送模块结构体指针
  * @param  message：报文指针
  * @param  length：报文长度
  * @return 
  * @remark 

  ********************************************************************************************/
uint8_t FillTxBlock(TxBlockTypeDef *txBlock, uint8_t *message, uint16_t length,  uint8_t custom)
{
  uint16_t i;
  
  for(i=0; i<TX_BLOCK_COUNT; i++)
  { 
    if((txBlock[i].flag & TX_FLAG_USED) == 0)
    {
      #ifdef DYNAMIC_MEMORY
        txBlock[i].message = (uint8_t*)malloc(length * sizeof(uint8_t));
      #endif

      memcpy(txBlock[i].message, message, length);
      txBlock[i].length = length;
      txBlock[i].flag |= TX_FLAG_USED;
      txBlock[i].time = sysTime;

      /* 可以自定义标志位，自动添加占用标志位，默认只发送一次 */
      txBlock[i].flag |= custom;
      
      break;
    }
    
  }
  
  return (i==TX_BLOCK_COUNT)?1:0;
}
/*********************************************************************************************

  * @brief  释放发送缓冲
  * @param  txBlock：发送结构体指针
  * @return 
  * @remark 

  ********************************************************************************************/
void FreeTxBlock(TxBlockTypeDef *txBlock)
{
#ifdef DYNAMIC_MEMORY
  free(txBlock->message);
#endif
  
  txBlock->flag = 0;
  txBlock->length = 0;
  txBlock->retransCounter = 0;
  txBlock->time = 0;
}

/*********************************************************************************************

  * @brief  清除指定发送缓冲
  * @param  txBlock：发送结构体指针
            func，通过该func里的处理，将对应发送缓冲清除
  * @return 
  * @remark 

  ********************************************************************************************/
void ClearSpecifyBlock(TxBlockTypeDef *txBlock, uint8_t (*func)(uint8_t*, uint16_t, void*), void *p)
{
  uint16_t i;
  
  for(i=0; i<TX_BLOCK_COUNT; i++)
  { 
    if((txBlock[i].flag & TX_FLAG_USED) != 0)
    {
      if(!func(txBlock[i].message, txBlock[i].length, p))
      { FreeTxBlock(txBlock + i); }
    }
    
  }
}

/*********************************************************************************************

  * @brief  判断数据包
  * @param  srcPacket：源数据包指针
  * @param  desPacket：目的数据包指针
  * @param  length：长度
  * @return 
  * @remark 判断数据包是否相同，相同为0，不同为1

  ********************************************************************************************/
uint8_t isPacketSame(uint8_t *srcPacket, uint8_t *desPacket, uint16_t length)
{
  uint16_t i = 0;
  for(i=0; i<length; i++)
  { 
    if(desPacket[i] != srcPacket[i])
    { break; } 
  }
  
  return (i==length)? 0 : 1;
}