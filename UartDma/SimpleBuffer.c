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
}

/*********************************************************************************************

  * @brief  �������ڵ�������д�����Ķ�����
  * @param  rxBlockList��   ���ջ���ṹ��
            packet��   Ҫ������տ�����ݰ�ָ��
            len������
  * @retval ��
  * @remark 

  ********************************************************************************************/
void FillRxBlock( RxBlockTypeDef *rxBlock, uint8_t *packet, uint16_t Len)
{
  if(Len == 0)
  { return; }
  
  /* �ҵ����л��壬���� */
  for(uint16_t i=0; i<RX_BLOCK_COUNT; i++)
  {
    if(!(rxBlock[i].flag & RX_FLAG_USED))                                      //���ҿ��б��Ķ���
    {
      rxBlock[i].flag |= RX_FLAG_USED;                                              //���Ŀ�ʹ�ñ�־λ��λ

#ifdef DYNAMIC_MEMORY      
      rxBlock[i].message = (uint8_t*)malloc((Len + 1) * sizeof(uint8_t));         //���ݻ��峤�������ڴ棬��һ���ֽڣ�������д�ַ���ֹͣ��
#endif
      
      memcpy(rxBlock[i].message, packet, Len);  
      
      rxBlock[i].message[Len] = 0;              // ��ӽ��������û������������ַ������� 
      
      rxBlock[i].length = Len; 
      break;
    }
  }
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
      
#ifdef DYNAMIC_MEMORY  
      free(rxBlock[i].message);                             //�ͷ�������ڴ�,����
#endif      
    
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
          /* ���ͳ�ʱ��������������ͷŷ��ͻ���� */
          if((txBlock[i].time + TX_TIME_OUT) < sysTime && txBlock[i].flag & TX_FLAG_TIMEOUT)
          {
            TxBlockErrorHandle(TxBlockError_TimeOut);
            FreeTxBlock(&txBlock[i]);
            continue;
          }
        
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
  * @return 
  * @remark 

  ********************************************************************************************/
void TxBlockErrorHandle(TxBlockError error)
{

  
}

/*********************************************************************************************

  * @brief  ��䷢�ͽṹ��
  * @param  txBlock������ģ��ṹ��ָ��
  * @param  message������ָ��
  * @param  length�����ĳ���
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

      /* �����Զ����־λ���Զ����ռ�ñ�־λ��Ĭ��ֻ����һ�� */
      txBlock[i].flag |= custom;
      
      break;
    }
    
  }
  
  return (i==TX_BLOCK_COUNT)?1:0;
}
/*********************************************************************************************

  * @brief  �ͷŷ��ͻ���
  * @param  txBlock�����ͽṹ��ָ��
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

  * @brief  ���ָ�����ͻ���
  * @param  txBlock�����ͽṹ��ָ��
            func��ͨ����func��Ĵ�������Ӧ���ͻ������
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