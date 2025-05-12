// SPDX-License-Identifier: Apache-2.0
// Copyright 2015-2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/** prevent recursive inclusion **/
#ifndef __TRACE_H
#define __TRACE_H

#ifdef __cplusplus
extern "C" {
#endif

/** Includes **/
#include "stdio.h"
#include "ql_log.h"
#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "assert", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "assert", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "assert", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "assert", msg, ##__VA_ARGS__)
/** constants/macros **/
#define DEBUG_TRANSPORT                   1
#define DEBUG_HEX_STREAM_PRINT            0
#ifndef assert
#define assert(x) do { if (!(x)) { \
	LOGI("Aborting at: %s:%u\n", __FILE__, __LINE__); while(1);}} while (0);
#endif

/** Exported Structures **/

/** Exported variables **/

/** Inline functions **/

/** Exported Functions **/
void print_hex_dump(uint8_t *buff, uint16_t rx_len, char *human_str);

#ifdef __cplusplus
}
#endif

#endif

