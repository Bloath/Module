/* Includes ------------------------------------------------------------------*/
#include "Communicate_API.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/

/*********************************************************************************************

  * @brief  发送块错误处理
  * @param  communicate：半双工实例指针
  * @retval 无
  * @remark 

  ********************************************************************************************/
void Comunicate_HDMaster_Polling(CommunicateStruct *communicate)
{
    /* 根据主状态进行处理 */
    switch (communicate->process)
    {
    /* 初始化 */
    case Process_Init:
        communicate->process = Process_Idle;
        communicate->time = *communicate->refTime;
        break;

    /* 轮询间隔时间 */
    case Process_Idle:
        /* 间隔一段时间 */
        if ((communicate->time + communicate->loopInterval) > *communicate->refTime)
        {   break;  }
        else
        {   communicate->process = Process_Start;   }

    /* 空闲状态，填充查询暂存报文，切换为等待状态 */
    case Process_Start:
        if (communicate->CallBack_FillHoldMsg != NULL)
        {   communicate->CallBack_FillHoldMsg(communicate); }       // 空闲数据包填充
        communicate->process = Process_Wait;                        // 切换等待状态
        break;

    /* 等待状态，发送缓冲为手动清除，如果没有回复的话是不会清除的，否则会出现阻塞 
     等待状态为死锁，直到接收到确认报文或者查询暂存回复*/
    case Process_Wait:
        communicate->time = *communicate->refTime;
        if (communicate->txQueue->usedBlockQuantity == 0)
        {   communicate->process = Process_Init;    }
        break;
    }
}

/*********************************************************************************************

  * @brief  半双工通讯 主处理
  * @param  communicate：半双工处理
  * @retval 无
  * @remark 

  ********************************************************************************************/
void Communicate_Handle(CommunicateStruct *communicate)
{
    /* 半双工 + 客户端模式，发送查询暂存报文 */
    if ((communicate->attribute & (COM_ATTR_HD_CLINET | COM_ATTR_POLL_MNL)) == COM_ATTR_HD_CLINET )
    {   Comunicate_HDMaster_Polling(communicate);   }

    /* 发送队列处理 */
    if (communicate->CallBack_TxFunc != NULL)
    {   TxQueue_Handle(communicate->txQueue, communicate->CallBack_TxFunc, communicate->txFuncParam);   }
    
    if (communicate->CallBack_RxHandleFunc != NULL)
    {   RxQueue_Handle(communicate->rxQueue, communicate->CallBack_RxHandleFunc, (void *)communicate);  }
}

