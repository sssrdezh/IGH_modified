/******************************************************************************
 *
 *  $Id: master.h,v bc2d4bf9cbe5 2012/09/06 18:22:24 fp $
 *
 *  Copyright (C) 2006-2009  Florian Pose, Ingenieurgemeinschaft IgH
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

#include "include/ecrt.h"

/*****************************************************************************/

struct ec_master {
    int fd; // file descriptor 主站的文件描述符
    uint8_t *process_data; // 指向过程数据的指针
    size_t process_data_size; // 过程数据的大小

    ec_domain_t *first_domain; // 指向第一个 domain 的指针
    ec_slave_config_t *first_config; // 指向第一个 slave config 的指针
};

/*****************************************************************************/

void ec_master_clear(ec_master_t *);

/*****************************************************************************/
