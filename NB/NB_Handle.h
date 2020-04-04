#ifndef _NB_HANDLE_H_
#define _NB_HANDLE_H_

/* Includes ------------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void NB_Http_StartConnect();
void NB_HttpGet_ReceiveHandle(char *message, uint16_t len);
int NB_HttpGet_PacketPackage(struct TxBaseBlockStruct *block, struct PacketStruct *packet);
bool NB_HttpPost_Transmit(struct TxBaseBlockStruct *txBlock);


void NB_OC_HandleBeforeNetting();
void NB_OC_ReceiveHandle(char *message, uint16_t len);
int NB_OC_PacketPackage(struct TxBaseBlockStruct *block, struct PacketStruct *packet);
#endif