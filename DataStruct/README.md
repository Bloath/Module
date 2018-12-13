[TOC]

> 这些“ 数据结构 ”是单片机可用的小型管理块，例如用于处理通讯的BufferQueue、用于数据存储管理的LoopDisorder，都是在我实际项目提炼出来的小玩意，大大减轻单片机管理通讯、存储之类的负

<br>

## 一、缓冲队列 BufferQueue

> 对于通讯来说，无非都是发送和接收。对于一个对速度有要求的场合，发送和接收的处理都有可能被其他事务打断，则必须借助缓冲队列，将需要发送或接收的需要解析的数据先保存起来，待主机处理完其他事务后进行处理。



### 1.1 快速上手

```C
#include "Module.h"

TxQueueStruct txQueue;				// 新建发送缓冲队列
RxQueueStruct rxQueue;				// 新建接收缓冲队列

// 创建 发送函数 以及 接收报文处理/解析函数
bool HAL_Transmit(uint8_t *message, uint16_t length, void *param);
{	return false;	}
void RxMsg_Analyse(uint8_t *message, uint16_t length, void *param)
{}

void main()
{
    uint8_t message[10] = {1,2,3,4,5,6,7,8,9,0};
    
    /* 在任意地方填充 */
    TxQueue(&txQueue, message, 10, TX_ONCE_AC);			// 将数组填充到发送缓冲队列中，并标记为 发送一次并自动清除
    RxQueue(&rxQueue, message, 10, false);				// 将数组填充到接收队列中
    
    while(1)
    {
        TxQueue_Handle(&txQueue, HAL_Transmit, NULL);		// 循环根据发送缓冲队列的数据块，通过HAL_Transmit调用发送数据
        RxQueue_Handle(&rxQueue, RxMsg_Analyse, NULL);		// 循环将接收队列中的数据块交给RxMsg_Analyse处理
    }
}
```

注意事项：

* **Malloc支持**: 填充到缓冲队列的数据，大部分是通过`Malloc`函数申请新空间存放的，所以需要`Module/Common/Malloc`相关代码支持，在编译时要添加进来
* **Malloc选项**: 如果要填充的数据本身就是通过Malloc申请的，可以在缓冲队列的Add函数中对是否为Malloc进行标记
  * 发送缓冲：add函数中的`mode`参数有`TX_FLAG_IS_MALLOC`标志位，如果为Malloc申请，则需要在mode参数中 算术与 该位
  * 接收缓冲：add函数的第三个参数为`bool isMalloc`，根据是否为Malloc填写即可

### 1.2 函数接口列表以及运行流程

```C
/* 接收缓冲队列相关 */ 
int RxQueue_Add(RxQueueStruct *rxQueue, uint8_t *packet, uint16_t Len, bool isMalloc);                        // 将接收缓冲中的数据填充到接收报文队列中
void RxQueue_Handle(RxQueueStruct *rxQueue, void (*RxPacketHandle)(uint8_t *, uint16_t, void *), void *para); // 接收报文队列处理

/* 发送缓冲队列相关 */
int TxQueue_Add(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t mode);                      // 填充发送队列，包含清除重发以及未使用标志位为1
int TxQueue_AddWithId(TxQueueStruct *txQueue, uint8_t *message, uint16_t length, uint8_t mode, TX_ID_SIZE id); // 填充发送队列，通过Id对发送快进行标记
void TxQueue_Handle(TxQueueStruct *txQueue, bool (*Transmit)(uint8_t *, uint16_t), void *packageParam);        // 发送报文队列处理

void TxQueue_FreeByFunc(TxQueueStruct *txQueue, bool (*func)(TxBaseBlockStruct*, void *), void *para);  // 通过指定函数，释放指定发送块
void TxQueue_FreeById(TxQueueStruct *txQueue, TX_ID_SIZE id);                                           // 通过ID，释放指定发送块
void TxQueue_FreeByIndex(TxQueueStruct *txQueue, uint8_t index);                                        // 通过Index，释放制定发送快
```



