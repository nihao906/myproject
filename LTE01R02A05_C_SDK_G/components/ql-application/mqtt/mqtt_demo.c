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

#define QL_MQTT_LOG(msg, ...)			QL_LOG(QL_MQTT_LOG_LEVEL, "ql_MQTT", msg, ##__VA_ARGS__)
#define QL_MQTT_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_MQTT", msg, ##__VA_ARGS__)
static ql_task_t mqtt_task = NULL;

#define MQTT_CLIENT_IDENTITY        "quectel_01"
#define MQTT_CLIENT_USER            ""
#define MQTT_CLIENT_PASS            ""

//#define MQTT_CLIENT_QUECTEL_URL                  "mqtt://220.180.239.212:8306"

//#define MQTT_CLIENT_QUECTEL_SSL_URL              "mqtts://220.180.239.212:8307"

static ql_sem_t  mqtt_semp;
static int  mqtt_connected = 0;
//static mqtt_client_t  mqtt_cli;

typedef struct{
    char time[20];
    char did[20];
    char type_code[20];
    char device_status_now[20];   //设备状态参数，其中各种类型都有，只能用char;
    char sensor_status_now[20];
    double voltage;
    double temp;
    double humidity;
}JSON;

JSON JSON_data[20];

memset(JSON_data, 0, sizeof(JSON_data));

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

