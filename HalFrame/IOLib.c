/* Includes ------------------------------------------------------------------*/
#include "IOLib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*********************************************************************************************

  * @brief  IoPwm_RunAsSpecify
  * @param  ioPwm：ioPwm实例指针
            status：状态
  * @return 
  * @remark -1：到时间了，没有新的时间进入
            0：运行期间
            1：到时瞬间

  ********************************************************************************************/
int IoPwm_RunAsSpecify(IOPwmStruct *ioPwm, IOPwmStatusStruct *status)
{
     /* 持续时间为0则报-1，一般默认的才会为0 */
    if(status->keepInterval == 0)
    {   return -1;  }
    
     /* 超时则报-1 */
    if((ioPwm->startTime + status->keepInterval) < realTime && status->keepInterval != -1)
    {   
        status->keepInterval = 0;               // 持续时间置0
        ioPwm->CallBack_IOOperation(false);     // 发现超时则直接非使能
        return 1;  
    }
  
    /* 计算出占空比 */
    uint16_t toggleInterval = (ioPwm->CallBack_IsIOActive() == false)? (status->totalInterval - status->activeInterval): status->activeInterval;

    
    /* 按照占空比实际处理*/
    if((ioPwm->__runTime + toggleInterval) < sysTime)
    {
        ioPwm->__runTime = sysTime;     
        ioPwm->CallBack_IOOperation((ioPwm->CallBack_IsIOActive() == true)? false:true);       // 根据IO状态反转切换
    }
    return 0;
}

/*********************************************************************************************

  * @brief  IOPwm_Handle
  * @param   
  * @return 
  * @remark 

  ********************************************************************************************/
void IOPwm_Handle(IOPwmStruct *ioPwm)
{
    switch(ioPwm->process.current)
    {
    case Process_Init:  // 初始阶段，执行
        if(ioPwm->CallBack_Init != 0)
        {   ioPwm->CallBack_Init(); }
        PROCESS_CHANGE(ioPwm->process, Process_Idle);
        break;
        
    case Process_Idle:  // 默认状态执行阶段，当默认状态keepInterval不为0，则当超时自动切换到当前状态
        if(IoPwm_RunAsSpecify(ioPwm, &(ioPwm->defaultStatus)) == 1)
        {
            PROCESS_CHANGE(ioPwm->process, Process_Run);
            ioPwm->startTime = realTime;
            ioPwm->__runTime = sysTime;
        }
        break;
        
    case Process_Run:   // 运行阶段，按照currentStatus执行，并返回非0（）时切换为初始化
        if(IoPwm_RunAsSpecify(ioPwm, &(ioPwm->currentStatus)) != 0)
        {   PROCESS_CHANGE(ioPwm->process, Process_Idle);   }
        break;
    }
}
/*********************************************************************************************

  * @brief  IOPwm_ChangeStatus
  * @param  ioPwm：ioPwm实例指针
            status：状态
  * @return 
  * @remark 

  ********************************************************************************************/
void IOPwm_ChangeStatus(IOPwmStruct *ioPwm, IOPwmStatusStruct *status)
{
    memcpy(&(ioPwm->currentStatus), status, sizeof(IOPwmStatusStruct));
    ioPwm->__runTime = sysTime;
    ioPwm->startTime = realTime;
    PROCESS_CHANGE(ioPwm->process, Process_Run);
}
/*********************************************************************************************

  * @brief  IOPwm_ChangeDefault
  * @param  ioPwm：ioPwm实例指针
            status：状态
  * @return 
  * @remark 

  ********************************************************************************************/
void IOPwm_ChangeDefault(IOPwmStruct *ioPwm, IOPwmStatusStruct *status)
{
    memcpy(&(ioPwm->defaultStatus), status, sizeof(IOPwmStatusStruct));
    ioPwm->__runTime = sysTime;
    ioPwm->startTime = realTime;
}
/*********************************************************************************************

  * @brief  IOPwm_ChangeStatus
  * @param  status：状态实例指针
            dutyRatio：占空比
            totalInterval：总时长
            keepInterval：保持时长
  * @return 
  * @remark 

  ********************************************************************************************/
void IOPwm_StatusModify(IOPwmStatusStruct *status, uint8_t dutyRatio, uint16_t totalInterval, uint16_t keepInterval)
{
    status->activeInterval = (totalInterval * dutyRatio) / 100;
    status->totalInterval = totalInterval;
    status->keepInterval = keepInterval;
}
/*********************************************************************************************

  * @brief  IOPwm_IsInDefault
  * @param  ioPwm：ioPwm实例指针
  * @return 
  * @remark 是否处于默认运行下

  ********************************************************************************************/
bool IOPwm_IsInDefault(IOPwmStruct *ioPwm)
{
    return (ioPwm->process.current == Process_Idle);
}
/*********************************************************************************************

  * @brief  IOPwm_IsIdle
  * @param  ioPwm：ioPwm实例指针
  * @return 
  * @remark 是否为空闲

  ********************************************************************************************/
bool IOPwm_IsIdle(IOPwmStruct *ioPwm)
{
    return (ioPwm->process.current == Process_Idle && ioPwm->defaultStatus.keepInterval == 0);
}
/*********************************************************************************************

  * @brief  KeyDetect_PressCheck
  * @param  
  * @return 
  * @remark 按键按下检测

  ********************************************************************************************/
void KeyDetect_PressCheck(KeyDetectStruct *key, bool isInterrupted)
{
    if((isInterrupted == true || key->CallBack_IsKeyPressed(key) == true)
       && key->_isTrigged == false)
	{	
		key->_isTrigged = true;	
		key->__time = sysTime;
	}
}
/*********************************************************************************************

  * @brief  KeyDetect_Handle
  * @param  
  * @return 
  * @remark 按键抬起检测

  ********************************************************************************************/
void KeyDetect_Handle(KeyDetectStruct *key)
{
	// 检测到按键抬起
    if(key->CallBack_IsKeyPressed(key) == false && key->_isTrigged == true)
	{	
		key->_isTrigged = false;
        if(key->CallBack_KeyHandle != NULL)
		{   key->CallBack_KeyHandle(key, sysTime - key->__time); }
	}
}
/*********************************************************************************************

  * @brief  KeyDetect_IsKeyPressed
  * @param  
  * @return 
  * @remark 查看按键是否被按下

  ********************************************************************************************/
bool KeyDetect_IsKeyPressed(KeyDetectStruct *key)
{
	return key->_isTrigged;
}