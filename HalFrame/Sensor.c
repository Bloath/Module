/* Includes ------------------------------------------------------------------*/
#include "Sensor.h"
/* typedef -------------------------------------------------------------------*/
/* define --------------------------------------------------------------------*/
#define SENSOR_CALL(sensorObj, callBack)                                        \
    {                                                                           \
        errorCode = sensorObj->callBack(sensorObj->data);                       \
        if(errorCode != 0)                                                      \
        {   Sensor_ErrorHandle(sensorObj, errorCode);  }                        \
    }
      
/* macro ---------------------------------------------------------------------*/
/* variables -----------------------------------------------------------------*/
/* function prototypes -------------------------------------------------------*/
/* functions -----------------------------------------------------------------*/

/*********************************************************************************************

  * @brief  Sensor_ErrorHandle
  * @param   sensor：传感器框架指针
             errorCode：错误代码
  * @return 
  * @remark 传感器故障处理，errorCode < 0，则直接锁定
            再调用CallBack_ErrorHandle，根据用户需求做处理

  ********************************************************************************************/
void Sensor_ErrorHandle(SensorStruct *sensor, int errorCode)
{
    if(errorCode < 0)
    {   
        PROCESS_CHANGE(sensor->__process, Process_Lock);    
    }
    sensor->CallBack_ErrorHandle(sensor->data, errorCode);
}

/*********************************************************************************************

  * @brief  Sensor_Lock
  * @param   sensor：传感器框架指针
  * @return 
  * @remark 锁定

  ********************************************************************************************/
void Sensor_Lock(SensorStruct *sensor)
{
    PROCESS_CHANGE(sensor->__process, Process_Lock);  
}
/*********************************************************************************************

  * @brief  Sensor_Restore
  * @param   sensor：传感器框架指针
  * @return 
  * @remark 恢复

  ********************************************************************************************/
void Sensor_Restore(SensorStruct *sensor)
{
    PROCESS_CHANGE(sensor->__process, Process_Init);  
}

/*********************************************************************************************

  * @brief  Sensor_Handle
  * @param  sensor：传感器框架指针  
  * @return 
  * @remark 

  ********************************************************************************************/
void Sensor_Handle(SensorStruct *sensor)
{
    int errorCode = 0;
  
    switch(sensor->__process.current)
    {
    /* 初始化，调用初始化流程并进入空闲状态 */
    case Process_Init:
        PROCESS_CHANGE(sensor->__process, Process_Idle);
        if(sensor->CallBack_Init != NULL)
        {   SENSOR_CALL(sensor, CallBack_Init); }
        break;
    
    /* 空闲阶段，有两种触发方式
       1、 设定时间到了
       2、 发现触发信号启动*/
    case Process_Idle:
        if((sensor->interval != 0 && (sensor->__time + sensor->interval) < sysTime)
               || (sensor->CallBack_IsTrigged != NULL && (sensor->CallBack_IsTrigged(sensor->data) == true)))
        {   
            PROCESS_CHANGE(sensor->__process, Process_Idle);
            sensor->__time = sysTime;
        }
        break;
    
    /* 启动阶段 */
    case Process_Start:
        PROCESS_CHANGE(sensor->__process, Process_Wait);
        sensor->__time = sysTime;                           // 记录当前时间
        if(sensor->CallBack_Start != NULL)
        {   SENSOR_CALL(sensor, CallBack_Start);    }       // 启动
        break;
        
    /* 等待阶段，当启动阶段完成后，等待准备就绪标志位置位 */
    case Process_Wait:
        if(sensor->CallBack_IsReady != NULL)
        {   
            if(sensor->CallBack_IsReady(sensor->data) == true);    
            {   PROCESS_CHANGE(sensor->__process, Process_Run);    }
            if((sensor->__time + sensor->timeOut) < sysTime)
            {   Sensor_ErrorHandle(sensor, -1); }
        }
        else
        {   PROCESS_CHANGE(sensor->__process, Process_Run);  }
        break;     
        
    /* 运行阶段，数据处理 */
    case Process_Run:
        PROCESS_CHANGE(sensor->__process, Process_Finish);
        if(sensor->CallBack_Handle != NULL)
        {   SENSOR_CALL(sensor, CallBack_Handle);    }
        break;
        
    /* 完成阶段 */
    case Process_Finish:
        PROCESS_CHANGE(sensor->__process, Process_Idle);
        if(sensor->CallBack_Finish != NULL)
        {   SENSOR_CALL(sensor, CallBack_Finish);    }
        break;
    }
}