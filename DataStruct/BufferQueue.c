/* Includes ------------------------------------------------------------------*/
#include "Module/Module.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void TxQueue_FreeBlock(struct TxQueueStruct *txQueue, struct TxBaseBlockStruct *txBlock);   //释放发送块，释放内存清除标志位

/* Private functions ---------------------------------------------------------*/
/*********************************************************************************************

  * @brief  接收单个字节 
  * @param  data:       接收的单字节
            rxBuffer   接收缓冲结构体
  * @retval 计数器当前位置
            -1：溢出
  * @remark 

  ********************************************************************************************/
int ReceiveSingleByte(uint8_t data, struct RxBufferStruct *rxBuffer)
{
    rxBuffer->_buffer[rxBuffer->count] = data; //填入缓冲
    rxBuffer->count++;                        //计数器递增
    if (rxBuffer->count >= rxBuffer->length)
    {   return -1;  }

    return rxBuffer->count;
}

/*********************************************************************************************

  * @brief  将缓冲内的数据填写到报文队列中
  * @param  rxBlockList：   接收缓冲结构体
            packet： 要填入接收块的数据包指针
            len；长度
            isMalloc：是否为已经申请的
  * @retval 返回填充队列号，
            -1：队列已满
            -2：填充数据长度为0
            -3：内存池已满
  * @remark 

  ********************************************************************************************/
int RxQueue_Add(struct RxQueueStruct *rxQueue, uint8_t *packet, uint16_t Len, bool isMalloc)
{
    uint8_t i = 0;

    if (Len == 0)
    {   return -2;  }

    /* 找到空闲缓冲，填入 */
    for (i = 0; i < BLOCK_COUNT; i++)
    {
        if (!(rxQueue->__rxBlocks[i].flag & RX_FLAG_USED)) //查找空闲报文队列
        {
            /* 申请内存并填写 */
            if (isMalloc == false)
            {
                rxQueue->__rxBlocks[i].message = (uint8_t *)Malloc((Len + 1) * sizeof(uint8_t)); //根据缓冲长度申请内存，多一个字节，用于填写字符串停止符
                memset(rxQueue->__rxBlocks[i].message, 0, (Len + 1) * sizeof(uint8_t));
                if(rxQueue->__rxBlocks[i].message == NULL)
                {   return -3;   }
                memcpy(rxQueue->__rxBlocks[i].message, packet, Len);
            }
            else
            {   rxQueue->__rxBlocks[i].message = packet;  }
            
            rxQueue->__rxBlocks[i].flag |= RX_FLAG_USED; //报文块使用标志位置位
            rxQueue->__rxBlocks[i].message[Len] = 0; // 添加结束符，该缓冲块可以用作字符串处理
            rxQueue->__rxBlocks[i].length = Len;

            rxQueue->_usedBlockQuantity += 1;

            return i;
            //break;
        }
    }

    /* 如果为已经申请了，但是未添加，则直接free掉 */
    if (isMalloc == true)
    {   Free(packet);  }

    return -1;
}

/*********************************************************************************************

  * @brief  将缓冲内的数据填写到报文队列中
  * @param  rxBlockList：   接收缓冲结构体
  * @retval 无
  * @remark 

  ********************************************************************************************/
void RxQueue_Handle(struct RxQueueStruct *rxQueue)
{
    /* 没有则直接退出 */
    if (rxQueue->_usedBlockQuantity == 0)
    {   return; }
  
    for (uint16_t i = 0; i < BLOCK_COUNT; i++)
    {
        if (rxQueue->__rxBlocks[i].flag & RX_FLAG_USED) //查找需要处理的报文
        {
            if(rxQueue->CallBack_RxPacketHandle != NULL)
            {   rxQueue->CallBack_RxPacketHandle(rxQueue->__rxBlocks + i);    }
            
            /* 当接收缓冲的清除前回调不为空，且返回值为true时，则直接跳过，由回调函数后续处理清除该数据 */
            if (rxQueue->CallBack_BeforeFree != NULL)
            {   
                if (rxQueue->CallBack_BeforeFree(rxQueue->__rxBlocks + i))
                {   goto JumpFree;  }
            }

            Free(rxQueue->__rxBlocks[i].message);         //释放申请的内存
        
        JumpFree:
            rxQueue->__rxBlocks[i].flag &= ~RX_FLAG_USED; //清空已使用标志位
            rxQueue->_usedBlockQuantity -= 1;
        }
    }
}

/*********************************************************************************************

  * @brief  发送缓冲处理
  * @param  txBlock：发送缓冲块，发送缓冲队列头
  * @retval -1：有数据但是不能发送（发送一次并手动清除）
            -2：缓存为空
            -3：还未到定时发送阶段
            other：当前发送成功的数据的id号
  * @remark 

  ********************************************************************************************/
