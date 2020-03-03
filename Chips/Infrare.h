#ifndef _INFRARERX_H_
#define _INFRARERX_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"

/* Public typedef ------------------------------------------------------------*/
#define INFRARE_MAX_COUNT   40

struct InfrareStruct
{
    uint16_t trigInteval[INFRARE_MAX_COUNT];    // 触发间隔 
    uint16_t counter;
    uint32_t data;
    uint32_t __time;                            // 记录触发起始时间
    uint32_t *trigTimer;                        // 触发时长记录，单位为us
    bool isTrigged;
  
    void (*CallBack_GetSignal)(uint8_t user, uint8_t operation);
};

/* Public define -------------------------------------------------------------*/

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void Infrare_Trig(struct InfrareStruct *infrareObj);
void Infrare_Handle(struct InfrareStruct *infrareObj);
char Infrare_Decode(uint8_t operation);
#endif
