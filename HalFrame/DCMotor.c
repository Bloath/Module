/* Includes ------------------------------------------------------------------*/
#include "DCMotor.h"
/* typedef -------------------------------------------------------------------*/
/* define --------------------------------------------------------------------*/
/* macro ---------------------------------------------------------------------*/
/* variables -----------------------------------------------------------------*/
/* function prototypes -------------------------------------------------------*/
/* functions -----------------------------------------------------------------*/
/*********************************************************************************************

  * @brief  DCMotor_SetNextOperation
  * @param  motor：电机指针
            operation：操作
            isForced：是否为强制
  * @return 
  * @remark 设置下次操作

  ********************************************************************************************/
int DCMotor_SetNextOperation(struct DCMotorStruct *motor, enum DCMotorOperation operation, bool isForced)
{
    if(isForced == false && motor->isBanned == true)
    {   return -1;  }
  
    enum DCMotorOperation *operationPointer = (isForced == true)? &(motor->currentOperation):&(motor->nextOperation);

    *operationPointer = operation;
    if(isForced == true)
    {   PROCESS_CHANGE(motor->process, Process_Start);  }
      
    return 0;
}
/*********************************************************************************************

  * @brief  DCMotor_Handle
  * @param  
  * @return 
  * @remark 设置下次操作

  ********************************************************************************************/
void DCMotor_Handle(struct DCMotorStruct *motor)
{
    switch(motor->process.current)
    {
    /* 电机初始化 */
    case Process_Init:
        if(motor->CallBack_Init != NULL)
        {   motor->CallBack_Init();  }
        PROCESS_CHANGE(motor->process, Process_Idle);
        break;
        
    /* 电机空闲状态，等待操作命令 */
    case Process_Idle:
        if(motor->nextOperation != DCMotor_Idle)
        {
            if(motor->isBanned == true
               || motor->nextOperation == motor->currentOperation)
            {   
                motor->nextOperation = DCMotor_Idle; 
                break;
            }
            motor->currentOperation = motor->nextOperation;      
            motor->nextOperation = DCMotor_Idle;
            PROCESS_CHANGE(motor->process, Process_Start);
        }
        break;
        
    /* 电机启动操作 */
    case Process_Start:
        if(motor->currentOperation == DCMotor_Idle)
        {   
            PROCESS_CHANGE(motor->process, Process_Idle);   
            break;
        }
        
        if(motor->CallBack_BeforeRun != NULL)
        {   motor->CallBack_BeforeRun(motor);   }                   // 启动前回调
        
        motor->CallBack_Operation(motor->currentOperation);         // 根据操作进行HAL级别调用
        PROCESS_CHANGE(motor->process, Process_Run);
        break;
        
    /* 电机运行期间 */
    case Process_Run:
        if(motor->CallBack_AfterRun != NULL)
        {
            if(motor->CallBack_AfterRun(motor) == true)
            {   PROCESS_CHANGE(motor->process, Process_Wait);   }
        }
        else
        {   PROCESS_CHANGE(motor->process, Process_Wait);   }  
        break;
        
    /* 等待阶段，等待结束条件成立 */
    case Process_Wait:
        if(motor->CallBack_StopCondition != NULL)
        {
            if(motor->CallBack_StopCondition(motor) == true)
            {   PROCESS_CHANGE(motor->process, Process_BeforeFinish);   }
        }
        else
        {   PROCESS_CHANGE(motor->process, Process_BeforeFinish);   }  
        break;
      
    /* 等待阶段，等待结束条件成立 */
    case Process_BeforeFinish:
        motor->CallBack_Operation(DCMotor_Idle);                    // 使电机进入空闲状态
        PROCESS_CHANGE(motor->process, Process_Finish);
        break;
        
    /* 完成阶段 */
    case Process_Finish:
        motor->_lastOperation = motor->currentOperation;
        motor->currentOperation = DCMotor_Idle;
        if(motor->CallBack_Finish != NULL)
        {   motor->CallBack_Finish(motor);  }
        PROCESS_CHANGE(motor->process, Process_Idle);
        break;
    }
      
}
/*********************************************************************************************

  * @brief  DCMotor_Status
  * @param  motor：电机指针
            operation：操作
            isForced：是否为强制
  * @return 
  * @remark 确认DCMotor是否处在某个状态，非互斥

  ********************************************************************************************/
bool DCMotor_Status(struct DCMotorStruct *motor, enum DCMotorOperation operation)
{
    if(motor->process.current == Process_Idle
       && motor->_lastOperation == operation)
    {   return true;    }
    
    return false;
}