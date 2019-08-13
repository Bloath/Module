#ifndef _DCMOTOR_H_
#define _DCMOTOR_H_
/* Includes ------------------------------------------------------------------*/
#include "Module/Module.h"
/* typedef -------------------------------------------------------------------*/
typedef enum
{
    DCMotor_Idle = 0,
    DCMotor_Positive,
    DCMotor_Negative
}DCMotorOperation;

typedef struct __DCMotorStruct
{
    uint32_t __time;
    ProcessStruct process;
    
    DCMotorOperation nextOperation;
    DCMotorOperation currentOperation;
    DCMotorOperation _lastOperation;
      
    bool isBanned;
      
    void (*CallBack_Init)();
    void (*CallBack_Operation)(DCMotorOperation);
    void (*CallBack_BeforeRun)(struct __DCMotorStruct*);
    bool (*CallBack_AfterRun)(struct __DCMotorStruct*);
    bool (*CallBack_StopCondition)(struct __DCMotorStruct*);
    void (*CallBack_Finish)(struct __DCMotorStruct*);
}DCMotorStruct;
/* define --------------------------------------------------------------------*/
/* macro ---------------------------------------------------------------------*/
/* variables -----------------------------------------------------------------*/
/* function prototypes -------------------------------------------------------*/
/* functions -----------------------------------------------------------------*/
int DCMotor_SetNextOperation(DCMotorStruct *motor, DCMotorOperation operation, bool isForced);
void DCMotor_Handle(DCMotorStruct *motor);
bool DCMotor_Status(DCMotorStruct *motor, DCMotorOperation operation);

#endif
