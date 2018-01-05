# 嵌入式模块整理

> 在工作中常用的模块进行整理，平台为STM32



## 一、模块的使用

#### 1.1 复制Module文件夹

将文件夹复制到工程文件project.eww的上级目录，目录树如下

```
┍ Module
│	├ Common
│	├ ESP8266
│	├ UartDma
│	└ ZcProtocol
┕ EWARM
	┕ project.eww
```



#### 1.2 在IAR中添加模块文件路径

```c
$PROJ_DIR$/../Module/UartDma
$PROJ_DIR$/../Module/Common
$PROJ_DIR$/../Module/ZcProtocol
$PROJ_DIR$/../Module/ESP8266
```

