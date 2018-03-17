# BufferQueue 使用文档

>在单片机系统的通讯处理中。通讯处理总是会产生各式各样的问题，例如
>
1. 接收：在接收中断中处理数据包，导致实时性的问题
2. 接收：单字节接收，需要找到固定的头尾，来确认这一包发送完成
3. 发送：开启定时器，通过定时器触发不断发送，增加单片机复杂度
4. 发送：无法精准的控制发送、多次发送、停止发送。
5. 发送：发送不同指令的时候切换繁琐
>
以上问题都可以通过这个包解决



## 一、简介

### 1.1 依赖
* `Module/Sys_Conf`中的`sysTime`，ms为单位的自增变量
* `Module/Common/Malloc.h`，自定义的动态申请内存函数

### 1.2 名词解释
* 队列 Queue：包含多条缓冲以及其他参数
* 缓冲 buffer：用于管理数据的结构体变量块
* 数据包 packet：接收到的或者要发送的数组


### 1.3 特性

* **综合**
	* **多队列**：根据不同的通讯方式，无线、串口等，建立多种队列应对不同的通讯
	* **多缓冲**：每个队列中有多个缓冲，默认为5个。数据存入队列时，查找空闲缓冲，申请响应内存并存入
	* **动态申请**：所有放入缓冲内的数据都使用`/Common/Malloc.h`的`Malloc`动态申请内存，最大化利用单片机的RAM
* **发送**
	* **发送间隔**：通过调整该参数可以增大或减少两条发送缓冲的发送间隔
	* **发送模式**：单次自动清除、单次手动清除、多次自动清除、多次手动清除
	* **超时时间**：设置队列中缓冲发送之后超过规定时间自动清除
	* **手动清除**：可以通过id、函数的方式，释放队列中指定的缓冲，停止其发送。
* **接收**
	* **自动释放**：通过指定处理函数，队列中的缓冲以参数的形式带入函数，函数执行完成后释放该接收缓冲，腾出新内存。
	* **单字节接收**：对于低级的单片机，无法通过DMA+空闲中断直接获取完成一帧数据的，提供固定接收缓冲+单字节填充的方式

	
## 二、使用方法

### 2.1 流程

1. <font color="red">**新建**</font>队列 ==> `T/RxQueueStruct xxx_T/RxQueue`
2. <font color="red">**队列处理函数**</font>放到**主循环中** ==> `T/RxQueue_Handle(...)` ==> `while(1){}`
3. <font color="red">**填充**</font>队列缓冲 ===> `T/RxQueue_Add[...](...)`
4. <font color="red">**释放**</font>队列缓冲（接收自动释放） ==> `TxQueue_Free[...](...)`

### 2.2 建立新的队列	

```
typedef struct
{
  uint32_t time;     
  uint16_t usedBlockQuantity;		// 已用缓冲块个数，不可设置
  uint16_t maxTxCount;  			// 最大重发次数，可设置
  TxBaseBlockStruct txBlocks[BLOCK_COUNT];  // 多条缓冲
}TxQueueStruct;             

typedef struct
{
  uint32_t time; 
  uint16_t usedBlockQuantity;
  RxBaseBlockStruct rxBlocks[BLOCK_COUNT];
}RxQueueStruct;            

// 新建队列：直接通过结构新建结构体变量即可，建议用通讯媒介取名
// TxQueueStruct rs485_TxQueue
// RxQueueStruct rs485_RxQueue
```

### 2.3 队列处理函数

```
void RxQueue_Handle(RxQueueStruct *rxQueue, void (*rxPacketHandle)(uint8_t*, uint16_t)); 
void TxQueue_Handle(TxQueueStruct *txQueue, void (*Transmit)(uint8_t*, uint16_t), uint32_t interval)

* 参数1：队列指针
* 参数2：处理函数指针
* 参数3：间隔（发送独有），处理队列中非闲置缓冲块的间隔，用于调整发送频率（单位为ms）
```
##### 2.3.1 队列处理函数内部流程

1. 循环队列内所有的缓冲块，找到非闲置缓冲块。
2. 处理阶段
	* 接收：直接调用 参数2 的函数指针指向的函数，并将缓冲块的数据包带入
	* 发送：根据发送规则，决定是否调用 参数2 所指函数
3. 处理完成阶段
	* 接收：直接释放对应缓冲块
	* 发送：根据发送规则，决定是否释放

##### 2.3.1 队列处理函数的函数指针
>队列处理函数的第二个形参为函数指针.<br>用户自定义一个数据包处理函数，将此函数作为参数带入队列处理函数。<br>不同队列处理时调用不同的数据包函数进行处理，互不干扰

接收与发送的数据包处理略有不同

* 接收：指的是已经**接收到的数据包的处理函数**，更多的写逻辑业务，比如收到指令执行不同操作等内容
* 发送：指的是**硬件发送函数**，例如485的发送，其中应该写的是底层调用UART，启动硬件发送数据包。

#### 2.4 填充队列
```
// 填充接收队列，将接收到的完整数据包填充到队列中
uint16_t RxQueue_Add(RxQueueStruct *rxQueue, uint8_t *packet, uint16_t Len);   

// 填充发送队列，将需要发送的完整数据包填充队列中，并写入发送规则
uint16_t TxQueue_Add(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t custom); 
uint16_t TxQueue_AddWithId(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t custom, TX_ID_SIZE id);

```

1. **接收队列**的填充比较简单，就是将数据包填充到队列的闲置缓冲块中，并置位标准位
2. **发送队列**填充就相对复杂一点
	* **规则设置**：不同的缓冲块数据包有着不同的发送方式
		* 单次自动释放：只发一次，自动释放，一般用于丛机回复
		* 多次自动释放：发多次，次数上限通过队列的maxTxCount参数设置，一般用于低功耗设备的请求
		* 单次手动释放：只发一次，等待用户通过函数释放。
		* 多次手动释放：不停的发送，等待用户调用函数释放。用于主机发送请求
	* **添加标识符**：可以通过`TxQueue_AddWithId`函数，在填充队列时，给数据包打上标识。完成条件后可以通过`TxQueue_FreeById`快速释放

#### 2.5 释放队列

|释放类型|队列类型|释放过程|
|:---:|:---:|:---:|
|自动释放|接收/发送(单/多次自动释放)|在队列处理函数中，调用数据包处理函数完成后释放|
|通过id释放|发送|`TxQueue_FreeById`，在队列中查找id与函数参数id相同的数据包释放|
|通过自定义函数释放|发送|`TxQueue_FreeByFunc`，通过自定义函数以及参数，查找符合条件的缓冲块（自定义函数返回TRUE），进行释放|





