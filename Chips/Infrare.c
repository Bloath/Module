/* Includes ----------------------------------------------------------*/
#include "Infrare.h"

/* typedef -----------------------------------------------------------*/
/* define ------------------------------------------------------------*/
/* macro -------------------------------------------------------------*/
/* variables ---------------------------------------------------------*/
/* function prototypes -----------------------------------------------*/


/*********************************************************************************************
 
 * name: 	Infrare_Trig
 * param: 	infrareObj
 * return: 	
 * remark: 	红外线接收脚触发
 
  ********************************************************************************************/
void Infrare_Trig(struct InfrareStruct *infrareObj)
{
    if(infrareObj->isTrigged == false)
    {
        infrareObj->isTrigged = true;
        infrareObj->counter = 0;
        infrareObj->__time = SYSTIME;
    }
    
    infrareObj->trigInteval[infrareObj->counter++] = *(infrareObj->trigTimer);
    *(infrareObj->trigTimer) = 0;
}
/*********************************************************************************************
 
 * name: 	Infrare_Handle
 * param: 	infrareObj
 * return: 	
 * remark: 	红外线处理
 
  ********************************************************************************************/
void Infrare_Handle(struct InfrareStruct *infrareObj)
{
    int i=0;
    uint8_t operation, operationNor;
  
    if(infrareObj->isTrigged == true && (infrareObj->__time + 300) < SYSTIME)
    {
        infrareObj->data = 0;
        if(infrareObj->counter == 36)
        {
            for(i=0; i<32; i++)
            {
                infrareObj->data >>= 1;
                infrareObj->data |= (infrareObj->trigInteval[i + 2] > 1500)? (1 << 31):0;
            }
            operation = (uint8_t)(infrareObj->data >> 16);
            operationNor = (uint8_t)(infrareObj->data >> 24);
            if(operation == (operationNor ^ 0xFF))
            {
               if(infrareObj->CallBack_GetSignal != NULL)
            {   infrareObj->CallBack_GetSignal((uint8_t)(infrareObj->data), operation); } 
            }
        }
        infrareObj->isTrigged = false;
        infrareObj->counter = 0;
    }
}
/*********************************************************************************************
 
 * name: 	Infrare_Decode
 * param: 	operation：操作码
 * return: 	
 * remark: 	代表得字符串
 
  ********************************************************************************************/
char Infrare_Decode(uint8_t operation)
{
    switch(operation)
    {
    case 0x45:
        return '1';
    case 0x46:
        return '2';
    case 0x47:
        return '3';
    case 0x44:
        return '4';
    case 0x40:
        return '5';
    case 0x43:
        return '6';
    case 0x07:
        return '7';
    case 0x15:
        return '8';
    case 0x09:
        return '9';
    case 0x19:
        return '0';
    case 0x16:
        return '*';
    case 0x0D:
        return '#';
    case 0x1C:
        return '\r';
    }
}