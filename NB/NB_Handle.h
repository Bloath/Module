#ifndef _NB_HANDLE_H_
#define _NB_HANDLE_H_

/* Includes ------------------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void NB_Http_StartConnect();
void NB_Http_ReceiveHandle(char *message, uint16_t len);
int NB_Http_PacketPackage(TxBaseBlockStruct *block, void *param, PacketStruct *packet);

 void NB_OC_HandleBeforeNetting();
void NB_OC_ReceiveHandle(char *message, uint16_t len);
int NB_OC_PacketPackage(TxBaseBlockStruct *block, void *param, PacketStruct *packet);
#endif