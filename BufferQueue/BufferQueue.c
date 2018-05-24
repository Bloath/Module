/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Module/Common/Malloc.h"
#include "BufferQueue.h"
#include "BufferQueue_Handle.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void TxBlockErrorHandle(TxBlockError error);
void TxQueue_FreeBlock(TxBaseBlockStruct *txBlock);     //释放发送块，释放内存清除标志位

/* Private functions ---------------------------------------------------------*/
/*********************************************************************************************

  * @brief  接收单个字节
  * @param  data:       接收的单字节
  * @param  rxBuffer   接收缓冲结构体
  * @retval 无
  * @remark 

  ********************************************************************************************/
void ReceiveSingleByte(uint8_t data, RxBufferStruct *rxBuffer)
{
  rxBuffer->buffer[rxBuffer->count] = data;         //填入缓冲 
  rxBuffer->count ++;                               //计数器递增
  if(rxBuffer->count >= BUFFER_LENGTH)
  { BufferOverFlow(); }
}

/*********************************************************************************************

  * @brief  将缓冲内的数据填写到报文队列中
  * @param  rxBlockList：   接收缓冲结构体
            packet：   要填入接收块的数据包指针
            len；长度
  * @retval 无
  * @remark 返回填充队列号，如果为0xFFFF则为失败

  ********************************************************************************************/
uint16_t RxQueue_Add(RxQueueStruct *rxQueue, uint8_t *packet, uint16_t Len)
{
  uint8_t i = 0;
  
  if(Len == 0)
  { return 0xFFFF; }
  
  /* 找到空闲缓冲，填入 */
  for(i=0; i<BLOCK_COUNT; i++)
  {
    if(!(rxQueue->rxBlocks[i].flag & RX_FLAG_USED))                                      //查找空闲报文队列
    {
      rxQueue->rxBlocks[i].flag |= RX_FLAG_USED;                                              //报文块使用标志位置位
    
      /* 申请内存并填写 */
      rxQueue->rxBlocks[i].message = (uint8_t*)Malloc((Len + 1) * sizeof(uint8_t));         //根据缓冲长度申请内存，多一个字节，用于填写字符串停止符
      memcpy(rxQueue->rxBlocks[i].message, packet, Len);  
      rxQueue->rxBlocks[i].message[Len] = 0;              // 添加结束符，该缓冲块可以用作字符串处理 
      rxQueue->rxBlocks[i].length = Len; 
      
      rxQueue->usedBlockQuantity += 1;
      
      return i;
      //break;
    }
  }
  
  if(i == BLOCK_COUNT)
  { RxBlock_ErrorHandle(rxQueue, BlockFull); }
  
  return 0xFFFF;
}

/*********************************************************************************************

  * @brief  将缓冲内的数据填写到报文队列中
  * @param  rxBlockList：           接收缓冲结构体
  * @param  rxBlock：       报文接收块
  * @param  *f：         接收缓冲结构体
  * @retval 无
  * @remark 

  ********************************************************************************************/
void RxQueue_Handle(RxQueueStruct *rxQueue, void (*RxPacketHandle)(uint8_t*, uint16_t, void *), void *param)
{
  for(uint16_t i=0; i<BLOCK_COUNT; i++)
  {
    if(rxQueue->rxBlocks[i].flag & RX_FLAG_USED)                     //查找需要处理的报文
    {
      (*RxPacketHandle)(rxQueue->rxBlocks[i].message, rxQueue->rxBlocks[i].length, param);
      
      Free(rxQueue->rxBlocks[i].message);                             //释放申请的内存
    
      rxQueue->rxBlocks[i].flag &= ~RX_FLAG_USED;                    //清空已使用标志位
      rxQueue->usedBlockQuantity -= 1;
    }
  }
}

/*********************************************************************************************
  * @brief  发送缓冲处理
  * @param  txBlock：发送缓冲块，发送缓冲队列头
  * @param  Transmit：发送函数指针（调用底层发送函数）
  * @retval 无
  * @remark 
  ********************************************************************************************/
void TxQueue_Handle(TxQueueStruct *txQueue, BoolEnum (*Transmit)(uint8_t*, uint16_t))
{
  uint16_t i;
  BoolEnum isNeedClear = FALSE;
  
  if((txQueue->time + txQueue->interval) > sysTime)
  { return; }
  else
  {  txQueue->time = sysTime; }
  
  for(i=(txQueue->isTxUnordered == TRUE)? txQueue->indexCache: 0; i<BLOCK_COUNT; i++)
  {
      if(txQueue->txBlocks[i].flag & TX_FLAG_USED)            
      {
        
#ifdef TX_BLOCK_TIMEOUT
          /* 发送超时，进入错误处理，并释放发送缓冲块 */
          if((txQueue->txBlocks[i].time + TX_TIME_OUT) < sysTime && txQueue->txBlocks[i].flag & TX_FLAG_TIMEOUT)
          {
            TxBlockErrorHandle(TxBlockError_TimeOut);
            TxQueue_FreeBlock(txQueue->txBlock + i);
            txQueue->usedBlockQuantity -= 1;
            continue;
          }
#endif
        
          /*在已发送标志位为0，或者重复发送为真时
            进行数据的发送，并置位已发送标志位*/
          if(!(txQueue->txBlocks[i].flag & TX_FLAG_SENDED) || txQueue->txBlocks[i].flag & TX_FLAG_RT)
          {
            isNeedClear = Transmit(txQueue->txBlocks[i].message, txQueue->txBlocks[i].length);          // 发送数据
            txQueue->txBlocks[i].flag |= TX_FLAG_SENDED;                                                // 标记为已发送
            txQueue->txBlocks[i].retransCounter ++;                                                     // 重发次数递增
          }
          
          /* 非手动清除 且 (重发超过200次 或者 不需要重发) 的情况下
             清除缓存释放模块 */
          if(isNeedClear == TRUE ||
             (!(txQueue->txBlocks[i].flag & TX_FLAG_MC) 
              && (txQueue->txBlocks[i].retransCounter > txQueue->maxTxCount || !(txQueue->txBlocks[i].flag & TX_FLAG_RT))))
          {
            TxQueue_FreeBlock(&(txQueue->txBlocks[i]));
            txQueue->usedBlockQuantity -= 1;
          }  
          
          /* 如果为无序发送，则将索引缓存，下次直接从下一个缓存开始 */
          if(txQueue->isTxUnordered == TRUE)
          { txQueue->indexCache = (i != (BLOCK_COUNT - 1))? (i + 1):0; }
          
          break;
      }
  }
  
  if(i == BLOCK_COUNT)
  { txQueue->indexCache = 0; }
}

