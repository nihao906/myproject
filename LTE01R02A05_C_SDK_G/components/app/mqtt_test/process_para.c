#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_osi_def.h"
#include "ql_api_osi.h"
#include "ql_api_nw.h"
#include "ql_log.h"
#include "ql_api_datacall.h"
#include "ql_mqttclient.h"
#include "ql_ssl.h"
#include "cJSON.h"
#include "construct_cjson.h"
#include "data_init.h"
#include "SaveData.h"

#define QL_MQTT_LOG_LEVEL QL_LOG_LEVEL_INFO
#define QL_MQTT_LOG(msg, ...)			QL_LOG(QL_MQTT_LOG_LEVEL, "ql_MQTT", msg, ##__VA_ARGS__)
#define QL_MQTT_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_MQTT", msg, ##__VA_ARGS__)

extern mqtt_client_t  mqtt_cli;
//extern int report_flag = 0;
int i=0;

int process_0parameter_instruction(const unsigned char *payload, unsigned short payload_len)
{
    int ret;
    unsigned char *buffer = NULL;
    unsigned short buffer_len = 0;

    if (payload_len == 0) {
        QL_MQTT_LOG("Error: Empty payload");
        return 0;
    }

    char *payload_str = (char *)malloc(payload_len + 1);
    if (!payload_str) {
        QL_MQTT_LOG("Memory allocation failed");
        return 0;
    }
    memcpy(payload_str, payload, payload_len);
    payload_str[payload_len] = '\0';
	
    char *cmd = NULL, *apikey = NULL, *msgid = NULL;
    char *token = strtok(payload_str, "&");
    while (token != NULL) {
        if (strncmp(token, "$cmd=", 5) == 0) {
            cmd = token + 5;
        } else if (strncmp(token, "apikey=", 7) == 0) {
            apikey = token + 7;
        } else if (strncmp(token, "msgid=", 6) == 0) {
            msgid = token + 6;
        }
        token = strtok(NULL, "&");
    }

    if (!cmd || !apikey || !msgid) {
        QL_MQTT_LOG("Error: Missing required fields (cmd, apikey, msgid)");
        free(payload_str);
        return 0;
    }

    if (strcmp(apikey, "VALID_KEY") != 0) {
        QL_MQTT_LOG("Error: Invalid apikey '%s'", apikey);
        free(payload_str);
        return 0;
    }

    char response[256];
	if(strcmp(cmd, "reqtime") == 0){                           //获取设备时间
		snprintf(response, sizeof(response), "$cmd=reqtime & time=%s & msgid=%s", Time, msgid);
	}else if (strcmp(cmd, "getstatus") == 0){  //*****************************************************************
        if (construct_data_buffer_cjson("Hello, MQTT!", 12, &buffer, &buffer_len, 1) == 0)
        {//不能用%s将buffer读出来，buffer不是'\0'结尾，且有0x00和0x05的不可打印字符。
            ret = ql_mqtt_publish(&mqtt_cli, "test", "$cmd=getstatus & state=", 23, 3, 0, NULL, NULL);
            if (ret != MQTTCLIENT_SUCCESS) {
                QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
            } else {
                QL_MQTT_LOG("Response published: %s", response);
            }

            ret =  ql_mqtt_publish(&mqtt_cli, "test", (const char *)buffer, buffer_len, 3, 0, NULL, NULL);
            if (ret != MQTTCLIENT_SUCCESS) {
                QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
            } else {
                QL_MQTT_LOG("Response published: %s", response);
            }

            ret = ql_mqtt_publish(&mqtt_cli, "test", "msgid=", 6, 3, 0, NULL, NULL);
            if (ret != MQTTCLIENT_SUCCESS) {
                QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
            } else {
                QL_MQTT_LOG("Response published: %s", response);
            }

            ret = ql_mqtt_publish(&mqtt_cli, "test", msgid, strlen(msgid), 3, 0, NULL, NULL);
            if (ret != MQTTCLIENT_SUCCESS) {
                QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
            } else {
                QL_MQTT_LOG("Response published: %s", response);
            }
        }  
	}else if (strcmp(cmd, "reboot") == 0){	                   //远程控制重启设备
		snprintf(response, sizeof(response), "$cmd= reboot & result=succ & msgid=%s", msgid);
		snprintf(response, sizeof(response), "$cmd= reboot & result=fail & msgid=%s", msgid);
	}else if (strcmp(cmd, "getsensorID") == 0){                //返回所有传感器类型ID
        for(i = 0 ; i<2 ; i++)
        {
            snprintf(response, sizeof(response), "$cmd=getsensorID & sensor_id=%s & msgid=%s", sensor_type[i], msgid);
        }   
	}else if (strcmp(cmd, "sample") == 0){  //*****************************************************************
        if (construct_data_buffer_cjson("Hello, MQTT!", 12, &buffer, &buffer_len, 2) == 0)
        {//不能用%s将buffer读出来，buffer不是'\0'结尾，且有0x00和0x05的不可打印字符。
            ret = ql_mqtt_publish(&mqtt_cli, "test", "$cmd=sample & datastreams=", 26, 3, 0, NULL, NULL);
            if (ret != MQTTCLIENT_SUCCESS) {
                QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
            } else {
                QL_MQTT_LOG("Response published: %s", response);
            }

            ret =  ql_mqtt_publish(&mqtt_cli, "test", (const char *)buffer, buffer_len, 3, 0, NULL, NULL);
            if (ret != MQTTCLIENT_SUCCESS) {
                QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
            } else {
                QL_MQTT_LOG("Response published: %s", response);
            }

            ret = ql_mqtt_publish(&mqtt_cli, "test", "msgid=", 6, 3, 0, NULL, NULL);
            if (ret != MQTTCLIENT_SUCCESS) {
                QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
            } else {
                QL_MQTT_LOG("Response published: %s", response);
            }

            ret = ql_mqtt_publish(&mqtt_cli, "test", msgid, strlen(msgid), 3, 0, NULL, NULL);
            if (ret != MQTTCLIENT_SUCCESS) {
                QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
            } else {
                QL_MQTT_LOG("Response published: %s", response);
            }
        }  
	}else if (strcmp(cmd, "getworkmode") == 0){                //获取工作模式
		snprintf(response, sizeof(response), "$cmd=getworkmode & mode=%d & msgid=%s", report_flag, msgid);
	}else if (strcmp(cmd, "getcmdversion") == 0){              //获取指令集版本
		snprintf(response, sizeof(response), "$cmd=getcmdversion & version=%f & msgid=%s", cmdversion, msgid);
	}

    ret = ql_mqtt_publish(&mqtt_cli, "test", response, strlen(response), 3, 0, NULL, NULL);
    if (ret != MQTTCLIENT_SUCCESS) {
        QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
    } else {
        QL_MQTT_LOG("Response published: %s", response);
    }

    free(payload_str);
	return 1;
}

