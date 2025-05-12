#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ql_fs.h>
#include "osi_api.h"
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
#include "SaveData.h"

#define QL_MQTT_LOG_LEVEL QL_LOG_LEVEL_INFO
#define QL_MQTT_LOG(msg, ...)			QL_LOG(QL_MQTT_LOG_LEVEL, "ql_MQTT", msg, ##__VA_ARGS__)
#define QL_MQTT_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_MQTT", msg, ##__VA_ARGS__)
static ql_task_t mqtt_task = NULL;

#define MQTT_CLIENT_IDENTITY        "quectel_01"
#define MQTT_CLIENT_USER            ""
#define MQTT_CLIENT_PASS            ""

#define MQTT_CLIENT_QUECTEL_URL                  "192.168.57.1"

#define MQTT_CLIENT_QUECTEL_SSL_URL              "mqtts://220.180.239.212:8307"

static ql_sem_t  mqtt_semp;
static int  mqtt_connected = 0;
mqtt_client_t  mqtt_cli;

//osiTimer_t *nvram_timer;

static void mqtt_state_exception_cb(mqtt_client_t *client)
{
	QL_MQTT_LOG("mqtt session abnormal disconnect");
	mqtt_connected = 0;
}

static void mqtt_connect_result_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_e status)
{
	QL_MQTT_LOG("status: %d", status);
	if(status == 0){
		mqtt_connected = 1;
	}
	ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_inpub_data_cb(mqtt_client_t *client, void *arg, int pkt_id, const char *topic, const unsigned char *payload, unsigned short payload_len);

static void mqtt_disconnect_result_cb(mqtt_client_t *client, void *arg,int err){
	QL_MQTT_LOG("err: %d", err);
	
	ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_app_thread(void * arg)
{
	int ret = 0;
	int i = 0;
	int profile_idx = 1;
    ql_data_call_info_s info;
	char ip4_addr_str[16] = {0};
	uint8_t nSim = 0;
	uint16_t sim_cid;
    struct mqtt_connect_client_info_t  client_info = {0};
	ql_rtos_semaphore_create(&mqtt_semp, 0);
	ql_rtos_task_sleep_s(10);
    
	QL_MQTT_LOG("========== mqtt demo start ==========");
	QL_MQTT_LOG("wait for network register done");
		
	while((ret = ql_network_register_wait(nSim, 120)) != 0 && i < 10){
    	i++;
		ql_rtos_task_sleep_s(1);
	}
	if(ret == 0){
		i = 0;
		QL_MQTT_LOG("====network registered!!!!====");
	}else{
		QL_MQTT_LOG("====network register failure!!!!!====");
        goto exit;
	}

	ql_set_data_call_asyn_mode(nSim, profile_idx, 0);    //利用蜂窝网络，开始与网络建立连接

	QL_MQTT_LOG("===start data call====");
	ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, "uninet", NULL, NULL, 0); 
	QL_MQTT_LOG("===data call result:%d", ret);
	if(ret != 0){
		QL_MQTT_LOG("====data call failure!!!!=====");
        goto exit;
	}
	memset(&info, 0x00, sizeof(ql_data_call_info_s));
	
	ret = ql_get_data_call_info(nSim, profile_idx, &info);
	if(ret != 0){
		QL_MQTT_LOG("ql_get_data_call_info ret: %d", ret);
		ql_stop_data_call(nSim, profile_idx);
	}
    QL_MQTT_LOG("info->profile_idx: %d", info.profile_idx);
	QL_MQTT_LOG("info->ip_version: %d", info.ip_version);
            
	QL_MQTT_LOG("info->v4.state: %d", info.v4.state); 
	inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
	QL_MQTT_LOG("info.v4.addr.ip: %s\r\n", ip4_addr_str);

	inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
	QL_MQTT_LOG("info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);

	inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
	QL_MQTT_LOG("info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);

    if(QL_DATACALL_SUCCESS != ql_bind_sim_and_profile(nSim, profile_idx, &sim_cid))
    {
        QL_MQTT_LOG("nSim or profile_idx is invalid!!!!");
        goto exit;
    }
    
    if(ql_mqtt_client_init(&mqtt_cli, sim_cid) != MQTTCLIENT_SUCCESS){
        QL_MQTT_LOG("mqtt client init failed!!!!");
        goto exit;
    }

    QL_MQTT_LOG("mqtt_cli:%d", mqtt_cli);

    client_info.keep_alive = 60;
    client_info.pkt_timeout = 5;
    client_info.retry_times = 3;
    client_info.clean_session = 1;
    client_info.will_qos = 0;
    client_info.will_retain = 0;
    client_info.will_topic = NULL;
    client_info.will_msg = NULL;
    client_info.client_id = MQTT_CLIENT_IDENTITY;
    client_info.client_user = MQTT_CLIENT_USER;
    client_info.client_pass = MQTT_CLIENT_PASS;

    client_info.ssl_cfg = NULL;
    ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_QUECTEL_URL , mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);	

    if(ret  == MQTTCLIENT_WOUNDBLOCK){
        QL_MQTT_LOG("====wait connect result");
        ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
        if(mqtt_connected == 0){
            ql_mqtt_client_deinit(&mqtt_cli);
            goto exit;
        }
    }else{
        QL_MQTT_LOG("===mqtt connect failed ,ret = %d",ret);
        goto exit;
    }

    ql_mqtt_set_inpub_callback(&mqtt_cli, mqtt_inpub_data_cb, NULL);

    //注册就是订阅，订阅无独立函数
    if(ql_mqtt_sub_unsub(&mqtt_cli, "test", 1, NULL,NULL, 1) != MQTTCLIENT_SUCCESS){
        QL_MQTT_LOG("conneted failed");
        goto exit;   
    }
    
    while(mqtt_connected == 1)
    {   
        if(report_flag == 0){
            ql_rtos_task_sleep_s(7200);
            char *json_str = construct_multilayer_json(1.0, 
                                                        "1", 2, 3, 4,
                                                        5, 6, 
                                                        7.0, 8.0, 9.0,
                                                        10.0, 11.0, 12.0, 13.0, 14.0,
                                                        15.0, 16.0, 17, 18, 19, 20, 21, 22,
                                                        23,
                                                        24,
                                                        25, 26, 27,
                                                        28, 29, 30,
                                                        31, 32,
                                                        33, 34, 35, 36,
                                                        37, 38, 39, 40,
                                                        41, 42, 43, 44,
                                                        45,
                                                        46, 47, 48, 49, 50,
                                                        "51");
            if (json_str) {
            ql_mqtt_publish(&mqtt_cli, "test", json_str, strlen(json_str), 2, 0, NULL, NULL);
            free(json_str);
            }
        }
        else if(report_flag == 2){
            char *json_str = construct_multilayer_json(1.0, 
                                                        "1", 2, 3, 4,
                                                        5, 6, 
                                                        7.0, 8.0, 9.0,
                                                        10.0, 11.0, 12.0, 13.0, 14.0,
                                                        15.0, 16.0, 17, 18, 19, 20, 21, 22,
                                                        23,
                                                        24,
                                                        25, 26, 27,
                                                        28, 29, 30,
                                                        31, 32,
                                                        33, 34, 35, 36,
                                                        37, 38, 39, 40,
                                                        41, 42, 43, 44,
                                                        45,
                                                        46, 47, 48, 49, 50,
                                                        "51");
            if (json_str) {
            ql_mqtt_publish(&mqtt_cli, "test", json_str, strlen(json_str), 2, 0, NULL, NULL);
            free(json_str);
            }
            report_flag = 0;
        }
        else{
            ql_rtos_task_sleep_ms(1000);
        }
    }
    
    if(mqtt_connected == 1 && ql_mqtt_disconnect(&mqtt_cli, mqtt_disconnect_result_cb, NULL) == MQTTCLIENT_WOUNDBLOCK){
        QL_MQTT_LOG("=====wait disconnect result");
        ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
    }
    ql_mqtt_client_deinit(&mqtt_cli);

    exit:
        ql_rtos_semaphore_delete(mqtt_semp);
        ql_rtos_task_delete(mqtt_task);	

    return;	
}

int ql_mqtt_app_init(void)
{
    load_from_nvram("sensor_time_data", &sensor_time, sizeof(sensor), &last_sensor_time);
    load_from_nvram("sensor_attribute_data", &sensor_attribute, sizeof(sensor1), &last_sensor_attribute);
    load_from_nvram("report_flag", &report_flag, sizeof(report_flag), &last_report_flag);

	QlOSStatus err = QL_OSI_SUCCESS;
	
    err = ql_rtos_task_create(&mqtt_task, 16*1024, APP_PRIORITY_ABOVE_NORMAL, "QmqttApp", mqtt_app_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		QL_MQTT_LOG("mqtt_app init failed");
	}

	return err;
}

static void mqtt_inpub_data_cb(mqtt_client_t *client, void *arg, int pkt_id, const char *topic, const unsigned char *payload, unsigned short payload_len)
{
    QL_MQTT_LOG("Received - Topic: %s", topic);
    QL_MQTT_LOG("Payload: %s (Length: %d)", payload, payload_len);

    if(process_0parameter_instruction(payload, payload_len) == 1){
        return;
    }else if(process_1parameter_instruction(payload, payload_len) == 1){
        return;
    }else if(process_2parameter_instruction(payload, payload_len) == 1){
        return;
    }else if(process_3parameter_instruction(payload, payload_len) == 1){
        return;
    }else if(process_4parameter_instruction(payload, payload_len) == 1){
        return;
    }
    /*
    if (payload_len < 3) {
        QL_MQTT_LOG("Error: Payload too short");
        return;
    }

    unsigned char data_type = payload[0];
    QL_MQTT_LOG("Data type: 0x%02x", data_type);

    unsigned short data_length = (payload[1] << 8) | payload[2];
    QL_MQTT_LOG("Data length: %d", data_length);

	if ( (payload_len != data_length + 3) &&(data_type == 0x03 || data_type == 0x04) ) {
		QL_MQTT_LOG("Error: Payload length mismatch (expected %d, got %d)", data_length + 3, payload_len);
		return;
	}

	const unsigned char *data = payload + 3;

	//cJson_parse读取到'\0'结束，但MQTT的payload是原始数据，不保证是否会有'\0'。直接传payload有可能会越界访问
    char *json_str = (char *)malloc(payload_len + 1);
    if (!json_str) {
        QL_MQTT_LOG("Memory allocation failed");
        return;
    }
    memcpy(json_str, data, data_length);
    json_str[payload_len] = '\0';

	if(data_type == 0x03 || data_type == 0x04)
	{
		cJSON *root = cJSON_Parse(json_str);
		if (root)
		{
			char *json_treated_str = cJSON_Print(root);
			if (json_treated_str) 
			{
				printf("JSON: %s\n", json_treated_str);
				free(json_treated_str);  
			}
			else{
				printf("Error:Failed to print JSON");
			}
			cJSON_Delete(root);
		}
		else 
		{
			QL_MQTT_LOG("Invalid JSON: %s", cJSON_GetErrorPtr());
		}
	}
	else if(data_type == 0x05)
	{
		int offset = 3 + data_length;
		unsigned short file_length = (payload[offset] << 8) | payload[offset + 1];
		QL_MQTT_LOG("File data length: %d", file_length);

		if (payload_len != offset + 2 + file_length) {
			QL_MQTT_LOG("Error: Length mismatch");
			return;
		}

		const unsigned char *file_data = payload + offset + 2;

		FILE *f = fopen("file.txt", "wb");  
		if (f == NULL) {
			QL_MQTT_LOG("Error: Failed to open file for writing");
			return;
		}
		
		size_t written = fwrite(file_data, 1, file_length, f);
		if (written != file_length) {
			QL_MQTT_LOG("Error: Failed to write complete file, wrote %d,ecpect write %d bytes", written, file_length);
		} else {
			QL_MQTT_LOG("File saved successfully, %d bytes", file_length);
		}
		fclose(f);
	}
	
    free(json_str);*/
}