**运行流程**：缓冲队列中有三个基本流程单元，分别为填充、处理、释放

* **填充**：后缀为_Add的函数，将数据包填充到队列中，等待处理。注意要根据
* **循环处理**：一般放置在主程序，循环查找可用的数据包，调用处理函数。发送的调用时间和次数与数据块的`mode`参数
* **释放**：接收为调用完处理函数之后自动释放，发送则需要根据发送块`mode`指定的方式，单次/多次 + 自动/手动发送



### 1.3 发送缓冲队列的灵活使用

> 在逐步完善缓冲队列时，发现接收的部分 填充-处理-自动释放 就完全可以满足基本所有的要求，而发送方面就是多种多样的问题，下面就来一一说明



#### 1.3.1 TxQueueStruct的参数设置

```C
typedef struct
{
    uint32_t __time;           // 对不同的发送函数来说，有着不同的发送间隔，需要进行单独设置
    uint8_t __indexCache;      // 索引缓存，配合无序发送使用
    uint8_t usedBlockQuantity; // 已使用的块数量
    uint8_t lastIndex;         // 上一次发送的索引
    bool isTxUnordered;        // 是否无序发送
    uint16_t maxTxCount;       // 最大重发次数
    uint16_t interval;         // 发送间隔
    TxBaseBlockStruct txBlocks[BLOCK_COUNT];
    void (*CallBack_AutoClearBlock)(TxBaseBlockStruct *, BlockFreeMethodEnum );             // 自动清除回调
    int (*CallBack_PackagBeforeTransmit)(TxBaseBlockStruct *, void *, PacketStruct *);      // 在发送前的组包处理，返回组完的包，需要通过Free释放
} TxQueueStruct;     
```



|            参数名             |   类型   | 作用                                                         |
| :---------------------------: | :------: | :----------------------------------------------------------- |
|      _usedBlockQuantity       |   只读   | 记录该缓冲队列中已使用的数据块数量                           |
|          _lastIndex           |   只读   | 记录最后一次调用 处理函数 的数据块索引，配合TxQueue_FreeByIndex函数可以指定清除 |
|         isTxUnordered         |   设置   | 标记该发送缓冲是否为 无序 ，如果为有序，则处理期间，排在后面数据块必须等待前面的数据块释放才会处理 |
|          maxTxCount           |   设置   | 最大重发次数，略                                             |
|           interval            |   读写   | 发送间隔，两次调用 处理函数 的事件间隔，单位为ms，<u>AT指令不能发太快啊大兄弟 |
|    CallBack_AutoClearBlock    | 回调函数 | 在数据块被自动清除前调用，加强对缓冲队列的控制               |
| CallBack_PackagBeforeTransmit | 回调函数 | 将数据块的数据进行二次封包后发送，例如切换为字符串、添加AT指令头等 |



#### 1.3.2 `mode`参数的使用技巧

|   宏定义    | 发送次数 |       清除方式       |                应用                |
| :---------: | :------: | :------------------: | :--------------------------------: |
| TX_ONCE_AC  |   单次   |         自动         |   发一次就清除，多用于主机的回复   |
| TX_ONCE_MC  |   单次   |         手动         |           相对用处比较少           |
| TX_MULTI_AC |   多次   | 自动（超过重发次数） |     设定重发次数，防止不断重发     |
| TX_MULTI_MC |   多次   |         手动         | 不断重发，直到收到回复后手动清除。 |

* 可以在上述4个的基础上，加上`TX_FLAG_IS_MALLOC`表示该数据已经为Malloc的，不需要申请新的空间，重复复制，例如 `TX_ONCE_AC | TX_FLAG_IS_MALLOC`

  * ```c
    TxQueueStruct txQueue;
    uint8_t array = (uint8_t *)Malloc(10);
    
    for(int i=0; i<10; i++)
    {	array[i] = i;	}
    
    TxQueue_Add(&txQueue, array, 10, TX_ONCE_AC | TX_FLAG_IS_MALLOC);		// 在发送模式中添加该标志位，交给发送缓冲来释放
    
    // Free(array)
    ```