int process_1parameter_instruction(const unsigned char *payload, unsigned short payload_len)
{
    int ret;
    if (payload_len == 0) {
        QL_MQTT_LOG("Error: Empty payload");
        return 0;
    }

    char *payload_str = (char *)malloc(payload_len + 1);
    if (!payload_str) {
        QL_MQTT_LOG("Memory allocation failed");
        return 0;
    }
    memcpy(payload_str, payload, payload_len);
    payload_str[payload_len] = '\0';
	
    char *cmd = NULL, *paramA = NULL, *apikey = NULL, *msgid = NULL;
    char *token = strtok(payload_str, "&");
    while (token != NULL) {
        if (strncmp(token, "$cmd=", 5) == 0) {
            cmd = token + 5;
        } else if (strncmp(token, "server=", 7) == 0) {
            paramA = token + 7;
        } else if (strncmp(token, "sensor_id=", 10) == 0) {
            paramA = token + 10;
        } else if (strncmp(token, "sensor_id=", 10) == 0) {
            paramA = token + 10;
        } else if (strncmp(token, "mode=", 5) == 0) {
            paramA = token + 5;
        } else if (strncmp(token, "apikey=", 7) == 0) {
            apikey = token + 7;
        } else if (strncmp(token, "msgid=", 6) == 0) {
            msgid = token + 6;
        }
        token = strtok(NULL, "&");
    }

    if (!cmd || !apikey || !msgid || !paramA) {
        QL_MQTT_LOG("Error: Missing required fields (cmd, apikey, msgid) or parameter problem");
        free(payload_str);
        return 0;
    }

    if (strcmp(apikey, "VALID_KEY") != 0) {
        QL_MQTT_LOG("Error: Invalid apikey '%s'", apikey);
        free(payload_str);
        return 0;
    }
	
    char response[256];   //**********************************************************************
	if(strncmp(cmd, "settime", 7) == 0){                            //校准时间操作
		snprintf(response, sizeof(response), "$cmd=settime & result=succ & msgid=%s", msgid);
	} else if (strncmp(cmd, "reqsensortime", 13) == 0){             //获取单个传感器采集时间间隔等
		snprintf(response, sizeof(response), "$cmd=reqsensortime & sensor_id=%s & sample_intv=%d & upload_intv=%d & plus_intv=value & msgid=%s", 
                                                sensor_time[0].sensor_id, sensor_time[0].sample_intv, sensor_time[0].upload_intv, msgid);
	} else if (strncmp(cmd, "getsensorattr", 13) == 0){             //获取传感器属性相关参数
		snprintf(response, sizeof(response), "$cmd=getsensorattr & sensor_id=%s & threshold=%f & upper_limit=%f & lower_limit=%f & msgid=%s", 
                                                sensor_attribute[0].sensor_id, sensor_attribute[0].threshold, 
                                                sensor_attribute[0].upper_limit, sensor_attribute[0].lower_limit, msgid);
	} else if (strncmp(cmd, "setworkmode", 11) == 0){               //设置传感器工作模式
        if(strncmp(paramA, "0", 1) == 0)
        {
            report_flag = 0;
        }
        else if(strncmp(paramA, "1", 1) == 0)
        {
            report_flag = 1;
        }
        else if(strncmp(paramA, "2", 1) == 0)
        {
            report_flag = 2;
        }

        save_to_nvram("report_flag", &report_flag, sizeof(report_flag), &last_report_flag);

		snprintf(response, sizeof(response), "$cmd=setworkmode&result=succ & msgid=%s", msgid);
		//snprintf(response, sizeof(response), "$cmd=setworkmode&result=fail & msgid=%s", msgid);
	} 
    
    ret = ql_mqtt_publish(&mqtt_cli, "test", response, strlen(response), 3, 0, NULL, NULL);
    if (ret != MQTTCLIENT_SUCCESS) {
        QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
    } else {
        QL_MQTT_LOG("Response published: %s", response);
    }

    free(payload_str);
	return 1;
}

