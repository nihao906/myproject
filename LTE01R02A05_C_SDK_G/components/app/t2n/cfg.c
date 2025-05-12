#include "./cfg.h"

#include <string.h>
#include <stdlib.h>
#include "ql_api_osi.h"
#include "ql_log.h"
#include "cJSON.h"
#include "ql_fs.h"
#include "ql_api_dev.h"
// #include "hx_device.h"
// #include "can_device.h"
#define MAGIC_CFG_OFFSET 0
#define METER_CFG_OFFSET 16
#define SERIAL1_CFG_OFFSET 32
#define SERIAL2_CFG_OFFSET 48
#define DSC_CFG_OFFSET 64
// #define _USE_CAN_ 0

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "t2n_cfg", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "t2n_cfg", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "t2n_cfg", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "t2n_cfg", msg, ##__VA_ARGS__)

#define LOG_TAG "cfg"
extern uint16_t gWordVar[];
// dsc_cfg_t dsc_cf;// = {{0,0,0,0},0,"123456"};

// meter_cfg_t meter_cfg = {DEV_KMS_GENARATOR,3,1,0x2222,10,10,10,10};
const char hw_ver[32] = "MCGPS_V1.0";
const char sw_ver[32] = "HXGPS20170122";

int save_cfg_file(cJSON *json, char *name)
{
    int n;
    // uint16_t fname[32];
    int fd;
    char *buff_p;
    UINT writedLen;
    fd = ql_fopen(name, "w");
    if (fd < 0)
    {
        LOGE("ql_fopen():Create File Fail,and Return Error is %x ", fd);
        return -1;
    }
    buff_p = cJSON_PrintUnformatted(json);
    if (buff_p == NULL)
    {
        LOGE("mem alloc fail!");
        ql_fclose(fd);
        return -2;
    }
    n = strlen(buff_p);
    writedLen = ql_fwrite(buff_p, n, 1, fd);
    if (n < 0)
    {
        LOGE("ql_fwrite():Write File Fail,and Return Error is %d", writedLen);
    }
    ql_fclose(fd);
    free(buff_p);
    if (writedLen > 0)
    {
        return 0;
    }
    return writedLen;
}

cJSON *load_cfg_file(char *name)
{
    // uint16_t fname[32];
    cJSON *json;
    // char *buff_p;
    // char *p;
    int32_t fd;
    int readLen, filesize;
    fd = ql_fopen(name, "rb");
    if (fd < 0)
    {
        LOGE("ql_fopen():Open [%s] Fail,and Return Error is %x ", name, fd);
        return NULL;
    }
    filesize = ql_fseek(fd, 0, SEEK_END); // 读取文件大小
    if (filesize > 0 && filesize < 1024 * 16)
    {
        char *buff_p = malloc(filesize);
        if (buff_p == NULL)
        {
            LOGE("malloc fail!");
            ql_fclose(fd);
            return NULL;
        }
        ql_fseek(fd, 0, SEEK_SET);
        readLen = ql_fread(buff_p, filesize, 1, fd);
        if (readLen < 0)
        {
            LOGE("ql_fread():Read File Fail,and Return Error is %d", readLen);
            free(buff_p);
            ql_fclose(fd);
            return NULL;
        }
        buff_p[filesize] = '\0';
        ql_fclose(fd);
        json = cJSON_Parse(buff_p);
        free(buff_p);
        if (json == NULL)
        {
            LOGE("JSON_Parse Err ret%s", cJSON_GetErrorPtr());
        }
    }
    return NULL;
}

void load_hw_cfg(void)
{
}

int load_dsc_cfg(dsc_cfg_t *dsc_cfg)
{
    int ret = -1;
    cJSON *node;
    cJSON *cfg_json = load_cfg_file("dsc_cfg.json");
    if (cfg_json != NULL)
    {
        node = cJSON_GetObjectItem(cfg_json, "host1");
        if (node != NULL)
        {
            strncpy(dsc_cfg->host[0], node->valuestring, 24);
            LOGD("host1=%s", node->valuestring);
        }
        node = cJSON_GetObjectItem(cfg_json, "port1");
        if (node != NULL)
        {
            dsc_cfg->port[0] = node->valueint;
            LOGD("port1=%d", node->valueint);
        }
        node = cJSON_GetObjectItem(cfg_json, "host2");
        if (node != NULL)
        {
            strncpy(dsc_cfg->host[1], node->valuestring, 24);
            LOGD("host2=%s", node->valuestring);
        }
        node = cJSON_GetObjectItem(cfg_json, "port2");
        if (node != NULL)
        {
            dsc_cfg->port[1] = node->valueint;
            LOGD("port2=%d", node->valueint);
        }
        cJSON_Delete(cfg_json);
        ret = 0;
    }
    return ret;
}

int save_dsc_cfg(dsc_cfg_t *dsc_cfg)
{
    int ret = -1;
    cJSON *cfg_json = cJSON_CreateObject();
    if (cfg_json != NULL)
    {
        cJSON_AddStringToObject(cfg_json, "host1", dsc_cfg->host[0]);
        cJSON_AddStringToObject(cfg_json, "host2", dsc_cfg->host[1]);
        cJSON_AddNumberToObject(cfg_json, "port1", dsc_cfg->port[0]);
        cJSON_AddNumberToObject(cfg_json, "port2", dsc_cfg->port[1]);
        ret = save_cfg_file(cfg_json, "dsc_cfg.json");
        cJSON_Delete(cfg_json);
    }
    return ret;
}

int load_sensor_cfg(senser_cfg_t *cfg)
{
    int ret = -1;
    cJSON *node, *child;
    cJSON *cfg_json = load_cfg_file("serser_cfg.json");
    if (cfg_json != NULL)
    {
        child = cfg_json->child;
        while (child != NULL)
        {
            node = cJSON_GetObjectItem(child, "temp_type");
            if (node != NULL)
            {
                cfg->temp_type = node->valueint;
            }
            node = cJSON_GetObjectItem(child, "temp_count");
            if (node != NULL)
            {
                cfg->temp_type = node->valueint;
            }
            node = cJSON_GetObjectItem(child, "temp_slave");
            if (node != NULL)
            {
                cfg->temp_slave = node->valueint;
            }
            node = cJSON_GetObjectItem(child, "cmv_type");
            if (node != NULL)
            {
                cfg->cmv_type = node->valueint;
            }
            child = child->next;
        }
        cJSON_Delete(cfg_json);
        ret = 0;
    }
    return ret;
}

int save_sensor_cfg(senser_cfg_t *cfg)
{
    cJSON *cfg_json = cJSON_CreateObject();
    if (cfg_json != NULL)
    {
        cJSON_AddNumberToObject(cfg_json, "temp_type", cfg->temp_type);
        cJSON_AddNumberToObject(cfg_json, "temp_count", cfg->temp_count);
        cJSON_AddNumberToObject(cfg_json, "temp_slave", cfg->temp_slave);
        cJSON_AddNumberToObject(cfg_json, "cmv_type", cfg->cmv_type);
        save_cfg_file(cfg_json, "serser_cfg.json");
        cJSON_Delete(cfg_json);
        return 0;
    }
    return -1;
}
