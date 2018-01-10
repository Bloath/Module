#ifndef _CONF_H_
#define _CONF_H_

#include "stm32f0xx_hal.h"

/**************************系统时间 ms***************************/
#define sysTime HAL_GetTick()

/*********************Simple Buffer 全局配置*********************/
#define DYNAMIC_MEMORY  1       //是否使用 动态申请内存， 使用后会导致代码量增加4K左右
#define STATIC_BUFFER_LEN 32    //不使用动态内存的情况下，静态内存的大小

#define BUFFER_LENGTH  400      //缓冲大小，填写报文最大长度的两杯，否则会导致溢出

#define BLOCK_COUNT 5           //发送缓冲、接收缓冲块队列的长度 

#define TX_TIME_OUT     3000    //超时时间，单位为ms

/*********************ESP8266 全局配置**********************/
#define ESP8266_INTERVAL 3000

/*********************协议相关 全局配置*********************/
#define DOMAIN "wxio.bloath.com"
#define HTTP_MAX_LEN 200                //HTTP协议最大长度
  
#endif