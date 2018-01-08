/*
 * @file       IIC.c
 * @brief      IIC通讯底层
 * @author     dong
 * @version    v0.1
 * @date       2016-11-03
 */
sbit SCL = P0^5;	
sbit SDA = P0^4; // 5V开关

#include "delay.h" 
#include "IIC.h"

void I2Cinit(void)
{
 P0DIR &= ~( 1<<5 );
}


/*******************************************
*  @函数名称：start
*  @功能：完成IIC的起始条件操作
*  @参数：无
*  @返回：无
********************************************/
void start(void)
{
  SCL_H;
  SDA_H;
  delay();
  SDA_L;
  delay();
  SCL_L;
  delay();
}
/*******************************************
*  @函数名称：stop
*  @功能：完成IIC的终止条件操作
*  @参数：无
*  @返回：无
********************************************/
void stop(void)
{
  SDA_L;
  delay();
  SCL_H;
  delay();
  SDA_H;
  delay();	   
}
/*******************************************
*  @函数名称：mack
*  @功能： 主机有应答操作
*  @参数：无
*  @返回：无
********************************************/
void mack(void)
{
  SDA_L;
  _nop; _nop;   
  SCL_H;
  delay();
  SCL_L;
  _nop;_nop;   	
  SDA_H;     
  delay();
}
/*******************************************
*  @函数名称：mnack
*  @功能： 主机无应答操作
*  @参数：无
*  @返回：无
********************************************/
void mnack(void)
{
  SDA_H;
  _nop; _nop; 	
  SCL_H;
  delay(); 
  SCL_L;
  _nop; _nop;   
  SDA_L;   
  delay();       
}

/*******************************************
*  @函数名称：check
*  @功能：从机的应答检查
*  @参数：无
*  @返回：从机是否有应答：1--有，0--无
********************************************/
unsigned char  check(void)
{
  unsigned char  s_ack;

  SDA_H;
  _nop; _nop;    	
  SCL_H;
  _nop; _nop;  
  s_ack = SDA_val;   
  SCL_L;
  delay();
  SDA_out;
  if(s_ack)       return NEGATIVE;
  else            return POSITIVE;
}
/*******************************************
*  @函数名称：write_1
*  @功能：向IIC总线发送一位1
*  @参数：无
*  @返回：无
********************************************/
void write_1(void)
{
  SDA_H;
  delay();
  SCL_H;
  delay();
  SCL_L;				
  delay();
}
/*******************************************
*  @函数名称：write_0
*  @功能：向IIC总线发送一位0
*  @参数：无
*  @返回值：无
********************************************/
void write_0(void)
{
  SDA_L;
  delay();
  SCL_H;
  delay();			 
  SCL_L;				
  delay();
}
/*******************************************
*  @函数名称：write_byte
*  @功能：向IIC总线发送一个字节的数据
*  @参数：data--发送的数据
*  @返回：无
********************************************/
void write_byte(unsigned char  data)
{
  unsigned char  i;

  for(i = 8;i > 0;i--)
  {
    if(data & 0x80)	write_1();
    else                write_0();
    data <<= 1;
  } 
  stop();
}
/*******************************************
*  @函数名称：write_bytes
*  @功能：向IIC总线发送N个字节的数据
*  @参数：databuff-- 需要写入的数据
          n--需要写入的数据的个数
*  @返回：写入是否成功的标志：POSITIVE--成功，NEGATIVE--失败
********************************************/
unsigned char  write_bytes(unsigned char  * databuff,unsigned char  n)
{
  unsigned char  i;

  for(i = 0;i < n;i++)
  {
    write_byte(* databuff);
    if(check())      
    {
            databuff++;			
    }
    else 
    {
            stop();
            return NEGATIVE;
    }
  }

  stop();
  return POSITIVE;			
}


/*******************************************
*  @函数名称：read_byte
*  @功能：从IIC总线读取一个字节
*  @参数：无
*  @返回：读取的数据
********************************************/
unsigned char  read_byte(void)
{
  unsigned char  data = 0x00;
  unsigned char  i;
  unsigned char  flag;

  for(i = 0;i < 8;i++)
  {
    SDA_H;
    delay();
    SCL_H;
    delay();
    flag = SDA_val;
    data <<= 1;
    if(flag)	data |= 0x01;
    SDA_out;
    SCL_L;
    delay();
  }

  return data;
}
/*******************************************
*  @函数名称：read_bytes
*  @功能：从IIC总线读取N个字节的数据
*  @参数：inbuffer--读取后数据存放的首地址
          n--数据的个数
*  @返回：无
********************************************/
void read_bytes(unsigned char  * databuff,unsigned char  n)
{
  unsigned char  i;

  for(i = 0;i < n;i++)
  {
    databuff[i] = read_byte();
    if(i < (n-1))	mack();
    else                mnack();
  }
   
  stop();
}

 
