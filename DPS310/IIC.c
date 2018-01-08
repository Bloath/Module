/*
 * @file       IIC.c
 * @brief      IICͨѶ�ײ�
 * @author     dong
 * @version    v0.1
 * @date       2016-11-03
 */
sbit SCL = P0^5;	
sbit SDA = P0^4; // 5V����

#include "delay.h" 
#include "IIC.h"

void I2Cinit(void)
{
 P0DIR &= ~( 1<<5 );
}


/*******************************************
*  @�������ƣ�start
*  @���ܣ����IIC����ʼ��������
*  @��������
*  @���أ���
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
*  @�������ƣ�stop
*  @���ܣ����IIC����ֹ��������
*  @��������
*  @���أ���
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
*  @�������ƣ�mack
*  @���ܣ� ������Ӧ�����
*  @��������
*  @���أ���
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
*  @�������ƣ�mnack
*  @���ܣ� ������Ӧ�����
*  @��������
*  @���أ���
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
*  @�������ƣ�check
*  @���ܣ��ӻ���Ӧ����
*  @��������
*  @���أ��ӻ��Ƿ���Ӧ��1--�У�0--��
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
*  @�������ƣ�write_1
*  @���ܣ���IIC���߷���һλ1
*  @��������
*  @���أ���
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
*  @�������ƣ�write_0
*  @���ܣ���IIC���߷���һλ0
*  @��������
*  @����ֵ����
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
*  @�������ƣ�write_byte
*  @���ܣ���IIC���߷���һ���ֽڵ�����
*  @������data--���͵�����
*  @���أ���
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
*  @�������ƣ�write_bytes
*  @���ܣ���IIC���߷���N���ֽڵ�����
*  @������databuff-- ��Ҫд�������
          n--��Ҫд������ݵĸ���
*  @���أ�д���Ƿ�ɹ��ı�־��POSITIVE--�ɹ���NEGATIVE--ʧ��
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
*  @�������ƣ�read_byte
*  @���ܣ���IIC���߶�ȡһ���ֽ�
*  @��������
*  @���أ���ȡ������
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
*  @�������ƣ�read_bytes
*  @���ܣ���IIC���߶�ȡN���ֽڵ�����
*  @������inbuffer--��ȡ�����ݴ�ŵ��׵�ַ
          n--���ݵĸ���
*  @���أ���
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

 
