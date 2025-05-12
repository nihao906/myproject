#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ql_fs.h>
#include "ql_osi_def.h"
#include "ql_api_osi.h"
#include "ql_api_nw.h"
#include "ql_log.h"
#include "ql_api_datacall.h"
#include "ql_mqttclient.h"
#include "ql_ssl.h"
#include "cJSON.h"
#include "data_init.h"
#include "snprintf_fun.h"
#include "construct_cjson.h"
#include "process_para.h"

#define QL_MQTT_LOG_LEVEL QL_LOG_LEVEL_INFO
#define QL_MQTT_LOG(msg, ...)			QL_LOG(QL_MQTT_LOG_LEVEL, "ql_MQTT", msg, ##__VA_ARGS__)
#define QL_MQTT_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_MQTT", msg, ##__VA_ARGS__)

//extern osiTimer_t *nvram_timer;

void load_from_nvram(const char *key, void *data, size_t size, void *last_data) {
    int ret = ql_nvm_fread(key, data, size, 1);
    if (ret != size) {
        memset(data, 0, size);
    }
    memcpy(last_data, data, size);
}

void save_to_nvram(const char *key, void *data, size_t size, void *last_data) 
{
    if (memcmp(data, last_data, size) != 0) {  
        int err = ql_nvm_fwrite(key, data, size, 1);
        if (err > 0) {
            memcpy(last_data, data, size);
            QL_MQTT_LOG("Saved to %s", key);
        } else {
            QL_MQTT_LOG("Write %s failed", key);
        }
    }
}

