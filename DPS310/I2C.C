/******************************************************************* 

一、程序说明： 

1, 24LC02器件地址是1010000R/W. 

2, 数组写入24LC02采取页写方式. 

3, 数组code从24LC02读出时采取自由读方式. 

4, 采用4.00M晶体。 

5，采用软件I2C。 

 

二、硬件连接： 

1， SDA------->23 pin.(当然你可以任意选择脚位） 

2, SCL------->18 Pin.(当然你可以任意选择脚位） 

3， PORTD----->外接8个LED，显示读出的数据，在这里，读出的刚好是一个闪动的流水灯状态。 

 

*******************************************************************/ 

＃i nclude "pic.h" 

 

#define uchar unsigned char 

#define nop() asm("nop"

#define SCL TRISC3 

#define SDA TRISC4 

 

void start_i2c(); 

void stop_i2c(); 

void send_byte(uchar c); 

uchar receive_byte(); 

void I_send_str(uchar sla,uchar suba,uchar *s,uchar no); 

void delay_250ms(); 

void i2c_error (); 

 

uchar code[]={0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff}; 

uchar no,ack,c,data; 

 

 

void main(void) 

{ 

uchar i; 

TRISC=0Xff; //C口设为输入 RC3为SCL线，RC4为SDA线。 

PORTC=0X00; 

TRISD=0X00; //D口为输出，显示IC24LC02中读出的内容 

PORTD=0X00; //初始显示全亮 

 

I_send_str(0xa0,0x00,code,9); //页写入code数组到24LC02,器件地址为0Xa0,子地址为0X00，共9个数。 

 

delay_250ms(); 

 

 

///////////开始读出到D口进行显示,根据Random read时序图。 

while (1) 

{ 

for (i=0x00;i<0x09;i++) 

{ 

start_i2c(); 

send_byte(0xa0); //发送器件地址，即DEVICE ADDRESS。 

if (ack==0) i2c_error(); //如果24LC02无应答。则进入I2C ERROR错误指示。 

send_byte(i); //发送字地址，即WORD ADDRESS。D口显示数组。 

if (ack==0) i2c_error(); 

start_i2c(); //重新启动总线。 

send_byte(0xa1); //发送读命令和器件地址DEVICE ADDRESS。 

if (ack==0) i2c_error(); 

data=receive_byte(); 

stop_i2c(); 

PORTD=data; 

delay_250ms(); 

} 

} 

} 

 

 

/******************************************************************* 

起动总线函数 

函数原型： void start_i2c(); 

Function: start on the I2C bus 

*******************************************************************/ 

void start_i2c() 

{ 

SDA=1; //发送启始条件的数据信号 

nop(); 

SCL=1; 

nop();nop();nop();nop();nop(); //24LC02要求建立时间大于4，7S 

SDA=0; //发送起始信号 

nop();nop();nop();nop();nop(); 

SCL=0; //钳住I2C总线，准备发送数据或接收数据 

nop();nop(); 

} 

 

 

/******************************************************************* 

停止总线函数 

函数原型： void stop_i2c(); 

Function: stop the I2C bus 

*******************************************************************/ 

void stop_i2c() 

{ 

 

SDA=0; //发送结束条件的数据信号 

nop(); 

SCL=1; 

nop();nop();nop();nop();nop(); 

SDA=1; 

nop();nop();nop();nop(); 

} 

 

/*================================================================= 

字节数据传送函数 

函数原型： void send_byte(uchar c); 

Function: 将数据C发送出去，可以是地址，也可以是数据，发完后等待回应，并对此状态 

位进行操作（不应答或非应答都使ack=0 ），发送数据正常，ack=1;ack=0 

表示被控器无应答或损坏。 

==================================================================*/ 

void send_byte(uchar c) 

{ 

uchar bit_count; 

for (bit_count=0;bit_count<8;bit_count++) 

{ 

if ((c<<bit_count)&0x80) {SDA=1;} 

else {SDA=0;} 

nop(); 

SCL=1; 

nop();nop();nop();nop();nop(); 

SCL=0; 

} 

nop();nop(); 

SDA=1; 

nop();nop(); 

SCL=1; 

nop();nop();nop(); 

if (RC4==1) ack=0; 

else ack=1; //用ASK=1为有应答信号 

SCL=0; 

nop();nop(); 

} 

 

/*================================================================== 

字节数据接收函数 

函数原型：uchar receive_byte(); 

FUNCTION: 用来接收从器件传来的数据，并判断总线错误（不发应答信号）， 

发完后请用应答函数。 

===================================================================*/ 

uchar receive_byte() 

{ 

uchar retc,bit_count; 

retc=0; 

SDA=1; 

for (bit_count=0;bit_count<8;bit_count++) 

{ 

nop(); 

SCL=0; 

nop();nop();nop();nop();nop(); 

SCL=1; 

nop();nop(); 

retc=retc<<1; 

if (RC4==1) retc=retc+1; 

nop();nop(); 

} 

SCL=0; 

nop();nop(); 

return (retc); 

} 

 

 

/*================================================================ 

向有子地址器件发送多字节数据函数 

函数原型： bit I_send_str(uchar sla,uchar suba,uchar *s,uchar no); 

Function: 从启动总线到发送地址，数据，结束总线的全过程，从器件地址sla。如果 

返回1表示操作成功，否则操作有误。 

=================================================================*/ 

void I_send_str(uchar sla,uchar suba,uchar *s,uchar no) 

{ 

uchar i; 

start_i2c(); 

send_byte(sla); 

if (ack==0) i2c_error(); 

send_byte(suba); 

if (ack==0) i2c_error(); 

for (i=0;i<no;i++) 

{ 

send_byte(*s); 

if (ack==0) i2c_error(); 

s++; 

} 

stop_i2c(); 

// return(1); 

} 

 

/***************************************************************** 

延时函数 

函数原型： void delay_250ms(); 

FUNCTION: 延明250ms 

*****************************************************************/ 

void delay_250ms() 

{ 

unsigned int d=24999; 

while (--d); 

} 

 

/***************************************************************** 

总线错误函数 

函数原型：　void i2c_error(); 

Function: 通过RD7闪动8次表示总线操作失败一次报警。 

*****************************************************************/ 

void i2c_error () 

{ 

uchar i; 

for (i=0;i<8;i++) 

{ 

RD7=0; 

delay_250ms(); 

RD7=1; 

delay_250ms(); 

} 

 

} 