int TxQueue_Handle(struct TxQueueStruct *txQueue)
{
    uint8_t tempId,tempIndex=0;
    bool isNeedFree = false;
    enum BlockFreeMethodEnum freeMethod;
    struct PacketStruct packet;
    uint8_t i = 0;
    int result = -1;
    
    /* 没有则直接退出 */
    if (txQueue->_usedBlockQuantity == 0)
    {   return -2; }
    
    /* 发送间隔 txQueue->interval */
    if ((txQueue->__time + txQueue->interval) > SYSTIME)
    {   return result; }
    else
    {   txQueue->__time = SYSTIME;  }

    /* 如果为有序发送，则需要查找最小ID，保证按照时间顺序处理 */
    if(txQueue->isTxUnordered == true)
    {   
        tempId = 255;
        for (i = 0; i < BLOCK_COUNT; i++)
        {
            if (txQueue->__txBlocks[i].flag & TX_FLAG_USED && txQueue->__txBlocks[i].seqId < tempId)
            {
                tempIndex = i;
                tempId = txQueue->__txBlocks[i].seqId;
            }
        }
    }
    
    /* 循环查找可用包，进行发送处理 */
    for (i = tempIndex; i < BLOCK_COUNT; i++)
    {
        if (txQueue->__txBlocks[i].flag & TX_FLAG_USED)
        {
#ifdef TX_BLOCK_TIMEOUT
            /* 发送超时，进入错误处理，并释放发送缓冲块 */
            if ((txQueue->__txBlocks[i].time + TX_TIME_OUT) < SYSTIME && txQueue->__txBlocks[i].flag & TX_FLAG_TIMEOUT)
            {
                isNeedFree = true;
                freeMethod = BlockFree_OverTime; // 超时清除
                goto autoFree;
            }
#endif
            /*在已发送标志位为0，或者重复发送为真时
            进行数据的发送，并置位已发送标志位*/
            if ((txQueue->__txBlocks[i].flag & TX_FLAG_SENDED) == 0 || txQueue->__txBlocks[i].flag & TX_FLAG_RT)
            {
                /* 如果CallBack_PackagBeforeTransmit为空，则直接发送原始报文 
                   不为空的，则将原始报文进行二次打包之后，发送
                   1. 减少缓冲块中占用内存
                   2. 再进行分析时不用再重新拆包解析 */
                if(txQueue->CallBack_PackagBeforeTransmit == NULL || (txQueue->__txBlocks[i].flag & TX_FLAG_PACKAGE) == 0)
                {   
                    if(txQueue->CallBack_Transmit != NULL)
                    {
                        isNeedFree = txQueue->CallBack_Transmit(txQueue->__txBlocks[i].message, txQueue->__txBlocks[i].length);                   // 发送原始报文
                        result = txQueue->__txBlocks[i].id;
                    }
                }           
                else if(txQueue->CallBack_PackagBeforeTransmit != NULL && (txQueue->__txBlocks[i].flag & TX_FLAG_PACKAGE) != 0)
                {
                    if(txQueue->CallBack_PackagBeforeTransmit(&(txQueue->__txBlocks[i]), &packet) == 0) // 二次封包
                    {
                        if(txQueue->CallBack_Transmit != NULL)
                        {
                            isNeedFree = txQueue->CallBack_Transmit(packet.data, packet.length);          // 发送二次封包报文
                            result = txQueue->__txBlocks[i].id;
                        }
                        Free(packet.data);                                              // 释放内存空间
                    }
                }
                else if((txQueue->__txBlocks[i].flag & TX_FLAG_PACKAGE) != 0)
                {   while(1);   }                               // 在此类情况中，会导致无限循环
                
                txQueue->_lastIndex = i;
                txQueue->__txBlocks[i].flag |= TX_FLAG_SENDED; // 标记为已发送
                txQueue->__txBlocks[i].retransCounter++;       // 重发次数递增

                if (isNeedFree == true)
                {
                    freeMethod = BlockFree_TransmitReturn; // 发送函数返回true，则清除
                    goto autoFree;
                }
            }

            /* 为手动清除：则直接跳过
             为自动清除：判断是否为重发
                         非重发：则发送一次应该被自动清除
                         重发：超过重发次数之后被清除*/
            if ((txQueue->__txBlocks[i].flag & TX_FLAG_MC) == 0)
            {
                if ((txQueue->__txBlocks[i].flag & TX_FLAG_RT) == 0)
                {
                    isNeedFree = true;
                    freeMethod = BlockFree_OnceAuto;
                }
                else if (txQueue->__txBlocks[i].retransCounter > txQueue->maxTxCount)
                {
                    isNeedFree = true;
                    freeMethod = BlockFree_OverRetransmit;
                }
            }

        autoFree: /* 根据不同 */
            if (isNeedFree == true)
            {
                if (txQueue->CallBack_AutoClearBlock != NULL)
                {   txQueue->CallBack_AutoClearBlock(&(txQueue->__txBlocks[i]), freeMethod);  }
                TxQueue_FreeBlock(txQueue, txQueue->__txBlocks + i);
            }

            return result;
        }
    }

    return result;
}
/*********************************************************************************************

  * @brief  时间更新
  * @param  txBlock：发送模块结构体指针
            time：时间
  * @return 
  * @remark 

  ********************************************************************************************/
