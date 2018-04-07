#ifndef _nRF24L01P_HANDLE_
#define _nRF24L01P_HANDLE_

/* Includes ------------------------------------------------------------------*/
#include "../../BufferQueue/BufferQueue.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
extern TxQueueStruct nRF24L01_TxQueue;
extern RxQueueStruct nRF24L01_RxQueue;

/* Private function prototypes ------------------------------------------------*/
BoolEnum nRF24L01P_Initialization();
uint8_t nRF24L01P_Receive();
void nRF24L01P_Transimit(uint8_t *packet, uint16_t len);
void nRF24L01P_SetRxMode(void);
void nRF24L01P_PowerDown(void);
void nRF24L01P_PowerOn(void);

#endif

