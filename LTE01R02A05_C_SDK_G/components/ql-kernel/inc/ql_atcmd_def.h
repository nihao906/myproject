/**  @file
  ql_atcmd_def.h

  @brief
  This file is used to define at command for different Quectel OPEN SDK Project.

*/

/*================================================================
  Copyright (c) 2021 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------



=================================================================*/
#ifndef QL_ATCMD_DEF_H
#define QL_ATCMD_DEF_H

/*****************  AT command defination  *******************/
/*
* Each command should be located in a single line
* <option> should be fixed as 0
*
* -------------------------------------------------------
* |  AT name  |     AT command handler     |   option   |
* -------------------------------------------------------
*/
//at demo
+QLATDEMO,          ql_exec_atdemo_cmd,             0


+QLVER,             ql_exec_ver_cmd,                0

#ifdef CONFIG_QUEC_PROJECT_FEATURE_TEESDK_VTRUST_AT
+VTRUST,            vtrust_at_cmd,                  0
#endif

#endif

