/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "../Module/Common/Malloc.h"
#include "SimpleBuffer.h"
#include "SimpleBuffer_Handle.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void TxBlockErrorHandle(TxBlockError error);

/* Private functions ---------------------------------------------------------*/
/*********************************************************************************************

  * @brief  ���յ����ֽ�
  * @param  data:       ���յĵ��ֽ�
  * @param  rxBuffer   ���ջ���ṹ��
  * @retval ��
  * @remark 

  ********************************************************************************************/
void ReceiveSingleByte(uint8_t data, RxBufferStruct *rxBuffer)
{
  rxBuffer->buffer[rxBuffer->count] = data;         //���뻺�� 
  rxBuffer->count ++;                               //����������
  if(rxBuffer->count >= BUFFER_LENGTH)
  { BufferOverFlow(); }
}

/*********************************************************************************************

  * @brief  �������ڵ�������д�����Ķ�����
  * @param  rxBlockList��   ���ջ���ṹ��
            packet��   Ҫ������տ�����ݰ�ָ��
            len������
  * @retval ��
  * @remark ���������кţ����Ϊ0xFFFF��Ϊʧ��

  ********************************************************************************************/
uint16_t RxQueue_Add(RxQueueStruct *rxQueue, uint8_t *packet, uint16_t Len)
{
  uint8_t i = 0;
  
  if(Len == 0)
  { return 0xFFFF; }
  
  /* �ҵ����л��壬���� */
  for(i=0; i<BLOCK_COUNT; i++)
  {
    if(!(rxQueue->rxBlocks[i].flag & RX_FLAG_USED))                                      //���ҿ��б��Ķ���
    {
      rxQueue->rxBlocks[i].flag |= RX_FLAG_USED;                                              //���Ŀ�ʹ�ñ�־λ��λ
    
      /* �����ڴ沢��д */
      rxQueue->rxBlocks[i].message = (uint8_t*)Malloc((Len + 1) * sizeof(uint8_t));         //���ݻ��峤�������ڴ棬��һ���ֽڣ�������д�ַ���ֹͣ��
      memcpy(rxQueue->rxBlocks[i].message, packet, Len);  
      rxQueue->rxBlocks[i].message[Len] = 0;              // ��ӽ��������û������������ַ������� 
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

  * @brief  �������ڵ�������д�����Ķ�����
  * @param  rxBlockList��           ���ջ���ṹ��
  * @param  rxBlock��       ���Ľ��տ�
  * @param  *f��         ���ջ���ṹ��
  * @retval ��
  * @remark 

  ********************************************************************************************/
void RxQueue_Handle(RxQueueStruct *rxQueue, void (*f)(uint8_t*, uint16_t))
{
  for(uint16_t i=0; i<BLOCK_COUNT; i++)
  {
    if(rxQueue->rxBlocks[i].flag & RX_FLAG_USED)                     //������Ҫ����ı���
    {
      (*f)(rxQueue->rxBlocks[i].message, rxQueue->rxBlocks[i].length);
      
      Free(rxQueue->rxBlocks[i].message);                             //�ͷ�������ڴ�
    
      rxQueue->rxBlocks[i].flag &= ~RX_FLAG_USED;                    //�����ʹ�ñ�־λ
      rxQueue->usedBlockQuantity -= 1;
    }
  }
}

/*********************************************************************************************
  * @brief  ���ͻ��崦��
  * @param  txBlock�����ͻ���飬���ͻ������ͷ
  * @param  Transmit�����ͺ���ָ�루���õײ㷢�ͺ�����
  * @param  interval�����ͻ��庯�����ü��ʱ��
  * @retval ��
  * @remark 
  ********************************************************************************************/
void TxQueue_Handle(TxQueueStruct *txQueue, void (*Transmit)(uint8_t*, uint16_t), uint32_t interval)
{
  uint16_t i;
  
  if((txQueue->time + interval) > sysTime)
  { return; }
  else
  {  txQueue->time = sysTime; }
  
  for(i=0; i<BLOCK_COUNT; i++)
  {
      if(txQueue->txBlocks[i].flag & TX_FLAG_USED)            
      {
        
#ifdef TX_BLOCK_TIMEOUT
          /* ���ͳ�ʱ��������������ͷŷ��ͻ���� */
          if((txQueue->txBlocks[i].time + TX_TIME_OUT) < sysTime && txQueue->txBlocks[i].flag & TX_FLAG_TIMEOUT)
          {
            TxBlockErrorHandle(TxBlockError_TimeOut);
            TxQueue_FreeBlock(txQueue->txBlock + i);
            txQueue->usedBlockQuantity -= 1;
            continue;
          }
#endif
        
          /*���ѷ��ͱ�־λΪ0�������ظ�����Ϊ��ʱ
            �������ݵķ��ͣ�����λ�ѷ��ͱ�־λ*/
          if(!(txQueue->txBlocks[i].flag & TX_FLAG_SENDED) || txQueue->txBlocks[i].flag & TX_FLAG_RT)
          {
            Transmit(txQueue->txBlocks[i].message, txQueue->txBlocks[i].length);             //��������
            txQueue->txBlocks[i].flag |= TX_FLAG_SENDED;
            //break;              //ע�����λ��
          }
        
          txQueue->txBlocks[i].retransCounter ++;                            //�ط���������
          
          /* ���ֶ���� �� (�ط�����200�� ���� ����Ҫ�ط�) �������
             ��������ͷ�ģ�� */
          if(!(txQueue->txBlocks[i].flag & TX_FLAG_MC) 
             && (txQueue->txBlocks[i].retransCounter > 200 || !(txQueue->txBlocks[i].flag & TX_FLAG_RT)))
          {
            TxQueue_FreeBlock(&(txQueue->txBlocks[i]));
            txQueue->usedBlockQuantity -= 1;
          }  
      }
  }
}

/*********************************************************************************************

  * @brief  ��䷢�ͽṹ��
  * @param  txBlock������ģ��ṹ��ָ��
  * @param  message������ָ��
  * @param  length�����ĳ���
  * @param  custom���Զ����־λ���ο�SimpleBuffer.h�е�TX_FLAG
  * @return 
  * @remark 

  ********************************************************************************************/
uint16_t TxQueue_Add(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t custom)
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
      
      /* �����Զ����־λ���Զ����ռ�ñ�־λ��Ĭ��ֻ����һ�� */
      txQueue->txBlocks[i].flag |= custom;
      
      return i;
      //break;
    }
  }
  
  if(i == BLOCK_COUNT)
  { TxBlock_ErrorHandle(txQueue, BlockFull); }
  
  return 0xFFFF;
}
/*********************************************************************************************

  * @brief  ��䷢�ͽṹ��
  * @param  txBlock������ģ��ṹ��ָ��
  * @param  message������ָ��
  * @param  length�����ĳ���
  * @param  custom���Զ����־λ���ο�SimpleBuffer.h�е�TX_FLAG
  * @param  id����������ı�ʶ
  * @return 
  * @remark 

  ********************************************************************************************/
