[TOC]

> 在写单片机裸机的情况下，不同的项目之间的代码的移植方式比较原始，通过文件的拷贝。一些比较好的控制代码也耦合在项目当中。这些代码也是我长期写单片机程序整理出来的小型控制块，针对通讯、内存管理、特殊转换等常用的较为繁琐的代码进行提炼。

```C
├── Common					// 常用功能，例如内存管理、常用转换、延时等
├── Communicate				// 通讯控制，具体看内部README
├── DataStruct				// 小型”数据结构“，例如通讯用的缓冲队列等，具体看内部README
├── ESP8266					// ESP8266模块控制
├── I2C_Chip				// I2C芯片驱动，包含模拟I2C
├── NB						// NB模块驱动控制
├── HalFrame				// 用于一些硬件的固定流程，I/O 按键 存储 电机等
├── SPI_Chip				// SPI芯片驱动，
├── ZcProtocol				// 我司的通讯协议相关解包封包

├── Module.h				// 总的头文件，引用该头文件即包含所有可用接口
├── ModuleHandle.exe		// 通过该可执行文件，将*.Default文件去掉后缀，或将更新的文件加上后缀
├── Module_Conf.h.Default	// 环境配置文件默认选项，包含系统时间、真实时间、时间戳、库头文件等
└── README.md
```



## 一、模块使用规则/说明

### 1.1 代码隔离

> 为了能够让多人使用和更新，必须将硬件相关代码、业务相关代码与控制代码进行隔离。该模块使用了这几种方法

* **回调函数**：将部分硬件功能封装为回调函数的形式，通过赋值或者函数指针的方式
* **宏定义**：在模块中有后缀为`_HAL.h`的头文件，就是为了存放直接调用硬件的宏定义
* **空函数**：在模块中有后缀为`__Handle.c/h`的文件，预留了一些空函数，用户可以直接将代码填写到空函数内。
* **配置文件**：在模块中后缀为`_Conf.h`的文件，通过修改其中的某些配置以使用不同业务。

**注意事项**：该代码段是通过git管理的，通过宏定义、空函数以及配置文件的方法会导致每个人的代码仓库中的代码混乱，为了解决这种问题，加入.gitignore规则，将

为了能够可以不断更新代码又不影响所有的使用者。

* 加入了.gitignore规则，将上述三种文件加入忽略列表。

* 需要修改的文件的文件名末尾加上.Default作为默认，用户在收到后去掉后缀即可任意修改，并且默认文件更新不影响修改代码
  * `Sys_Conf.h` ==> `Sys_Conf.h.Default`
  * `ESP8266_Handle.c`   ==> `ESP8266_Handle.c.Default`

* 为了方便使用，`ModuleHandle.exe`该文件可批量的完成去后缀、加后缀的功能


### 1.2 头文件的引用

* 使用：直接包含"Module.h"即可

* 编写：Module中模块不能直引用Module.h，需要分开添加所用模块的头文件

* ```C
  // BufferQueue.c中，添加不同模块的头文件以及自己模块的头文件
  #include "../Module_Conf.h"
  #include "../Common/Common.h"
  #include "BufferQueue.h"
  ```

### 1.3 命名规则
  * 模块总头文件 `xxx.h`，添加该文件夹下所有模块的头文件以及配置
  * 接口文件 `xxx_API.c/h`，模块的主要代码，要与硬件隔离并提供简单接口
  * *配置文件* `xxx_Conf.h`，针对模块的配置头文件。隔离文件
  * *硬件接口文件* `xxx_HAL.h`，针对不同MCU的简单硬件宏定义，隔离文件
  * *业务处理文件* `xxx_Handle.c/h`，模块的业务处理模块，不同场景存在不同的配置以及处理方式，隔离文件

 

## 二、使用建议

### 2.1 内部时间维护

> 该模块需要维护三个时间，所有的程序的运行都基于这三个时间

* sysTime：毫秒，复位时要求清零
* realTime：秒，复位时要求清零
* timeStamp：秒，要求与真实时间戳进行同步

这三个时间都是使用宏定义的方式，如果单片机没有类似的现成的寄存器，就需要新建32位的变量。

例如

```
uint32_t sysTimeCounter,realTimeCounter,timeStampCounter

#define sysTime	sysTimeCounter
#define realTime realTimeCounter
#define timeStamp timeStampCounter
```



### 2.2 Malloc

> 由于单片机的堆通常有限并且没有mmu，直接使用默认的malloc不但代码量会大，并且不容易管理，所以在这里推荐使用我自己写的Malloc与Free，用法与malloc与free相同，并且不需要初始化。预留了很多可以查看占用的变量，可以在调试时查看，具体的看DataStruct里的Readme



### 2.3 状态机

> 在单片机非RTOS的写法当中，状态机基本上是肯定会出现的，模块也为这里做了微不足道的省事的优化

```c
typedef enum
{
    Process_Init = 0,
    Process_Idle,
    Process_Start,
    Process_Run,
    Process_BeforeFinish,
    Process_Finish,
    Process_LongWait,
    Process_Wait,
    Process_Reset,
    Process_ResetWait,
    Process_OrderAt,
    Process_OrderAtWait,
    Process_ErrorTrigged,
    Process_ErrorHandle,
    Process_Lock,
} ProcessEnum;

typedef struct
{
    ProcessEnum current;
    ProcessEnum last;
}ProcessStruct;

#define PROCESS_CHANGE(processObj, process)     \
    {                                           \
        processObj.last = processObj.current;   \
        processObj.current = process;           \
    }
```

在写新的状态机的时候，可以将添加一个`ProcessStruct`对象，状态机的状态选择可以使用`ProcessEnum`列举的。并使用`PROCESS_CHANGE`进行切换，在回头调试的时候就可以知道在切换前的上一个状态是哪里，非常有帮助



## 三、部分常用模块

### 3.1 BufferQueue

> 在通讯或者其他场合，很多时候都需要有队列，其核心在于，延迟处理。BufferQueue就是用在这种场合。

BufferQueue分为两类

* RxBufferQueue：接收缓冲，特点是填充后只处理一次就立马清除
* TxBufferQueue：发送缓冲，
  * 单次、多次发送
  * 发送次数、发送超时限制
  * 有序发送、无序发送

具体的请参照`DataStruct/`中的README

