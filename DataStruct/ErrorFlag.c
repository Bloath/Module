/* Includes ------------------------------------------------------------------*/
#include "ErrorFlag.h"
#include "Module/Common/Common.h"
/* typedef -------------------------------------------------------------------*/
/* define --------------------------------------------------------------------*/
/* macro ---------------------------------------------------------------------*/
/* variables -----------------------------------------------------------------*/
/* function prototypes -------------------------------------------------------*/
/* functions -----------------------------------------------------------------*/
/*********************************************************************************************

  * @brief  Error_Handle
  * @param  error：the pointer of struct error obj
  * @return 
  * @remark 

  ********************************************************************************************/
void ErrorFlag_Handle(struct ErrorFlagStruct *error, void *param)
{
    uint32_t temp32u = 0, i, flagTrigged = 0, flagClear = 0;
  
    if(error->flag != error->__flagCache)
    {
        temp32u = error->flag ^ error->__flagCache;
        flagTrigged = temp32u & error->flag;
        flagClear = temp32u & (~error->flag);
        
        for(i=0; i<32; i++)
        {
            if(FLAG_IS_SET(flagTrigged, (1<<i)))
            {
                if(error->CallBack_ErrorTrigged != NULL)
                {   error->CallBack_ErrorTrigged(error, i, param); }
            }
        }
        
        for(i=0; i<32; i++)
        {
            if(FLAG_IS_SET(flagClear, (1<<i)))
            {
                if(error->CallBack_ErrorClear != NULL)
                {   error->CallBack_ErrorClear(error, i, param); }
            }
        }
        
        /* 更新标志位 */
        error->__flagCache = error->flag;
    }
}
/*********************************************************************************************

  * @brief  ErrorFlag_TrigWithParameter
  * @param  error：the pointer of struct error obj
            index: the index of error
            parameter：the error param
  * @return 
  * @remark trig and run Error_Handle immediately

  ********************************************************************************************/
void ErrorFlag_TrigWithParameter(struct ErrorFlagStruct *error, uint16_t index, void *param)
{
    ERRORFLAG_TRIG(error, index);
    ErrorFlag_Handle(error, param);   
}
/*********************************************************************************************

  * @brief  ErrorFlag_ClearWithParameter
  * @param  error：the pointer of struct error obj
            index: the index of error
            parameter：the error param
  * @return 
  * @remark 

  ********************************************************************************************/
void ErrorFlag_ClearWithParameter(struct ErrorFlagStruct *error, uint16_t index, void *param)
{
    ERRORFLAG_CLEAR(error, index);
    ErrorFlag_Handle(error, param);     
}