uint16_t TxQueue_AddWithId(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t custom, TX_ID_SIZE id)
{
  uint16_t blockId = TxQueue_Add(txQueue, message, length, custom);
  
  if(blockId != 0xFFFF)
  { txQueue->txBlocks[blockId].id = id; }
  
  return blockId;
}
/*********************************************************************************************

  * @brief  �ͷŷ��ͻ���
  * @param  txBlock�����ͽṹ��ָ��
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

  * @brief  ͨ���Զ��庯���ķ�ʽ�����Ӧ���ͻ����
  * @param  txBlock�����ͽṹ��ָ��
            func��ͨ����func��Ĵ�������Ӧ���ͻ������
            *p���Զ��庯����������
  * @return 
  * @remark 

  ********************************************************************************************/
void TxQueue_FreeByFunc(TxQueueStruct *txQueue, uint8_t (*func)(uint8_t*, uint16_t, void*), void *p)
{
  uint16_t i;
  
  for(i=0; i<BLOCK_COUNT; i++)
  { 
    if((txQueue->txBlocks[i].flag & TX_FLAG_USED) != 0)
    {
      if(!func(txQueue->txBlocks[i].message, txQueue->txBlocks[i].length, p))
      { 
        TxQueue_FreeBlock(txQueue->txBlocks + i); 
        txQueue->usedBlockQuantity -= 1;
      }
    }
  }
}
/*********************************************************************************************

  * @brief  ���ָ�����ͻ���
  * @param  txBlock�����ͽṹ��ָ��
            id��ͨ���Զ����ʶ�ķ�ʽ�ͷŷ��ͻ���
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