* 重发次数的设置在`TxQueueStruct`中的`maxTxCount`设置，在`TX_MULTI_AC`中重发次数超过该值，则会自动清除



#### 1.3.3 不同的手动清除发送块方法

> 在通讯中，经常需要多次放松，收到回复后，根据回复内容进行下一步的操作，BufferQueue提供了多种清除方法，适应不同场合

* **id控制**：`TxQueue_AddWithId`与`TxQueue_FreeById`在有些通讯场合，不同发送报文夹带ID，在填充和清除时都可以通过额外的ID进行标记和清除
* **索引控制**：在`TxQueueStruct`中的`_lastIndex`标记最后一个调用发送处理函数的块，在某些场合可通过`TxQueue_FreeByIndex`直接将其清除
* **函数控制**：如果上面两种简单的手动清除方式都无法满足要求时，也可以自己编写判断函数来找到队列中符合条件的数据块并清除。

```C
void TxQueue_FreeById(TxQueueStruct *txQueue, TX_ID_SIZE id);                                           // 通过ID，释放指定发送块
void TxQueue_FreeByIndex(TxQueueStruct *txQueue, uint8_t index);                                        // 通过Index，释放制定发送快
void TxQueue_FreeByFunc(TxQueueStruct *txQueue, bool (*func)(TxBaseBlockStruct*, void *), void *para);  // 通过指定函数，释放指定发送块
```



### 1.4 针对不同场合的硬件接收

>在接收数据部分，最原始的当属当个字节接收中断的UART了，有了DMA以及空闲中断接收后，串口的接收才变得看起来不那么原始。缓冲队列对这两种接收方式都封装了对应函数，可以帮助使用者安心的关心上层逻辑

#### 1.4.1 单字节 接收

```C
typedef struct
{
    uint8_t _buffer[BUFFER_LENGTH];
    uint16_t count;		// 在填充完成后需要将该位清空
} RxBufferStruct; 		//接收缓冲，包含N字节的缓冲池以及计数器

int ReceiveSingleByte(uint8_t rxByte, RxBufferStruct *rxBuffer);	// 在单字节接收中 调用
```

* **简单原理**：通过`ReceiveSingleByte`将数据填充到缓冲中，计数器+1
* **空闲判断**：对于这种应用，需要建立 **空闲判断** 机制，
  * 在接收时标记时间，
  * 在主循环中判断该时间是否超时，如果超时则将所有的缓冲的数据填充后，将count清零即可

#### 1.4.2 DMA环形 接收

> DMA环形接收：接收硬件使用DMA，DMA设置为环形（到末尾后自动从头开始）。

```C
typedef struct
{
    RxQueueStruct _rxQueue;                     // 发送缓冲队列
    uint8_t _buffer[BUFFER_LENGTH];             // DMA环形缓冲池
    uint16_t __bufferLength;                    // 缓冲池大小
    uint16_t __start;                           // 头位置标记
    uint16_t __end;                             // 尾位置标记
} DmaBufferStruct;

BufferQueue_Conf.h中的相关配置
#define BUFFER_LENGTH   1000            		//缓冲大小，填写报文最大长度的两倍，否则会导致溢出

void DmaBuffer_IdleHandle(DmaBufferStruct *dmaBuffer, uint16_t remainCount);		// DMA接收函数，remainCount为DMA模块中的剩余接收数量，在DMA相关寄存器内读取
```

使用前条件

1. DMA模式，环形模式
2. DMA目的地址指向 `DmaBufferStruct`的`_buffer`成员
3. 找到取值时机，例如串口空闲中断等
4. 使用`DmaBuffer_IdleHandle`将DMA缓冲中的数据取出，存储到``DmaBufferStruct``的`_rxQueue`成员中



