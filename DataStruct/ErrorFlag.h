#ifndef _ERRORFLAG_H_
#define _ERRORFLAG_H_
/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"
#include "Module/Common/Common.h"

/* Public typedef ------------------------------------------------------------*/
struct ErrorFlagStruct
{
    uint32_t flag;
    uint32_t __flagCache;
    
    void (*CallBack_ErrorTrigged)(struct ErrorFlagStruct *error, uint16_t index, void *param);
    void (*CallBack_ErrorClear)(struct ErrorFlagStruct *error, uint16_t index, void *param);
};
/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
#define ERRORFLAG_IS_TRIGGED(errorPointer, index)   (FLAG_IS_SET((errorPointer)->flag, (1<<index)))
#define ERRORFLAG_TRIG(errorPointer, index)         FLAG_SET((errorPointer)->flag, (1<<index))
#define ERRORFLAG_CLEAR(errorPointer, index)        FLAG_CLR((errorPointer)->flag, (1<<index))
#define ERRORFLAG_INDEX2FLAG(index)                 (1<<index) 
    
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void ErrorFlag_Handle(struct ErrorFlagStruct *error, void *param);
void ErrorFlag_TrigWithParameter(struct ErrorFlagStruct *error, uint16_t index, void *param);
void ErrorFlag_ClearWithParameter(struct ErrorFlagStruct *error, uint16_t index, void *param);
#endif