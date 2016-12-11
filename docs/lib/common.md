# common.c

common.c 文件主要实现了 include/ecrt.h 中的全局函数

## 头文件
本源文件包含了以下头文件:

系统头文件:
- <sys/types.h>
- <sys/stat.h>
- <fcntl.h>
- <stdio.h>
- <stdlib.h>
- <string.h>
- <sys/mman.h>
- <unistd.h>
- <sys/socket.h>

源码头文件:
- "ioctl.h"
- "master.h"

## 函数列表

1.``` unsigned int ecrt_version_magic(void) ```
> 本函数的作用是返回 IGH 的 version magic.
>
> 返回值为 unsigned int 类型, 用到了低 16 位.
>
> 本工程的主版本号为 1, 次版本号为5,
> 所以返回值的16进制表示为: 0x0015.

2. ``` ec_master_t *ecrt_request_master(unsigned int master_index) ```
> 本函数的作用是为应用程序请求一个主站.
>
> 返回值为 ec_master_t 类型,我们来看一下该类型的定义:
>> 在ecrt.h 中有以下代码:
>> ``` c
>> struct ec_master;
>> typedef struct ec_master ec_master_t; /**< \见 ec_master */
>> ```
>>
>> 而在 master.h 中有以下代码:
>> ``` c
>> struct ec_master {
>>    int fd; // file descriptor 主站的文件描述符
>>    uint8_t *process_data; // 指向过程数据的指针
>>    size_t process_data_size; // 过程数据的大小
>>    ec_domain_t *first_domain; // 指向第一个 domain 的指针
>>    ec_slave_config_t *first_config; // 指向第一个 slave config 的指针
>> };
>> ```
>>
>>
