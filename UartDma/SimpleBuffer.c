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
void ReceiveSingleByte(uint8_t data, RxBufferTypeDef *rxBuffer)
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
uint16_t FillRxBlock( RxBlockTypeDef *rxBlock, uint8_t *packet, uint16_t Len)
{
  uint8_t i = 0;
  
  if(Len == 0)
  { return 0xFFFF; }
  
  /* �ҵ����л��壬���� */
  for(i=0; i<RX_BLOCK_COUNT; i++)
  {
    if(!(rxBlock[i].flag & RX_FLAG_USED))                                      //���ҿ��б��Ķ���
    {
      rxBlock[i].flag |= RX_FLAG_USED;                                              //���Ŀ�ʹ�ñ�־λ��λ
    
      rxBlock[i].message = (uint8_t*)Malloc((Len + 1) * sizeof(uint8_t));         //���ݻ��峤�������ڴ棬��һ���ֽڣ�������д�ַ���ֹͣ��
      
      memcpy(rxBlock[i].message, packet, Len);  
      
      rxBlock[i].message[Len] = 0;              // ��ӽ��������û������������ַ������� 
      
      rxBlock[i].length = Len; 
      
      return i;
      //break;
    }
  }
  
  if(i == RX_BLOCK_COUNT)
  { RxBlock_ErrorHandle(rxBlock, BlockFull); }
  
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
void RxBlockListHandle(RxBlockTypeDef *rxBlock, void (*f)(uint8_t*, uint16_t))
{
  for(uint16_t i=0; i<RX_BLOCK_COUNT; i++)
  {
    if(rxBlock[i].flag & RX_FLAG_USED)                     //������Ҫ����ı���
    {
      (*f)(rxBlock[i].message, rxBlock[i].length);
      
      Free(rxBlock[i].message);                             //�ͷ�������ڴ�
    
      rxBlock[i].flag &= ~RX_FLAG_USED;                    //�����ʹ�ñ�־λ
    }
  }
}

/*********************************************************************************************
  * @brief  ���߷��ͻ��崦��
  * @param  txBlock�����ͻ���飬���ͻ������ͷ
  * @param  Transmit�����ͺ���ָ�루���õײ㷢�ͺ�����
  * @param  interval�����ͻ��庯�����ü��ʱ��
  * @retval ��
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
        
#ifdef TX_BLOCK_TIMEOUT
          /* ���ͳ�ʱ��������������ͷŷ��ͻ���� */
          if((txBlock[i].time + TX_TIME_OUT) < sysTime && txBlock[i].flag & TX_FLAG_TIMEOUT)
          {
            TxBlockErrorHandle(TxBlockError_TimeOut);
            FreeTxBlock(&txBlock[i]);
            continue;
          }
#endif
        
          /*���ѷ��ͱ�־λΪ0�������ظ�����Ϊ��ʱ
            �������ݵķ��ͣ�����λ�ѷ��ͱ�־λ*/
          if(!(txBlock[i].flag & TX_FLAG_SENDED) || txBlock[i].flag & TX_FLAG_RT)
          {
            Transmit(txBlock[i].message, txBlock[i].length);             //��������
            txBlock[i].flag |= TX_FLAG_SENDED;
            //break;              //ע�����λ��
          }
        
          txBlock[i].retransCounter ++;                            //�ط���������
          
          /* ���ֶ���� �� (�ط�����200�� ���� ����Ҫ�ط�) �������
             ��������ͷ�ģ�� */
          if(!(txBlock[i].flag & TX_FLAG_MC) 
             && (txBlock[i].retransCounter > 200 || !(txBlock[i].flag & TX_FLAG_RT)))
          {
            FreeTxBlock(&txBlock[i]);
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
uint16_t FillTxBlock(TxBlockTypeDef *txBlock, uint8_t *message, uint16_t length, uint8_t custom)
{
  uint16_t i;
  
  for(i=0; i<TX_BLOCK_COUNT; i++)
  { 
    if((txBlock[i].flag & TX_FLAG_USED) == 0)
    {
      txBlock[i].message = (uint8_t*)Malloc(length * sizeof(uint8_t));

      memcpy(txBlock[i].message, message, length);
      txBlock[i].length = length;
      txBlock[i].flag |= TX_FLAG_USED;
      
#ifdef TX_BLOCK_TIMEOUT
      txBlock[i].time = sysTime;
#endif
      
      /* �����Զ����־λ���Զ����ռ�ñ�־λ��Ĭ��ֻ����һ�� */
      txBlock[i].flag |= custom;
      
      return i;
      //break;
    }
  }
  
  if(i == TX_BLOCK_COUNT)
  { TxBlock_ErrorHandle(txBlock, BlockFull); }
  
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
uint16_t FillTxBlockWithId(TxBlockTypeDef *txBlock, uint8_t *message, uint16_t length, uint8_t custom, TX_ID_SIZE id)
{
  uint16_t blockId = FillTxBlock(txBlock, message, length, custom);
  
  if(blockId != 0xFFFF)
  { txBlock[blockId].id = id; }
  
  return blockId;
}
/*********************************************************************************************

  * @brief  �ͷŷ��ͻ���
  * @param  txBlock�����ͽṹ��ָ��
  * @return 
  * @remark 

  ********************************************************************************************/
void FreeTxBlock(TxBlockTypeDef *txBlock)
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
void FreeTxBlockByFunc(TxBlockTypeDef *txBlock, uint8_t (*func)(uint8_t*, uint16_t, void*), void *p)
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

  * @brief  ���ָ�����ͻ���
  * @param  txBlock�����ͽṹ��ָ��
            id��ͨ���Զ����ʶ�ķ�ʽ�ͷŷ��ͻ���
  * @return 
  * @remark 

  ********************************************************************************************/
void FreeTxBlockById(TxBlockTypeDef *txBlock, TX_ID_SIZE id)
{
  uint16_t i;
  
  for(i=0; i<TX_BLOCK_COUNT; i++)
  { 
    if((txBlock[i].flag & TX_FLAG_USED) != 0)
    {
      if(txBlock[i].id == id)
      { FreeTxBlock(txBlock + i); }
    }
  }
}
/*********************************************************************************************

  * @brief  �ж����ݰ�
  * @param  srcPacket��Դ���ݰ�ָ��
  * @param  desPacket��Ŀ�����ݰ�ָ��
  * @param  length������
  * @return 
  * @remark �ж����ݰ��Ƿ���ͬ����ͬΪ0����ͬΪ1

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