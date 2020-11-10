#ifndef _NB_HANDLE_H_
#define _NB_HANDLE_H_

/* Includes ------------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* ÏÂÔØ½×¶ÎµÄ´íÎó´úÂë  */
#define NBSOTA_DOWNLOAD_FINISH              101
#define NBSOTA_DOWNLOAD_NO_MEMORY           102
#define NBSOTA_DOWNLOAD_OVER_RAM            103
#define NBSOTA_DOWNLOAD_TIMEOUT             104
#define NBSOTA_DOWNLOAD_LACK_POWER          105
#define NBSOTA_DOWNLOAD_SIGNAL_LOSE         106
#define NBSOTA_DOWNLOAD_UNDEFINED           107

#define NBSOTA_UPDATE_UPDATE_FINISH         201
#define NBSOTA_UPDATE_LACK_POWER            202
#define NBSOTA_UPDATE_OVER_RAM              203
#define NBSOTA_UPDATE_CANT_MATCH            204
#define NBSOTA_UPDATE_MD5_WRONG             205
#define NBSOTA_UPDATE_UNDEFINED             206
#define NBSOTA_UPDATE_OVER_RETRY            207
#define NBSOTA_UPDATE_OVERDUE               208
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
bool NB_Http_Transmit(struct TxUnitStruct *unit, void *param);
void NBSota_RxHandle(struct RxUnitStruct *unit, void *param);
int NBSota_Start(struct NBSotaStruct *sota);
void NBIdCheck_RxHandle(struct RxUnitStruct *unit, void *param);
void NBSota_Init(struct NBSotaStruct *sota, const char *auth, const char *version);
void NBSota_ErrorHandle(struct NBSotaStruct *sota);
void NBSocket_clear(struct NBSocketStruct *socket);
#endif