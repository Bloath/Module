#ifndef _SENSOR_H_
#define _SENSOR_H_

/* Includes ------------------------------------------------------------------*/
#include "../Module_Conf.h"
/* typedef -------------------------------------------------------------------*/

typedef struct
{
    int (*CallBack_Init)(void *);
    bool (*CallBack_IsTrigged)(void *);
    int (*CallBack_Start)(void *);
    int (*CallBack_IsReady)(void *);
    int (*CallBack_Handle)(void *);
    int (*CallBack_Finish)(void *);
    void (*CallBack_ErrorHandle)(void *, int);
    void *data;                         // 数据指针，贯穿整个传感器回调
    uint32_t interval;                  // 采集间隔
    uint32_t timeOut;                   // 采集超时设置，是指Run 与 finsh之间，等待IsFinish的时间间隔，以ms为单位
    
    ProcessStruct __process;
    uint32_t __time;
}SensorStruct;

/* define --------------------------------------------------------------------*/
/* macro ---------------------------------------------------------------------*/
/* variables -----------------------------------------------------------------*/
/* function prototypes -------------------------------------------------------*/
/* functions -----------------------------------------------------------------*/
void Sensor_Lock(SensorStruct *sensor);
void Sensor_Restore(SensorStruct *sensor);
void Sensor_Handle(SensorStruct *sensor);
#endif