void TxQueue_TimeSync(struct TxQueueStruct *txQueue, uint32_t time)
{
    txQueue->__time = time;
}
/*********************************************************************************************

  * @brief  填充发送结构体
  * @param  txBlock：发送模块结构体指针
            message：报文指针
            length：报文长度
            custom：自定义标志位，参考SimpleBuffer.h中的TX_FLAG
  * @return -1 失败：队列已满
            -3 失败：内存池已满
  * @remark 

  ********************************************************************************************/
int TxQueue_Add(struct TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t mode)
{
    uint16_t i;

    /* 如果没有 */
    if (txQueue->_usedBlockQuantity == 0)
    {   txQueue->seqId = 0; }
    
    for (i = 0; i < BLOCK_COUNT; i++)
    {
        if ((txQueue->__txBlocks[i].flag & TX_FLAG_USED) == 0)
        {
            /* 判断是否已经是申请内存，减少malloc池的重复申请 */
            if((mode & TX_FLAG_IS_MALLOC) == 0)
            {
                txQueue->__txBlocks[i].message = (uint8_t *)Malloc(length * sizeof(uint8_t));
                memset(txQueue->__txBlocks[i].message, 0, length * sizeof(uint8_t));
                if(txQueue->__txBlocks[i].message == NULL)
                {   return -3 ;  }
                memcpy(txQueue->__txBlocks[i].message, message, length);
            }
            else
            {   txQueue->__txBlocks[i].message = message; }
            
            txQueue->__txBlocks[i].length = length;                 // 标记长度
            txQueue->__txBlocks[i].flag |= TX_FLAG_USED;            // 标记为已经占用
            txQueue->__txBlocks[i].seqId = txQueue->seqId;          // 顺序ID
            txQueue->seqId ++;                                      // 顺序ID递增
              
            txQueue->_usedBlockQuantity += 1;                       // 已经使用块计数器 + 1

#ifdef TX_BLOCK_TIMEOUT
            txQueue->__txBlocks[i].time = SYSTIME;
#endif

            /* 可以自定义标志位，自动添加占用标志位，默认只发送一次 */
            txQueue->__txBlocks[i].flag |= mode;

            return i;

        }
    }

    /* 如果为已经申请了，但是未添加，则直接free掉 */
    if ((mode | TX_FLAG_IS_MALLOC) != 0)
    {   Free(message);  }

    return -1;
}
/*********************************************************************************************

  * @brief  填充发送结构体
  * @param  txBlock：发送模块结构体指针
            message：报文指针
            length：报文长度
            custom：自定义标志位，参考SimpleBuffer.h中的TX_FLAG
            id：用于清除的标识
  * @return 返回块Id
  * @remark 

  ********************************************************************************************/
int TxQueue_AddWithId(struct TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t mode, TX_ID_SIZE id)
{
    int16_t blockId = TxQueue_Add(txQueue, message, length, mode);

    if (blockId != -1)
    {   txQueue->__txBlocks[blockId].id = id; }

    return blockId;
}
/*********************************************************************************************

  * @brief  释放发送缓冲
  * @param  txQueue：发送缓冲指针
            txBlock：发送结构体指针
  * @return 
  * @remark 

  ********************************************************************************************/
void TxQueue_FreeBlock(struct TxQueueStruct *txQueue, struct TxBaseBlockStruct *txBlock)
{
    if ((txBlock->flag & TX_FLAG_USED) != 0)
    {
        Free(txBlock->message);

        txQueue->_usedBlockQuantity -= 1;
        txBlock->length = 0;
        txBlock->retransCounter = 0;
        txBlock->id = 0;

#ifdef TX_BLOCK_TIMEOUT
        txBlock->time = 0;
#endif
        txBlock->flag = 0;
    }
}

/*********************************************************************************************

  * @brief  通过自定义函数的方式清除相应发送缓冲块
  * @param  txBlock：发送结构体指针
            func，通过该func里的处理，将对应发送缓冲清除
            *p：自定义函数参数传递
  * @return 
  * @remark 

  ********************************************************************************************/
