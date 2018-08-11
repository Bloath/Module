#ifndef _NB_API_H_
#define _NB_API_H_

/* Includes ------------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
typedef enum
{
  NbSignal_Undetected,
  NbSignal_Weak,
  NbSignal_Normal,  
}NbSignalStrengthEnum;

typedef enum
{
  NbError_WeakSignal = 0,
  NbError_Undetected,
}NbErrorEnum;

typedef struct
{
  uint32_t time;
  uint8_t errorCounter;
  NbSignalStrengthEnum signal;
  ProcessEnum process;
}NbStruct;
/* Public variables ----------------------------------------------------------*/
extern TxQueueStruct NB_TxQueue;                      // ģ��ķ��ͻ���
extern RxQueueStruct NB_RxQueue;                      // ģ��Ľ��ջ���
/* Public function prototypes ------------------------------------------------*/
void NB_Handle();
void NB_RxHandle(uint8_t *packet, uint16_t len, void *param);

void NB_StringTrans(const char *string);
void NB_DataTrans(uint8_t *message, uint16_t len);
#endif