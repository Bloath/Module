#ifndef _DCMOTOR_H_
#define _DCMOTOR_H_
/* Includes ------------------------------------------------------------------*/
#include "Module/Module.h"
/* typedef -------------------------------------------------------------------*/
enum DCMotorOperation
{
    DCMotor_Idle = 0,
    DCMotor_Positive,
    DCMotor_Negative
};

struct DCMotorStruct
{
    uint32_t __time;
    struct ProcessStruct process;
    
    enum DCMotorOperation nextOperation;
    enum DCMotorOperation currentOperation;
    enum DCMotorOperation _lastOperation;
      
    bool isBanned;
      
    void (*CallBack_Init)();
    void (*CallBack_Operation)(enum DCMotorOperation);
    void (*CallBack_BeforeRun)(struct DCMotorStruct*);
    bool (*CallBack_AfterRun)(struct DCMotorStruct*);
    bool (*CallBack_StopCondition)(struct DCMotorStruct*);
    void (*CallBack_Finish)(struct DCMotorStruct*);
};
/* define --------------------------------------------------------------------*/
/* macro ---------------------------------------------------------------------*/
/* variables -----------------------------------------------------------------*/
/* function prototypes -------------------------------------------------------*/
/* functions -----------------------------------------------------------------*/
int DCMotor_SetNextOperation(struct DCMotorStruct *motor, enum DCMotorOperation operation, bool isForced);
void DCMotor_Handle(struct DCMotorStruct *motor);
bool DCMotor_Status(struct DCMotorStruct *motor, enum DCMotorOperation operation);

#endif