void TxQueue_FreeByFunc(struct TxQueueStruct *txQueue, bool (*func)(struct TxBaseBlockStruct *, void *), void *para)
{
    uint16_t i;

    for (i = 0; i < BLOCK_COUNT; i++)
    {
        if ((txQueue->__txBlocks[i].flag & TX_FLAG_USED) != 0)
        {
            if (func(txQueue->__txBlocks + i, para) == true)
            {   TxQueue_FreeBlock(txQueue, txQueue->__txBlocks + i);  }
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
void TxQueue_FreeById(struct TxQueueStruct *txQueue, TX_ID_SIZE id)
{
    uint16_t i;

    for (i = 0; i < BLOCK_COUNT; i++)
    {
        if ((txQueue->__txBlocks[i].flag & TX_FLAG_USED) != 0)
        {
            if (txQueue->__txBlocks[i].id == id)
            {   TxQueue_FreeBlock(txQueue, txQueue->__txBlocks + i);   }
        }
    }
}
/*********************************************************************************************

  * @brief  清除指定发送缓冲
  * @param  txBlock：发送结构体指针
            index：需要被清除的索引
  * @return 
  * @remark 

  ********************************************************************************************/
void TxQueue_FreeByIndex(struct TxQueueStruct *txQueue, uint8_t index)
{
    TxQueue_FreeBlock(txQueue, txQueue->__txBlocks + index);
}
/*********************************************************************************************

  * @brief  清除所有缓冲
  * @param  txBlock：发送结构体指针
  * @return 
  * @remark 

  ********************************************************************************************/
void TxQueue_FreeAll(struct TxQueueStruct *txQueue)
{
    for (uint8_t i = 0; i < BLOCK_COUNT; i++)
    {
        if ((txQueue->__txBlocks[i].flag & TX_FLAG_USED) != 0)
        {   TxQueue_FreeBlock(txQueue, txQueue->__txBlocks + i);  }
    }
}

/*********************************************************************************************

  * @brief  基于DMA的接收缓冲处理
  * @param  dmaBuffer：dmaStruct结构体
            buffer：缓冲指针
            bufferSize：缓冲大小
  * @return 
  * @remark 有平台在变量初始化时有可能不会将START END 清零

  ********************************************************************************************/
void DmaBuffer_Init(struct DmaBufferStruct *dmaBuffer, uint8_t *buffer, uint16_t bufferSize)
{
    dmaBuffer->_buffer = buffer;
    dmaBuffer->__bufferLength = bufferSize;
    dmaBuffer->__start = 0;
    dmaBuffer->__end = 0;
}

/*********************************************************************************************

  * @brief  基于DMA的接收处理
  * @param  dmaBuffer：dmaStruct结构体
            remainCount：剩余长度
  * @return 
  * @remark 串口空闲中断接收，包含空闲中断标志位的清除

  ********************************************************************************************/
void DmaBuffer_IdleHandle(struct DmaBufferStruct *dmaBuffer, uint16_t remainCount)
{
    dmaBuffer->__end = dmaBuffer->__bufferLength - remainCount;             // 结束位置计算

    /* 通过判断end与start的位置，进行不同的处理 */
    if (dmaBuffer->__end > dmaBuffer->__start)
    {   RxQueue_Add(&dmaBuffer->_rxQueue, dmaBuffer->_buffer + dmaBuffer->__start, dmaBuffer->__end - dmaBuffer->__start, false);  }
    else if (dmaBuffer->__end < dmaBuffer->__start)
    {
        uint8_t *message = (uint8_t *)Malloc(dmaBuffer->__bufferLength - dmaBuffer->__start + dmaBuffer->__end);
        memset(message, 0, dmaBuffer->__bufferLength - dmaBuffer->__start + dmaBuffer->__end);
        
        if(message != NULL)
        {
            memcpy(message,
                   dmaBuffer->_buffer + dmaBuffer->__start,
                   dmaBuffer->__bufferLength - dmaBuffer->__start);

            memcpy(message + dmaBuffer->__bufferLength - dmaBuffer->__start,
                   dmaBuffer->_buffer,
                   dmaBuffer->__end + 1);

            RxQueue_Add(&dmaBuffer->_rxQueue,
                        message,
                        dmaBuffer->__bufferLength - dmaBuffer->__start + dmaBuffer->__end, 
                        true);
        }
        else
        {
            if(dmaBuffer->CallBack_MallocFail != NULL)
            {   dmaBuffer->CallBack_MallocFail();   }
        }
    }

    dmaBuffer->__start = dmaBuffer->__end;
}
