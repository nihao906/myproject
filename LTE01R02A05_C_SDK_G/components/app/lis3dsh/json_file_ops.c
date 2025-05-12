#include "json_file_ops.h"
#include <stdint.h>
#include <string.h>
#include <ql_fs.h>

#include "ql_log.h"
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "json_ops", msg, ##__VA_ARGS__)

static void get_dirname(char *filename, char *dirname)
{
    memcpy(dirname, filename, strlen(filename));
    char *p = strrchr(dirname, '/');
    if (p) *p = '\0';
    else memcpy(dirname, "SD:/", sizeof("SD:/"));   
}


/**
 * @brief 加载json配置文件
 *
 * @param name
 * @return cJSON*
 */
static cJSON *load_json_cfg_file(const char *name)
{
    cJSON *json = NULL;
    int8_t *buff_p = NULL;
    int32_t fileFd = -1;
    int file_size = 0;
    int32_t ret = 0;

    if (name == NULL)
    {
        LOGI("file name is NULL");
        return NULL;
    }

    fileFd = ql_fopen(name, "rb");
    if (fileFd < 0)
    {
        LOGI("LSAPI_FS_Open failed, file:%s\n", name);
        return NULL;
    }
    file_size = ql_fseek(fileFd, 0, QL_SEEK_END);
    ql_fseek(fileFd, 0, QL_SEEK_SET);

    buff_p = (int8_t *)malloc(file_size + 1);
    if (buff_p == NULL)
    {
        LOGI("LSAPI_OSI_Malloc failed, size:%d\n", file_size + 1);
        goto exit0;
    }
    memset(buff_p, 0x00, file_size + 1);

    ret = ql_fread((void *)buff_p, file_size, 1, fileFd);
    if (ret <= 0)
    {
        LOGI("LSAPI_FS_Read failed. ret:%d\n", ret);
        goto exit1;
    }
    buff_p[file_size] = '\0';

    json = cJSON_Parse((const char *)buff_p);
    if (json == NULL)
    {
        LOGI("JSON_Parse Err ret%s", cJSON_GetErrorPtr());
    }

exit1:
    free(buff_p);
    buff_p = NULL;
exit0:
    ql_fclose(fileFd);
    return json;
}


/**
 * @brief 保存json配置文件
 *
 * @param json
 * @param name
 */
static void save_json_cfg_file(cJSON *json, const char *name)
{
    int8_t *buff_p = NULL;
    int32_t buf_len = 0;
    int32_t fileFd = -1;
    char dirname[20] = {0};
    int ret = 0;

    if (name == NULL || json == NULL)
    {
        LOGI("save json to file faild, name or json is null");
        return;
    }

    /* 没有目录创建目录 */ 
    get_dirname(name, dirname);
    QDIR *dirp = ql_opendir(dirname);
	if(dirp == NULL)
    {
		LOGI("create dir %s", dirname);
        ql_mkdir(dirname, 0);
    }
    else
        ql_closedir(dirp);

    fileFd = ql_fopen(name,"wb");
    if (fileFd < 0)
    {
        LOGI("LSAPI_FS_Open failed, file:%s\n", name);
        return;
    }

    buff_p = cJSON_Print(json);
    if (buff_p == NULL)
    {
        LOGI("cJSON_Print failed, buff_p is null\n");
        goto exit0;
    }
    buf_len = strlen(buff_p);

    ret = ql_fwrite((void *)buff_p, buf_len, 1,fileFd);
    if (ret <= 0)
    {
        LOGI("LSAPI_FS_Write failed. ret:%d\n", ret);
        goto exit1;
    }

exit1:
    free(buff_p);
exit0:
    ql_fclose(fileFd);
    return;
}


/**
 * @brief 保存震动配置参数
 */
void save_shake_cfg(float feq_min, float feq_max, float a_th, const char *filePath)
{
    cJSON *cfg_json = cJSON_CreateObject();
    if (cfg_json != NULL)
    {
        cJSON_AddNumberToObject(cfg_json, "feq_min", feq_min);
        cJSON_AddNumberToObject(cfg_json, "feq_max", feq_max);
        cJSON_AddNumberToObject(cfg_json, "a_th", a_th);
        save_json_cfg_file(cfg_json, filePath);
        cJSON_Delete(cfg_json);
    }
}


/**
 * @brief 加载震动配置参数
 */
int load_shake_cfg(float *feq_min_q, float *feq_max_q, float *a_th_q, const char *filePath)
{
    cJSON *node, *child;
    cJSON *cfg_json = load_json_cfg_file(filePath);
    if (cfg_json != NULL)
    {
        node = cJSON_GetObjectItem(cfg_json, "feq_min");
        if (node != NULL)
        {
            *feq_min_q = node->valuedouble;
        }
        node = cJSON_GetObjectItem(cfg_json, "feq_max");
        if (node != NULL)
        {
            *feq_max_q = (double)node->valuedouble;
        }
        node = cJSON_GetObjectItem(cfg_json, "a_th");
        if (node != NULL)
        {
            *a_th_q = (double)node->valuedouble;
        }
        cJSON_Delete(cfg_json);
    }
    else
    {
        LOGI("cfg_json is null, filePath:%s\n", filePath);
        return -1;
    }
    return 0;
}



