#ifndef _IOLIB_H_
#define _IOLIB_H_
/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"

/* Public typedef ------------------------------------------------------------*/
struct IOPwmStatusStruct
{
    uint16_t activeInterval;            // 使能时间
    uint16_t totalInterval;             // 总时间 ms
    int16_t keepInterval;               // 持续时间 s, 为-1则为一直保持
};

struct IOPwmStruct
{
    struct ProcessStruct process;
    struct IOPwmStatusStruct defaultStatus;    // 默认状态，为非使能
    struct IOPwmStatusStruct currentStatus;    // 当前状态，当超出keepInterval之后切换为默认状态
    uint32_t __runTime;
    uint32_t startTime;
    
    void (*CallBack_Init)();
    void (*CallBack_IOOperation)(bool isActive);
    bool (*CallBack_IsIOActive)();
};

struct KeyDetectStruct
{
	uint32_t __time;
    struct GpioStruct io;
	bool _isTrigged;
    uint8_t id;
	
	bool (*CallBack_IsKeyPressed)(struct KeyDetectStruct*);				// 检查是否被按下，必填
	void (*CallBack_KeyRaiseHandle)(struct KeyDetectStruct*, uint32_t );	    // 抬起后得处理
};

struct OperationKeep
{
    uint32_t time;
    uint32_t *referenceTime;
    void (*CallBack_IOOperation)(bool isActive);
    uint16_t interval;
    bool isRunning;
};

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
void IOPwm_Handle(struct IOPwmStruct *ioPwm);
void IOPwm_ChangeStatus(struct IOPwmStruct *ioPwm, struct IOPwmStatusStruct *status);
void IOPwm_ChangeDefault(struct IOPwmStruct *ioPwm, struct IOPwmStatusStruct *status);
void IOPwm_StatusModify(struct IOPwmStatusStruct *status, uint8_t dutyRatio, uint16_t totalInterval, uint16_t keepInterval);
bool IOPwm_IsInDefault(struct IOPwmStruct *ioPwm);
bool IOPwm_IsIdle(struct IOPwmStruct *ioPwm);

void KeyDetect_PressCheck(struct KeyDetectStruct *key, bool isInterrupted);
void KeyDetect_Handle(struct KeyDetectStruct *key);

void OperationKeep_Start(struct OperationKeep *operation, uint32_t interval);
void OperationKeep_Handle(struct OperationKeep *operation);
#endif