/*********************************************************************************************

  * @brief  填充发送结构体
  * @param  txBlock：发送模块结构体指针
  * @param  message：报文指针
  * @param  length：报文长度
  * @param  custom：自定义标志位，参考SimpleBuffer.h中的TX_FLAG
  * @return 
  * @remark 

  ********************************************************************************************/
uint16_t TxQueue_Add(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, TxModeEnum mode)
{
  uint16_t i;
  
  for(i=0; i<BLOCK_COUNT; i++)
  { 
    if((txQueue->txBlocks[i].flag & TX_FLAG_USED) == 0)
    {
      txQueue->txBlocks[i].message = (uint8_t*)Malloc(length * sizeof(uint8_t));

      memcpy(txQueue->txBlocks[i].message, message, length);
      txQueue->txBlocks[i].length = length;
      txQueue->txBlocks[i].flag |= TX_FLAG_USED;
      
      txQueue->usedBlockQuantity += 1;
      
#ifdef TX_BLOCK_TIMEOUT
      txQueue->txBlocks[i].time = sysTime;
#endif
      
      /* 可以自定义标志位，自动添加占用标志位，默认只发送一次 */
      txQueue->txBlocks[i].flag |= mode;
      
      return i;
      //break;
    }
  }
  
  if(i == BLOCK_COUNT)
  { TxBlock_ErrorHandle(txQueue, BlockFull); }
  
  return 0xFFFF;
}
/*********************************************************************************************

  * @brief  填充发送结构体
  * @param  txBlock：发送模块结构体指针
  * @param  message：报文指针
  * @param  length：报文长度
  * @param  custom：自定义标志位，参考SimpleBuffer.h中的TX_FLAG
  * @param  id：用于清除的标识
  * @return 
  * @remark 

  ********************************************************************************************/
uint16_t TxQueue_AddWithId(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, TxModeEnum mode, TX_ID_SIZE id)
{
  uint16_t blockId = TxQueue_Add(txQueue, message, length, mode);
  
  if(blockId != 0xFFFF)
  { txQueue->txBlocks[blockId].id = id; }
  
  return blockId;
}
/*********************************************************************************************

  * @brief  释放发送缓冲
  * @param  txBlock：发送结构体指针
  * @return 
  * @remark 

  ********************************************************************************************/
void TxQueue_FreeBlock(TxBaseBlockStruct *txBlock)
{
  Free(txBlock->message);
  
  txBlock->flag = 0;
  txBlock->length = 0;
  txBlock->retransCounter = 0;
  txBlock->id = 0;
  
#ifdef TX_BLOCK_TIMEOUT  
  txBlock->time = 0;
#endif
}

/*********************************************************************************************

  * @brief  通过自定义函数的方式清除相应发送缓冲块
  * @param  txBlock：发送结构体指针
            func，通过该func里的处理，将对应发送缓冲清除
            *p：自定义函数参数传递
  * @return 
  * @remark 

  ********************************************************************************************/
void TxQueue_FreeByFunc(TxQueueStruct *txQueue, BoolEnum (*func)(uint8_t*, uint16_t, void*), void *para)
{
  uint16_t i;
  
  for(i=0; i<BLOCK_COUNT; i++)
  { 
    if((txQueue->txBlocks[i].flag & TX_FLAG_USED) != 0)
    {
      if(func(txQueue->txBlocks[i].message, txQueue->txBlocks[i].length, para) == TRUE)
      { 
        TxQueue_FreeBlock(txQueue->txBlocks + i); 
        txQueue->usedBlockQuantity -= 1;
      }
    }
  }
}
/*********************************************************************************************

  * @brief  清除指定发送缓冲
  * @param  txBlock：发送结构体指针
            id：通过自定义标识的方式释放发送缓冲
  * @return 
  * @remark 

  ********************************************************************************************/
void TxQueue_FreeById(TxQueueStruct *txQueue, TX_ID_SIZE id)
{
  uint16_t i;
  
  for(i=0; i<BLOCK_COUNT; i++)
  { 
    if((txQueue->txBlocks[i].flag & TX_FLAG_USED) != 0)
    {
      if(txQueue->txBlocks[i].id == id)
      { 
        TxQueue_FreeBlock(txQueue->txBlocks + i); 
        txQueue->usedBlockQuantity -= 1;
      }
    }
  }
}
