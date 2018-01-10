#ifndef _CONF_H_
#define _CONF_H_

#include "stm32f0xx_hal.h"

/**************************ϵͳʱ�� ms***************************/
#define sysTime HAL_GetTick()

/*********************Simple Buffer ȫ������*********************/
#define DYNAMIC_MEMORY  1       //�Ƿ�ʹ�� ��̬�����ڴ棬 ʹ�ú�ᵼ�´���������4K����
#define STATIC_BUFFER_LEN 32    //��ʹ�ö�̬�ڴ������£���̬�ڴ�Ĵ�С

#define BUFFER_LENGTH  400      //�����С����д������󳤶ȵ�����������ᵼ�����

#define BLOCK_COUNT 5           //���ͻ��塢���ջ������еĳ��� 

#define TX_TIME_OUT     3000    //��ʱʱ�䣬��λΪms

/*********************ESP8266 ȫ������**********************/
#define ESP8266_INTERVAL 3000

/*********************Э����� ȫ������*********************/
#define DOMAIN "wxio.bloath.com"
#define HTTP_MAX_LEN 200                //HTTPЭ����󳤶�
  
#endif