int process_2parameter_instruction(const unsigned char *payload, unsigned short payload_len)
{
    int ret;
    if (payload_len == 0) {
        QL_MQTT_LOG("Error: Empty payload");
        return 0;
    }

    char *payload_str = (char *)malloc(payload_len + 1);
    if (!payload_str) {
        QL_MQTT_LOG("Memory allocation failed");
        return 0;
    }
    memcpy(payload_str, payload, payload_len);
    payload_str[payload_len] = '\0';
	
    char *cmd = NULL, *paramA = NULL, *paramB = NULL, *apikey = NULL, *msgid = NULL;
    char *token = strtok(payload_str, "&");
    while (token != NULL) {
        if (strncmp(token, "$cmd=", 5) == 0) {
            cmd = token + 5;
        } else if (strncmp(token, "md5=", 4) == 0) {
            paramA = token + 4;
        } else if (strncmp(token, "size=", 5) == 0) {
            paramB = token + 5;
        } else if (strncmp(token, "apikey=", 7) == 0) {
            apikey = token + 7;
        } else if (strncmp(token, "msgid=", 6) == 0) {
            msgid = token + 6;
        }
        token = strtok(NULL, "&");
    }

    if (!cmd || !apikey || !msgid || !paramA || !paramB) {
        QL_MQTT_LOG("Error: Missing required fields (cmd, apikey, msgid) or parameter problem");
        free(payload_str);
        return 0;
    }

    if (strcmp(apikey, "VALID_KEY") != 0) {
        QL_MQTT_LOG("Error: Invalid apikey '%s'", apikey);
        free(payload_str);
        return 0;
    }

    char response[256];                         //**********固件升级                      
    snprintf(response, sizeof(response), "$cmd=supportsize & range=0.05,100 & msgid=%s", msgid);

    ret = ql_mqtt_publish(&mqtt_cli, "test", response, strlen(response), 3, 0, NULL, NULL);
    if (ret != MQTTCLIENT_SUCCESS) {
        QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
    } else {
        QL_MQTT_LOG("Response published: %s", response);
    }

    free(payload_str);
	return 1;
}

