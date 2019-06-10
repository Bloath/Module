/* Includes ------------------------------------------------------------------*/
#include "IOPwm.h"

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
  * @remark 

  ********************************************************************************************/
int IoPwm_RunAsSpecify(IOPwmStruct *ioPwm, IOPwmStatusStruct *status)
{
    /* 根据激活状态
       激活：则进行开关操作
       非激活：则直接进入非使能状态*/
    uint32_t toggleInterval = (status->totalInterval * status->dutyRatio) / 100;
    toggleInterval = (ioPwm->CallBack_IsIOActive() == false)? (status->totalInterval - toggleInterval): toggleInterval;
                                                               
    if(status->isActive == true)
    {
        /* 占空比非100，则直接使能
           占空比不为0则按照占空比实际处理*/
        if(status->dutyRatio == 100)
        {   
            if(ioPwm->CallBack_IsIOActive() == false)
            {   ioPwm->CallBack_IOOperation(true);  }
        }
        else if((ioPwm->__runTime + toggleInterval) < sysTime)
        {
            ioPwm->__runTime = sysTime;     
            ioPwm->CallBack_IOOperation((ioPwm->CallBack_IsIOActive() == true)? false:true);       // 根据IO状态反转切换
        }
    } 
    else
    {   ioPwm->CallBack_IOOperation(false); }
    
    /* 持续时间为0则报-2，一般默认的才会为0 */
    if(status->keepInterval == 0)
    {   return -2;  }
    
    /* 超时则报-1 */
    if((ioPwm->startTime + status->keepInterval) < realTime)
    {   return -1;  }
    
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
        if(IoPwm_RunAsSpecify(ioPwm, &(ioPwm->defaultStatus)) == -1)
        {
            PROCESS_CHANGE(ioPwm->process, Process_Run);
            ioPwm->startTime = realTime;
            ioPwm->__runTime = sysTime;
        }
        break;
        
    case Process_Run:   // 运行阶段，按照currentStatus执行，并返回非0（到时了）时切换为初始化
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
            isActive：是否为激活
            dutyRatio：占空比
            totalInterval：总时长
            keepInterval：保持时长
  * @return 
  * @remark 

  ********************************************************************************************/
void IOPwm_StatusModify(IOPwmStatusStruct *status, bool isActive, uint8_t dutyRatio, uint16_t totalInterval, uint16_t keepInterval)
{
    status->isActive = isActive;
    status->dutyRatio = dutyRatio;
    status->totalInterval = totalInterval;
    status->keepInterval = keepInterval;
}