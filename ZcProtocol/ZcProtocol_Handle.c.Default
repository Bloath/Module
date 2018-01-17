/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"

#include "../Sys_Conf.h"
#include "ZcProtocol.h"
#include "ZcProtocol_API.h"
/* private typedef ------------------------------------------------------------*/
/* private define -------------------------------------------------------------*/
/* private macro --------------------------------------------------------------*/
/* private variables ----------------------------------------------------------*/
/* private function prototypes ------------------------------------------------*/
/*********************************************************************************************

  * @brief  拙诚协议=》操作类指令处理
  * @param  protocol：协议指针
  * @retval 0：处理成功
            1：没发现操作类指令
            2：处理失败
  * @remark 

  ********************************************************************************************/
uint8_t ZcProtocol_OperationCmdHandle(ZcProtocol *protocol)
{
  uint8_t res = 0;              
  
  
  switch(protocol->head.cmd)                          
  { 
    /* 地址域 */
    case ZC_CMD_ADDRESS:
      ZcProtocol_NetTransmit(ZC_CMD_ADDRESS, zcPrtc.head.address, 7, 0);
      break;
    
    /* 设备相关属性 */
    case ZC_CMD_DEVICE_ATTR:
      
      break;
      
    /* 计量相关属性 */
    case ZC_CMD_MEASURE_ATTR:
      
      break;
    
    /* 阶梯费用 */
    case ZC_CMD_LADIR_PRICE:
      
      break;
    
    /* 充值记录 */
    case ZC_CMD_RECHARGE:
      
      break;
    
    /* 用气历史记录 */
    case ZC_CMD_USE_HISTORY:
      
      break;
      
    /* 报警信息 */
    case ZC_CMD_ALARM:
      
      break;
      
    /* 环境参数 */
    case ZC_CMD_ENVIROMENT:
      
      break;
      
    /* 管道状态 */
    case ZC_CMD_PIPE_STATUS:
      
      break;
      
    /* 阀门开关记录 */
    case ZC_CMD_VALVE_RECORD:
      
      break;
      
    /* 阀门开关操作 */
    case ZC_CMD_VALVE_OPRT:
      
      break;
      
    /* 并非操作指令，返回1 */
    default:
      res = 1;
      break;
    }
  
  return res;
}
