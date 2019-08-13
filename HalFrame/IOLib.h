#ifndef _IOLIB_H_
#define _IOLIB_H_
/* Includes ------------------------------------------------------------------*/
#include "../Module_Conf.h"

/* Public typedef ------------------------------------------------------------*/
typedef struct
{
    uint16_t activeInterval;            // 使能时间
    uint16_t totalInterval;             // 总时间 ms
    int16_t keepInterval;               // 持续时间 s, 为-1则为一直保持
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

typedef struct __KeyDetectStruct
{
	uint32_t __time;
	bool _isTrigged;
	
	bool (*CallBack_IsKeyPressed)(struct __KeyDetectStruct*);				// 检查是否被按下，必填
	void (*CallBack_KeyHandle)(struct __KeyDetectStruct*, uint32_t );	    // 抬起后得处理
}KeyDetectStruct;

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void IOPwm_Handle(IOPwmStruct *ioPwm);
void IOPwm_ChangeStatus(IOPwmStruct *ioPwm, IOPwmStatusStruct *status);
void IOPwm_ChangeDefault(IOPwmStruct *ioPwm, IOPwmStatusStruct *status);
void IOPwm_StatusModify(IOPwmStatusStruct *status, uint8_t dutyRatio, uint16_t totalInterval, uint16_t keepInterval);
bool IOPwm_IsInDefault(IOPwmStruct *ioPwm);
bool IOPwm_IsIdle(IOPwmStruct *ioPwm);

void KeyDetect_PressCheck(KeyDetectStruct *key, bool isInterrupted);
void KeyDetect_Handle(KeyDetectStruct *key);
bool KeyDetect_IsKeyPressed(KeyDetectStruct *key);
#endif