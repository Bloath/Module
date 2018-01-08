/*
 * @file       IIC.h
 * @brief      IIC通讯底层头文件
 * @author     dong
 * @version    v0.1
 * @date       2016-11-03
 */

#define SCL_H  SCL=1; 
#define SCL_L  SCL=0; 

#define  SDA_out   SDA=0; //Set_GPIOX_Value(SDA, LOW)   
#define  SDA_L    { P0DIR &= ~( 1<<4 );SDA=0;} //GPIO_Init(SDA, OUT, LOW) 
#define  SDA_H    { P0DIR |=  ( 1<<4 );SDA=1;} //GPIO_Init(SDA, IN, HIGH) 
#define  SDA_val   (SDA) //GPIOX_Value(SDA)

void I2Cinit(void);

void start(void);

void stop(void);

void mack(void);

void mnack(void);

unsigned char  check(void);

void write_1(void);

void write_0(void);

void write_byte(unsigned char  data);

unsigned char  write_bytes(unsigned char  * databuff,unsigned char  n);

unsigned char  read_byte(void);

void read_bytes(unsigned char  * databuff,unsigned char  n);


