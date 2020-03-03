/* Includes ------------------------------------------------------------------*/
#include "Priority.h"
/* typedef -------------------------------------------------------------------*/
/* define --------------------------------------------------------------------*/
/* macro ---------------------------------------------------------------------*/
/* variables -----------------------------------------------------------------*/
/* function prototypes -------------------------------------------------------*/
/* functions -----------------------------------------------------------------*/
/*********************************************************************************************

  * @brief  Priority_AddTask
  * @param  priorityObj：优先级任务指针
            taskObj：任务指针
            level：任务等级
  * @return 
  * @remark 

  ********************************************************************************************/
void Priority_AddTask(struct PriorityStruct *priorityObj, struct TaskUnitStruct *taskObj, uint8_t level)
{
    for(int i=0; i<PRIORITY_TASK_COUNT; i++)
    {
        if(priorityObj->__taskList[i] == NULL)
        {
            priorityObj->_usedTaskCount++;
            priorityObj->__taskList[i] = taskObj;
            taskObj->level = level;
            break;
        }
    }
}
/*********************************************************************************************

  * @brief  Priority_LoopTaskList
  * @param  priorityObj：优先级任务指针
            CallBack_Handle：处理回调函数
            param：参数
  * @return 
  * @remark 轮询，在优先级中，很多都需要轮询，

  ********************************************************************************************/
void Priority_LoopTaskList(struct PriorityStruct *priorityObj, void (*CallBack_Handle)(struct TaskUnitStruct*, void*), void *param)
{
    for(int i=0; i<PRIORITY_TASK_COUNT; i++)
    {
        if(priorityObj->__taskList[i] != NULL)
        {   CallBack_Handle(priorityObj->__taskList[i], param); }
    }
}
/*********************************************************************************************

  * @brief  Priority_LevelHandle
  * @param  task：任务指针
            param：参数
  * @return 
  * @remark 当

  ********************************************************************************************/
void Priority_LevelHandle(struct TaskUnitStruct* task, void* param)
{
    int level = *(int *)param;

    /* 在切换时查看是否有需要 */
    if(task->CallBack_IsNeedHandle() == false)
    {   
        if(FLAG_IS_SET(task->flag, TASK_FLAG_HANDLING) == true && task->CallBack_Finish != NULL)
        {   task->CallBack_Finish();    }               // 调用完成回调
        task->flag = 0;                                 // 清空标志位
        
        return;
    }
   
    if(task->level != level)
    {
        /* 等级切换时，如果低等级任务并未暂停，则调用pause回调进入暂停 */
        if(FLAG_IS_SET(task->flag, TASK_FLAG_PAUSING) == false)
        {
            if(task->CallBack_Pause != NULL)
            {   task->CallBack_Pause(); }
            FLAG_SET(task->flag, TASK_FLAG_PAUSING);
        }
    }
    else
    {
        /*  如果和当前最高等级相同，则按照标志位
            1. init
            2. unpause
            3. handle，在handle出现返回值为true时，表示完成，调用finish  */
        // 如果没有初始化，则初始化
        if(FLAG_IS_SET(task->flag, TASK_FLAG_INIT) == false)
        {
            if(task->CallBack_Init != NULL)
            {   task->CallBack_Init(); }
            FLAG_SET(task->flag, TASK_FLAG_INIT)
        }
      
        // 如果处在暂定，则解除暂停
        if(FLAG_IS_SET(task->flag, TASK_FLAG_PAUSING))
        {
            if(task->CallBack_Unpause != NULL)
            {   task->CallBack_Unpause(); }
            FLAG_CLR(task->flag, TASK_FLAG_PAUSING)
        }
        
        task->CallBack_Handle();                            // 处理函数
        if(FLAG_IS_SET(task->flag, TASK_FLAG_HANDLING) == false)
        {   FLAG_SET(task->flag, TASK_FLAG_HANDLING);   }   // 置位处理标志位
    }
    
}
/*********************************************************************************************

  * @brief  Priority_FindHighestTask
  * @param  priorityObj：优先级任务指针
  * @return -1：没有任何需要处理的事务
            其他：当前有需要处理的事务任务中最高优先级的等级
  * @remark 找到当前需要处理的非空闲的最高level

  ********************************************************************************************/
int Priority_FindHighestTask(struct PriorityStruct *priorityObj)
{
    int level = -1;
  
    for(int i=0; i<PRIORITY_TASK_COUNT; i++)
    {
        if(priorityObj->__taskList[i] != NULL)
        {
            if(priorityObj->__taskList[i]->CallBack_IsNeedHandle() == true)
            {
                if(priorityObj->__taskList[i]->level > level)
                {   level = priorityObj->__taskList[i]->level;  }
            }
        }
    }
    
    return level;
}

/*********************************************************************************************

  * @brief  
  * @param   
  * @return 
  * @remark 

  ********************************************************************************************/
void Priority_Handle(struct PriorityStruct *priorityObj)
{
    int level = -1;

    level = Priority_FindHighestTask(priorityObj);              // 先轮询找到当前需要处理的最高优先级
    Priority_LoopTaskList(priorityObj, Priority_LevelHandle, &level); 
    if(level != priorityObj->currentLevel)                       
    {   priorityObj->currentLevel = level;  }
}