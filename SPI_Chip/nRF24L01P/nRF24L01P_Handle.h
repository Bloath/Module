#ifndef _nRF24L01P_HANDLE_
#define _nRF24L01P_HANDLE_

/* Includes ------------------------------------------------------------------*/
#include "../../UartDma/SimpleBuffer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
extern TxQueueStruct nRF24L01_TxQueue;
extern RxQueueStruct nRF24L01_RxQueue;

/* Private function prototypes ------------------------------------------------*/
void nRF24L01P_Initialization();
uint8_t nRF24L01P_Receive();
void nRF24L01P_Transimit(uint8_t *packet, uint16_t len);
void nRF24L01P_SetRxMode(void);

#endif

