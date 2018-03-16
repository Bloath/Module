# SimpleBuffer

>一种简单的异步通讯处理机制。

#### 一. 机制
将 **需要发送的**以及**接收后还未处理的**数据暂存到缓冲队列中，在主循环中进行循环处理。

* **接收场景**：
	1. 通过串口接收到一条完整的数据（通常用空闲中断来间隔）
	2. 在中断中，将数据复制到缓冲队列中
	3. 在主循环中，循环该缓冲队列，处理其中的接收数据
	4. 处理完成后，释放数据
* **发送场景**
	1. 将需要发送的数据填写到缓冲中
	2. 在主循环中，找到未发送的或者需要重发的数据，进行发送
	3. 在发送完成、重发次数过多或者手动清除标志位后，释放数据

#### 二、用法

##### 2.1 源码结构

```c
├── SimpleBuffer.c  	  	包含缓冲块的填充、释放以及处理
├── SimpleBuffer.h		  	包含缓冲块的结构体定义
├── SimpleBuffer_Conf.h		包含缓冲块的全局定义，包含缓冲块数量，缓冲宽度，缓冲块申请机制
├── stm32_UartDma.c			STM32平台的DMA接收处理
└── stm32_UartDma.h			相关头文件信息
```

##### 2.2 相关定义
```
SimpleBuffer_Conf.h

define => DYNAMIC_MEMORY	在向缓冲块列表中填入数据时，是否动态申请内存，
define => BLOCK_COUNT		缓冲块数量，一般为5，如果异步处理的数据较多，可以增加

SimpleBuffer.h

struct => RxBufferTypeDef	接收缓冲，将串口的单数据暂存在该缓冲中
struct => RxBlockTypeDef	接收缓冲块，将接收到的一条完整的数据填写到队列中
struct => TxBlockTypeDef	发送缓冲块，将需要发送的数据填写到队列中
struct => RxBufferStruct 	是RxBufferTypeDef和RxBlockTypeDef的集合

define => 

```

##### 2.3 使用方法

1. 新建发送缓冲以及接收缓冲

	```c
	RxBlockTypeDef xxRxBlockList[RX_BLOCK_COUNT]
	TxBlockTypeDef xxTxBlockList[TX_BLOCK_COUNT]
	```
	
2. 在接收到数据或者需要发送数据时，填充该队列
	
	```
	
	s
	```
	
3. 在主循环中，加入缓冲列