int process_3parameter_instruction(const unsigned char *payload, unsigned short payload_len)
{
    int ret;
    if (payload_len == 0) {
        QL_MQTT_LOG("Error: Empty payload");
        return 0;
    }

    char *payload_str = (char *)malloc(payload_len + 1);
    if (!payload_str) {
        QL_MQTT_LOG("Memory allocation failed");
        return 0;
    }
    memcpy(payload_str, payload, payload_len);
    payload_str[payload_len] = '\0';
	
    char *cmd = NULL, *paramA = NULL, *paramB = NULL, *paramC = NULL, *apikey = NULL, *msgid = NULL;
    char *token = strtok(payload_str, "&");
    while (token != NULL) {
        if (strncmp(token, "$cmd=", 5) == 0) {
            cmd = token + 5;
        } else if (strncmp(token, "b_num=", 6) == 0) {
            paramA = token + 6;
        } else if (strncmp(token, "b_size=", 7) == 0) {
            paramB = token + 7;
        } else if (strncmp(token, "b_content=", 10) == 0) {
            paramC = token + 10;
        } else if (strncmp(token, "apikey=", 7) == 0) {
            apikey = token + 7;
        } else if (strncmp(token, "msgid=", 6) == 0) {
            msgid = token + 6;
        }
        token = strtok(NULL, "&");
    }

    if (!cmd || !apikey || !msgid || !paramA || !paramB || !paramC) {
        QL_MQTT_LOG("Error: Missing required fields (cmd, apikey, msgid) or parameter problem");
        free(payload_str);
        return 0;
    }

    if (strcmp(apikey, "VALID_KEY") != 0) {
        QL_MQTT_LOG("Error: Invalid apikey '%s'", apikey);
        free(payload_str);
        return 0;
    }

    char response[256];                         //文字转语音播报功能
    speaker_data.b_num = (int)paramA;
    speaker_data.b_value = (int)paramB;
    speaker_data.b_content = paramC;     
    snprintf(response, sizeof(response), "$cmd=supportsize & success & msgid=%s", msgid);

    ret = ql_mqtt_publish(&mqtt_cli, "test", response, strlen(response), 3, 0, NULL, NULL);
    if (ret != MQTTCLIENT_SUCCESS) {
        QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
    } else {
        QL_MQTT_LOG("Response published: %s", response);
    }

    free(payload_str);
	return 1;
}

