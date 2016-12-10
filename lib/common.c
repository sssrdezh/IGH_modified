/******************************************************************************
 *
 *  $Id: common.c,v 3bdd7a747fae 2012/09/20 13:28:25 fp $
 *
 *  Copyright (C) 2006-2012  Florian Pose, Ingenieurgemeinschaft IgH
 *
 *  This file is part of the IgH EtherCAT master userspace library.
 *
 *  The IgH EtherCAT master userspace library is free software; you can
 *  redistribute it and/or modify it under the terms of the GNU Lesser General
 *  Public License as published by the Free Software Foundation; version 2.1
 *  of the License.
 *
 *  The IgH EtherCAT master userspace library is distributed in the hope that
 *  it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the IgH EtherCAT master userspace library. If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  The license mentioned above concerns the source code only. Using the
 *  EtherCAT technology and brand is only permitted in compliance with the
 *  industrial property and similar rights of Beckhoff Automation GmbH.
 *
 *****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/socket.h>

#include "ioctl.h"
#include "master.h"

/*****************************************************************************/

unsigned int ecrt_version_magic(void)
{
    // #define ECRT_VERSION(a, b) (((a) << 8) + (b))
    // #define ECRT_VERSION_MAGIC ECRT_VERSION(ECRT_VER_MAJOR, ECRT_VER_MINOR)
    return ECRT_VERSION_MAGIC;
}

/*****************************************************************************/

ec_master_t *ecrt_request_master(unsigned int master_index)
{
    // 尝试打开一个 master.
    ec_master_t *master = ecrt_open_master(master_index);
    if (master) // 如果打开成功,进if{}.
    {
        // 预定一个 master,如果返回值 <0,说明失败,进if{}.
        if (ecrt_master_reserve(master) < 0)
        {
            // 清除 master
            ec_master_clear(master);
            free(master);
            master = NULL;
        }
    }

    return master;
}

/*****************************************************************************/

#define MAX_PATH_LEN 64

ec_master_t *ecrt_open_master(unsigned int master_index)
{
    char path[MAX_PATH_LEN]; // 声明 master module 的路径.
    ec_master_t *master = NULL; // 声明 master.
    ec_ioctl_module_t module_data;
    int ret; // 声明 ioctl 的返回值.

    master = malloc(sizeof(ec_master_t)); // 为 master 分配内存空间.

    // 如果分配内存空间失败,进if{}.
    if (!master)
    {
        // 向 stderr 打印错误信息.
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }

    // 初始化 master 结构体
    master->process_data = NULL; // 指向过程数据的指针地址
    master->process_data_size = 0; // 过程数据的大小
    master->first_domain = NULL; // 指向第一个 domain 的指针地址
    master->first_config = NULL; // 指向第一个 slave config 的指针地址

    // snprintf() 函数包含在stdio.h 中.
    // 这里我们假设 master_index 的值为0,
    // 若定义了 USE_RTDM, 则 path = "EtherCAT0\0",
    // 若未定义 USE_RTDM, 则 path = "/dev/EtherCAT0\0".
    snprintf(path, MAX_PATH_LEN - 1,
#ifdef USE_RTDM
            "EtherCAT%u",
#else
            "/dev/EtherCAT%u",
#endif
            master_index);

    // open()函数需要 sys/types.h, sys/stat.h, fcntl.h
    // 若定义了 USE_RTDM ,则使用 rt_dev_open() 函数打开 EtherCAT0,
    // 若未定义 USE_RTDM,则使用 open() 函数打开 /dev/EtherCAT0.
    // 可读可写
    // 如果成功,则返回一个文件描述符, 失败则返回-1.
#ifdef USE_RTDM
    master->fd = rt_dev_open(path, O_RDWR);
#else
    master->fd = open(path, O_RDWR);
#endif

    //检查open是否成功, 若fd为错误码,进 if{}.
    if (EC_IOCTL_IS_ERROR(master->fd))
    {
        // 向 stderr 中打印错误信息.
        fprintf(stderr, "Failed to open %s: %s\n", path,
                strerror(EC_IOCTL_ERRNO(master->fd)));
        goto out_clear;
    }

    // ioctl是设备驱动程序中对设备的I/O通道进行管理的函数.
    // #define EC_IOCTL_MODULE  EC_IOR(0x00, ec_ioctl_module_t)
    // #define EC_IOR(nr, type)   _IOR(EC_IOCTL_TYPE, nr, type)
    // #define EC_IOCTL_TYPE 0xa4
    // ret = ioctl(master->fd, _IOR(0xa4, 0x00, ec_ioctl_module_t), &module_data)
    // _IOR用于创建从设备读取数据的命令.
    ret = ioctl(master->fd, EC_IOCTL_MODULE, &module_data);

    // 检查 ioctl 是否成功, 若 ret 错误,进if()
    if (EC_IOCTL_IS_ERROR(ret))
    {
        // 向 stderr 中打印错误信息.
        fprintf(stderr, "Failed to get module information from %s: %s\n",
                path, strerror(EC_IOCTL_ERRNO(ret)));
        goto out_clear;
    }

    // 若 ioctl 成功,则数据传回 module_data
    // 检查 version magic
    // #define EC_IOCTL_VERSION_MAGIC 27 (master/ioctl.h)
    // 如果 version magic 不相等,进if()
    if (module_data.ioctl_version_magic != EC_IOCTL_VERSION_MAGIC)
    {
        // 向 stderr 中打印错误信息.
        fprintf(stderr, "ioctl() version magic is differing:"
                " %s: %u, libethercat: %u.\n",
                path, module_data.ioctl_version_magic,
                EC_IOCTL_VERSION_MAGIC);
        goto out_clear;
    }

    return master;

out_clear:
    ec_master_clear(master);
    free(master);
    return 0;
}

/*****************************************************************************/

void ecrt_release_master(ec_master_t *master)
{
    ec_master_clear(master);
    free(master);
}

/*****************************************************************************/
