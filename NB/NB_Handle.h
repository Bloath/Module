#ifndef _NB_HANDLE_H_
#define _NB_HANDLE_H_

/* Includes ------------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
bool NB_Http_Transmit(struct TxUnitStruct *unit, void *param);
void NB_Http_ReceiveHandle(struct RxUnitStruct *unit, void *param);
void NBSota_RxHandle(struct RxUnitStruct *unit, void *param);
int NBSota_Start(struct NBSotaStruct *sota);
#endif