#ifndef _IOLIB_H_
#define _IOLIB_H_
/* Includes ------------------------------------------------------------------*/
#include "../Module_Conf.h"

/* Public typedef ------------------------------------------------------------*/
typedef struct
{
    bool isActive;
    uint8_t dutyRatio;                  // 占空比
    uint16_t totalInterval;             // 总时间
    uint16_t keepInterval;              // 持续时间
}IOPwmStatusStruct;

typedef struct
{
    ProcessStruct process;
    IOPwmStatusStruct defaultStatus;    // 默认状态，为非使能
    IOPwmStatusStruct currentStatus;    // 当前状态，当超出keepInterval之后切换为默认状态
    uint32_t __runTime;
    uint32_t startTime;
    
    void (*CallBack_Init)();
    void (*CallBack_IOOperation)(bool isActive);
    bool (*CallBack_IsIOActive)();
}IOPwmStruct;

typedef struct
{
	uint32_t __time;
	bool _isTrigged;
	
	bool (*CallBack_IsKeyPressed)();					// 检查是否被按下，必填
	void (*CallBack_KeyHandle)(uint32_t interval);	// 抬起后得处理
}KeyDetectStruct;

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern IOPwmStruct ioPwm;
/* Public function prototypes ------------------------------------------------*/
void IOPwm_Handle(IOPwmStruct *ioPwm);
void IOPwm_ChangeStatus(IOPwmStruct *ioPwm, IOPwmStatusStruct *status);
void IOPwm_ChangeDefault(IOPwmStruct *ioPwm, IOPwmStatusStruct *status);
void IOPwm_StatusModify(IOPwmStatusStruct *status, bool isActive, uint8_t dutyRatio, uint16_t totalInterval, uint16_t keepInterval);

void KeyDetect_PressCheck(KeyDetectStruct *key);
void KeyDetect_Handle(KeyDetectStruct *key);
bool KeyDetect_IsKeyPressed(KeyDetectStruct *key);
#endif