#ifndef _nRF24L01P_HANDLE_
#define _nRF24L01P_HANDLE_

/* Includes ------------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
void nRF24L01P_Initialization();
uint8_t nRF24L01P_Receive(uint8_t *data, uint8_t *len);
void nRF24L01P_Transimit(uint8_t *packet, uint16_t len);
void nRF24L01P_SetRxMode(void);

#endif