static void mqtt_requst_result_cb(mqtt_client_t *client, void *arg,int err)
{
	QL_MQTT_LOG("err: %d", err);
	
	ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_inpub_data_cb(mqtt_client_t *client, void *arg, int pkt_id, const char *topic, const unsigned char *payload, unsigned short payload_len);
int construct_data_buffer_cjson(const char *file_data, unsigned short file_len, unsigned char **buffer_out, unsigned short *buffer_len_out ,int status_or_sample);
int process_0parameter_instruction(const unsigned char *payload, unsigned short payload_len);
int process_1parameter_instruction(const unsigned char *payload, unsigned short payload_len);
int process_2parameter_instruction(const unsigned char *payload, unsigned short payload_len);
int process_3parameter_instruction(const unsigned char *payload, unsigned short payload_len);
int process_4parameter_instruction(const unsigned char *payload, unsigned short payload_len);

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

	
    int ret;

    if(QL_DATACALL_SUCCESS != ql_bind_sim_and_profile(nSim, profile_idx, &sim_cid))
    {
        QL_MQTT_LOG("nSim or profile_idx is invalid!!!!");
        break;
    }
    
    if(ql_mqtt_client_init(&mqtt_cli, sim_cid) != MQTTCLIENT_SUCCESS){
        QL_MQTT_LOG("mqtt client init failed!!!!");
        break;
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
    
    /*
    if(case_id == 0)
    {
    client_info.ssl_cfg = NULL;
    ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_QUECTEL_URL , mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);	
    }else
    {
        struct mqtt_ssl_config_t quectel_ssl_cfg = 
        {
            .ssl_ctx_id = 1,
#if USE_CRT_BUFFER
            .verify_level = MQTT_SSL_VERIFY_SERVER_CLIENT,
            .client_cert_type= QL_SSL_CLIENT_CERT_BUFFER,
            .cacert_buffer = root_ca_crt_buffer,
            .client_cert_path = client_crt_buffer,
            .client_key_path = client_key_buffer,
#else
            .verify_level = MQTT_SSL_VERIFY_NONE,
            .cacert_path = NULL,
            .client_cert_path = NULL,
            .client_key_path = NULL,
#endif			
            .client_key_pwd = NULL,
            .ssl_version = QL_SSL_VERSION_ALL,
            .sni_enable = 0,
            .ssl_negotiate_timeout = QL_SSL_NEGOTIATE_TIME_DEF,
            .ignore_invalid_certsign = 0,
            .ignore_multi_certchain_verify = 0,
            .ignore_certitem = MBEDTLS_X509_BADCERT_NOT_TRUSTED|MBEDTLS_X509_BADCERT_EXPIRED|MBEDTLS_X509_BADCERT_FUTURE,
        }; 
        
        client_info.ssl_cfg = &quectel_ssl_cfg;
        ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_QUECTEL_SSL_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
    }*/

    client_info.ssl_cfg = NULL;
    ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_QUECTEL_URL , mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);	

    if(ret  == MQTTCLIENT_WOUNDBLOCK){
        QL_MQTT_LOG("====wait connect result");
        ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
        if(mqtt_connected == 0){
            ql_mqtt_client_deinit(&mqtt_cli);
            break;
        }
    }else{
        QL_MQTT_LOG("===mqtt connect failed ,ret = %d",ret);
        break;
    }

    ql_mqtt_set_inpub_callback(&mqtt_cli, mqtt_inpub_data_cb, NULL);
    
    while(mqtt_connected == 1)
    {
        //注册就是订阅，订阅无独立函数
        if(ql_mqtt_sub_unsub(&mqtt_cli, "test", 1, mqtt_requst_result_cb,NULL, 1) == MQTTCLIENT_WOUNDBLOCK){
            QL_MQTT_LOG("======wait subscrible result");
            ql_rtos_semaphore_wait(mqtt_semp, 10000);   //只等待10s，避免锁死
        }

        if(ql_mqtt_publish(&mqtt_cli, "test", "hi, mqtt qos 0", strlen("hi, mqtt qos 0"), 0, 0, mqtt_requst_result_cb,NULL) == MQTTCLIENT_WOUNDBLOCK){
            QL_MQTT_LOG("======wait publish result");
            ql_rtos_semaphore_wait(mqtt_semp, 10000);   //只等待10s，避免锁死
        }
        
        ql_rtos_task_sleep_ms(500);
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

    if(process_0parameter_instruction(&payload, payload_len) == 1){
        return;
    }else if(process_1parameter_instruction(&payload, payload_len) == 1){
        return;
    }else if(process_2parameter_instruction(&payload, payload_len) == 1){
        return;
    }else if(process_3parameter_instruction(&payload, payload_len) == 1){
        return;
    }else if(process_4parameter_instruction(&payload, payload_len) == 1){
        return;
    }

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
	
    free(json_str);
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

    char response[256];   //**********************************************************************                      
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
	if(strcmp(cmd, "reqtime") == 0){
		snprintf(response, sizeof(response), "$cmd=reqtime & time=2019-05-01 13:00:00 & msgid=%s", msgid);
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
	}else if (strcmp(cmd, "reboot") == 0){	   //*****************************************************************
		snprintf(response, sizeof(response), "$cmd= reboot & result=succ & msgid=%s", msgid);
		snprintf(response, sizeof(response), "$cmd= reboot & result=fail & msgid=%s", msgid);
	}else if (strcmp(cmd, "getsensorID") == 0){  //*****************************************************************
		snprintf(response, sizeof(response), "$cmd=getsensorID & sensor_id=value & msgid=%s", msgid);    
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
	}else if (strcmp(cmd, "getworkmode") == 0){  //*****************************************************************
		snprintf(response, sizeof(response), "$cmd=getworkmode & mode=value & msgid=%s", msgid);
		snprintf(response, sizeof(response), "$cmd=getworkmode & mode=value & msgid=%s", msgid);
	}else if (strcmp(cmd, "getcmdversion") == 0){  //*****************************************************************
		snprintf(response, sizeof(response), "$cmd=getcmdversion & version=1.1 & msgid=%s", msgid);
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
	if(strncmp(cmd, "settime") == 0){
		snprintf(response, sizeof(response), "$cmd=settime & result=succ & msgid=%s", msgid);
	} else if (strncmp(cmd, "reqsensortime") == 0){
		snprintf(response, sizeof(response), "$cmd=reqsensortime & sensor_id=value & sample_intv=value & upload_intv=value & plus_intv=value & msgid=%s", msgid);
	} else if (strncmp(cmd, "getsensorattr") == 0){
		snprintf(response, sizeof(response), "$cmd=getsensorattr & sensor_id=value & threshold=value & upper_limit=value & lower_limit=value & msgid=%s", msgid);
	} else if (strncmp(cmd, "setworkmode") == 0){
		snprintf(response, sizeof(response), "$cmd=setworkmode&result=succ & msgid=%s", msgid);
		snprintf(response, sizeof(response), "$cmd=setworkmode&result=fail & msgid=%s", msgid);
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

    char response[256];   //**********************************************************************                      
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

    if (!cmd || !apikey || !msgid || !paramA || !paramC || !paramD) {
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
	if(strncmp(cmd, "setsensortime") == 0){
		snprintf(response, sizeof(response), "$cmd=setsensortime & result=succ & msgid=%s", msgid);
        snprintf(response, sizeof(response), "$$cmd=setsensortime & result=fail & msgid=%s", msgid);
	} else if (strncmp(cmd, "setsensorattr") == 0){
		snprintf(response, sizeof(response), "$cmd=setsensorattr & result=succ & msgid=%s", msgid);
        snprintf(response, sizeof(response), "$$cmd=setsensortime & result=fail & msgid=%s", msgid);
	} else if (strncmp(cmd, "meteorologicalearlywarning") == 0){
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

int construct_data_buffer_cjson(const char *file_data, unsigned short file_len, unsigned char **buffer_out, unsigned short *buffer_len_out ,int status_or_sample);
{
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        QL_MQTT_LOG("Failed to create cJSON object");
        return -1;
    }
    //$cmd=getstatus&state={"ext_power_volt":24.04,"temp":42.00,"signal_4g":27.0,"sw_version":"1.0.1","4g_on":true}
    //$cmd=sample&datastreams={"L2_LF_1":"34.56","L2_LF_2":"67.45","L2_LF_3":"12.2"}
    if(status_or_sample == 1)
    {
        cJSON_AddNumberToObject(root, "ext_power_volt", JSON_data[0].voltage);
        cJSON_AddNumberToObject(root, "temp", JSON_data[0].temp);
    }else if(status_or_sample == 2)
    {
        cJSON_AddStringToObject(root, "L2_LF_1", JSON_data[0].type_code);
        cJSON_AddStringToObject(root, "L2_LF_2", JSON_data[0].type_code);
    }

    char *json_str = cJSON_PrintUnformatted(root);
    if (!json_str) {
        QL_MQTT_LOG("Failed to print JSON");
        cJSON_Delete(root);
        return -1;
    }
    unsigned short json_len = strlen(json_str);

    unsigned char data_type = (file_len > 0) ? 0x05 : 0x03;
    unsigned short total_len = 1 + 2 + json_len + 2 + file_len;
    
    //规定了字节结构，需要自己构造字节
    unsigned char *buffer = (unsigned char *)malloc(total_len);
    if (!buffer) {
        QL_MQTT_LOG("Memory allocation failed");
        cJSON_Free(json_str);
        cJSON_Delete(root);
        return -1;
    }

    int offset = 0;
    buffer[offset++] = data_type;

    buffer[offset++] = (json_len >> 8) & 0xFF;       
    buffer[offset++] = json_len & 0xFF;
    memcpy(buffer + offset, json_str, json_len);     
    offset += json_len;

    buffer[offset++] = (file_len >> 8) & 0xFF;       
    buffer[offset++] = file_len & 0xFF;              
    if (file_len > 0) {
        memcpy(buffer + offset, file_data, file_len);
    }

    *buffer_out = buffer;
    *buffer_len_out = total_len;

    cJSON_Free(json_str);
    cJSON_Delete(root);

    return 0;
}