## 二、循环/无序 管理块 LoopDisorder

> 在单片机裸机程序中，有如下需求
>
> 1. 存储N条数据，超过N条后从头记录。
> 2. 将最近接收的ID存储，在接收新报文时，需要查询并屏蔽重复报文。
>
> 对于这两种类型，经常会出现计数器溢出或者清除不到位的情况，单独建立对应的结构体还有些繁琐。这种情况下就可以使用 **循环缓存管理块LoopCacheStruct** 与 **无序缓存管理块DisorderCacheStruct 处理**



### 2.1 使用方法

````C
typedef struct
{
    void *__data;
    uint8_t __counter;    
    uint8_t __size;
    uint8_t _maxLen;
} LoopCacheStruct;

typedef struct
{
    void *__data;
    uint8_t __size;
    uint8_t _maxLen;
    uint32_t _usedFlag;    // usedFlag作为是否被占用的标准位，说明该cache最多只能指向32个成员的对象数组
} DisorderCacheStruct;
````

缓存管理块使用的四个步骤

* **初始化**：将管理块与已有对象数组属性进行匹配，例如指针地址、对象宽度、最大个数等
* **插入**：将数据添加到对象数组，循环缓存是根据计数器索引来插入，无序缓存是通过空闲标志位来处理（32位的标志位，所以无序缓存最多智能管理32个成员的数组）
* **获取/删除**：通过索引使用宏定义即可访问，无序缓存还提供了查找第一个被使用的成员，并返回该成员索引，后续可通过该索引将其从数组中删除
* **清空**：在某些场合，在使用缓存使用完成后，需要清空缓存，也提供了相应的接口



### 2.2 接口列表

```C

#define CACHE_GET(cache, i) ((void *)((uint32_t)cache->__data + cache->__size * i))								// 从缓存中获取成员指针(void *)

/*****************************循环缓存*****************************/
void LoopCache_Init(LoopCacheStruct *loopCache, void *data, uint8_t size, uint8_t maxLen);                      // 初始化
int LoopCache_Append(LoopCacheStruct *loopCache, void *newData);                                                // 添加新成员
void LoopCache_ClearNull(LoopCacheStruct *loopCache, bool (*nullCondition)(void *, void *), void *param);       // 通过函数指针清除空成员，并将非空成员前推
void LoopCache_Clear(LoopCacheStruct *loopCache);                                                               // 清空

/*****************************无序缓存*****************************/
void DisorderCache_Init(DisorderCacheStruct *disorderCache, void *data, uint8_t size, uint8_t maxLen);          // 初始化
int DisorderCache_Append(DisorderCacheStruct *disorderCache, void *newData);                                    // 添加新成员
int DisorderCache_Get(DisorderCacheStruct *disorderCache);                                             			// 获取第一个已占用的索引
void DisorderCache_Remove(DisorderCacheStruct *disorderCache, uint8_t index);                                   // 根据索引将成员从缓存中删除
void DisorderCache_Clear(DisorderCacheStruct *disorderCache);                                                   // 清空
```



```C
uint32_t idQueue[10];
DisorderCacheStruct idDisorder;

DisorderCache_Init(&idDisorder, (void *)idQueue, sizeof(uint32_t), 10);				// 初始化，将管理块与对象数组信息

uint32_t temp32u = 100;
DisorderCache_Append(&idDisorder, (void *)&temp32);									// 添加新成员

int index = DisorderCache_Get(&idDisorder);											// 获取第一个占用的成员索引
if (index != -1)
{
    temp32u = *(uint32_t *)(CACHE_GET(idDisorder, index));							// 通过宏定义取出成员指针
    DisorderCache_Remove(&idDisorder, index);										// 将该成员踢出成员数组中
}
```