int process_4parameter_instruction(const unsigned char *payload, unsigned short payload_len)
{
    int ret;
    if (payload_len == 0) {
        QL_MQTT_LOG("Error: Empty payload");
        return 0;
    }

    char *payload_str = (char *)malloc(payload_len + 1);
    if (!payload_str) {
        QL_MQTT_LOG("Memory allocation failed");
        return 0;
    }
    memcpy(payload_str, payload, payload_len);
    payload_str[payload_len] = '\0';
	
    char *cmd = NULL, *paramA = NULL, *paramB = NULL, *paramC = NULL, *paramD = NULL, *apikey = NULL, *msgid = NULL;
    char *token = strtok(payload_str, "&");
    while (token != NULL) {
        if (strncmp(token, "$cmd=", 5) == 0) {
            cmd = token + 5;
        } else if (strncmp(token, "sensor_id=", 10) == 0) {
            paramA = token + 10;
        } else if (strncmp(token, "sensor_id=", 10) == 0) {
            paramA = token + 10;
        } else if (strncmp(token, "level=", 6) == 0) {
            paramA = token + 6;
        } else if (strncmp(token, "sample_intv=", 12) == 0) {
            paramB = token + 12;
        } else if (strncmp(token, "threshold=", 10) == 0) {
            paramB = token + 10;
        } else if (strncmp(token, "effective_time=", 15) == 0) {
            paramB = token + 15;
        } else if (strncmp(token, "upload_intv=", 12) == 0) {
            paramC = token + 12;
        } else if (strncmp(token, "upper_limit=", 12) == 0) {
            paramC = token + 12;
        } else if (strncmp(token, "lon_range=", 10) == 0) {
            paramC = token + 10;
		} else if (strncmp(token, "plus_intv=", 10) == 0) {
            paramD = token + 10;
        } else if (strncmp(token, "lower_limit=", 12) == 0) {
            paramD = token + 12;
        } else if (strncmp(token, "lat_range=", 10) == 0) {
            paramD = token + 10;
        } else if (strncmp(token, "apikey=", 7) == 0) {
            apikey = token + 7;
        } else if (strncmp(token, "msgid=", 6) == 0) {
            msgid = token + 6;
        }
        token = strtok(NULL, "&");
    }

    if (!cmd || !apikey || !msgid || !paramA || !paramB || !paramC || !paramD) {
        QL_MQTT_LOG("Error: Missing required fields (cmd, apikey, msgid) or parameter problem");
        free(payload_str);
        return 0;
    }

    if (strcmp(apikey, "VALID_KEY") != 0) {
        QL_MQTT_LOG("Error: Invalid apikey '%s'", apikey);
        free(payload_str);
        return 0;
    }
	
    char response[256];   //**********************************************************************
	if(strncmp(cmd, "setsensortime", 13) == 0){                                      //设置传感器时间相关参数
        if(strncmp(sensor_time[0].sensor_id, paramA, 7) == 0)
        {
            sensor_time[0].sample_intv = (int)*paramB;
            sensor_time[0].upload_intv = (int)*paramC;
        }

        save_to_nvram("sensor_time_data", &sensor_time, sizeof(sensor), &last_sensor_time);

		snprintf(response, sizeof(response), "$cmd=setsensortime & result=succ & msgid=%s", msgid);
        //snprintf(response, sizeof(response), "$$cmd=setsensortime & result=fail & msgid=%s", msgid);
	} else if (strncmp(cmd, "setsensorattr", 13) == 0){
        if(strncmp(sensor_attribute[0].sensor_id, paramA, 7) == 0)
        {
            sensor_attribute[0].threshold = (float)*paramB;
            sensor_attribute[0].upper_limit = (float)*paramC;
            sensor_attribute[0].lower_limit = (float)*paramD;
        }

        save_to_nvram("sensor_attribute_data", &sensor_attribute, sizeof(sensor1), &last_sensor_attribute);

		snprintf(response, sizeof(response), "$cmd=setsensorattr & result=succ & msgid=%s", msgid);
        //snprintf(response, sizeof(response), "$$cmd=setsensortime & result=fail & msgid=%s", msgid);
	} else if (strncmp(cmd, "meteorologicalearlywarning", 26) == 0){
		snprintf(response, sizeof(response), "$cmd=meteorologicalearlywarning & result=succ & msgid=%s", msgid);
        snprintf(response, sizeof(response), "$cmd=meteorologicalearlywarning & result=succ & msgid=%s", msgid);
	} 

    ret = ql_mqtt_publish(&mqtt_cli, "test", response, strlen(response), 3, 0, NULL, NULL);
    if (ret != MQTTCLIENT_SUCCESS) {
        QL_MQTT_LOG("Failed to publish response, ret=%d", ret);
    } else {
        QL_MQTT_LOG("Response published: %s", response);
    }

    free(payload_str);
	return 1;
}


