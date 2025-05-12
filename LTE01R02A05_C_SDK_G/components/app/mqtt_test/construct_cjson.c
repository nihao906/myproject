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

#define QL_MQTT_LOG_LEVEL QL_LOG_LEVEL_INFO
#define QL_MQTT_LOG(msg, ...)			QL_LOG(QL_MQTT_LOG_LEVEL, "ql_MQTT", msg, ##__VA_ARGS__)
#define QL_MQTT_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_MQTT", msg, ##__VA_ARGS__)

int construct_data_buffer_cjson(const char *file_data, unsigned short file_len, unsigned char **buffer_out, unsigned short *buffer_len_out ,int status_or_sample)
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
        cJSON_AddNumberToObject(root, "ext_power_volt", 1);
        cJSON_AddNumberToObject(root, "temp", 2);
    }else if(status_or_sample == 2)
    {
        cJSON_AddStringToObject(root, "L2_LF_1", "1");
        cJSON_AddStringToObject(root, "L2_LF_2", "2");
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
        cJSON_Delete(json_str);
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

    cJSON_Delete(json_str);
    cJSON_Delete(root);

    return 0;
}


char *construct_multilayer_json(double crack_value, 
                                const char *gpsInitial, double gpsTotalX, double gpsTotalY, double gpsTotalZ,
                                double dispsX, double dispsY, 
                                double gX, double gY, double gZ,
                                double X, double Y, double Z, double angle, double trend,
                                double PLX, double PLY, double PLZ, double value, double SJX, double SJY, double SJZ, double SJValue,
                                double Physical_value,
                                double Physical_soil_value,
                                double OSP, double VSP, double freq,
                                double soil_OSP, double soil_VSP, double soil_freq,
                                double rainfall_value, double rainfall_totalvalue,
                                double air_temp, double soil_temp, double soil_water_content, double surface_water_temp,
                                double surface_water_level, double Groundwater_temp, double Groundwater_level, double Pore_water_temp,
                                double Pore_water_pressure, double osmotic_pressure, double velocity_of_flow, double settlement,
                                double air_pressure,
                                double Mud_water_level, double radar_X, double radar_Y, double radar_Z, double radar_speed,
                                const char *time)
{
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        QL_MQTT_LOG("Failed to create JSON root");
        return NULL;
    }

    //变形监测

    cJSON_AddNumberToObject(root, "L1_LF_1", crack_value);
    /*********************************
    **********************************/
    cJSON *status = cJSON_CreateObject();
    if (!status) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create status object");
        return NULL;
    }
    cJSON_AddStringToObject(status, "gpsInitial", gpsInitial);
    cJSON_AddNumberToObject(status, "gpsTotalX", gpsTotalX);
    cJSON_AddNumberToObject(status, "gpsTotalY", gpsTotalY);
    cJSON_AddNumberToObject(status, "gpsTotalZ", gpsTotalZ);
    cJSON_AddItemToObject(root, "L1_GP_1", status);
    /*********************************
    **********************************/
    cJSON *status1 = cJSON_CreateObject();
    if (!status1) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create status1 object");
        return NULL;
    }
    cJSON_AddNumberToObject(status1, "dispsX", dispsX);
    cJSON_AddNumberToObject(status1, "dispsY", dispsY);
    cJSON_AddItemToObject(root, "L1_SW_1", status1);
    /*********************************
    **********************************/
    cJSON *status2 = cJSON_CreateObject();
    if (!status2) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create status2 object");
        return NULL;
    }
    cJSON_AddNumberToObject(status2, "gX", gX);
    cJSON_AddNumberToObject(status2, "gY", gY);
    cJSON_AddNumberToObject(status2, "gZ", gZ);
    cJSON_AddItemToObject(root, "L1_JS_1", status2);
    /*********************************
    **********************************/
    cJSON *status3 = cJSON_CreateObject();
    if (!status3) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create status3 object");
        return NULL;
    }
    cJSON_AddNumberToObject(status3, "X", X);
    cJSON_AddNumberToObject(status3, "Y", Y);
    cJSON_AddNumberToObject(status3, "Z", Z);
    cJSON_AddNumberToObject(status3, "angle", angle);
    cJSON_AddNumberToObject(status3, "trend", trend);
    cJSON_AddItemToObject(root, "L1_QJ_1", status3);
    /*********************************
    **********************************/
    cJSON *status4 = cJSON_CreateObject();
    if (!status4) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create status4 object");
        return NULL;
    }
    cJSON_AddNumberToObject(status4, "PLX", PLX);
    cJSON_AddNumberToObject(status4, "PLY", PLY);
    cJSON_AddNumberToObject(status4, "PLZ", PLZ);
    cJSON_AddNumberToObject(status4, "value", value);
    cJSON_AddNumberToObject(status4, "SJX", SJX);
    cJSON_AddNumberToObject(status4, "SJY", SJY);
    cJSON_AddNumberToObject(status4, "SJZ", SJZ);
    cJSON_AddNumberToObject(status4, "SJValue", SJValue);
    cJSON_AddItemToObject(root, "L1_ZD_1", status4);

    //物理场监测

    cJSON_AddNumberToObject(root, "L2_YL_1", Physical_value);
    /*********************************
    **********************************/
    cJSON_AddNumberToObject(root, "L2_TY_1", Physical_soil_value);
    /*********************************
    **********************************/
    cJSON *Physical_status = cJSON_CreateObject();
    if (!Physical_status) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create Physical_status object");
        return NULL;
    }
    cJSON_AddNumberToObject(Physical_status, "OSP", OSP);
    cJSON_AddNumberToObject(Physical_status, "VSP", VSP);
    cJSON_AddNumberToObject(Physical_status, "freq", freq);
    cJSON_AddItemToObject(root, "L2_CS_1", Physical_status);
    /*********************************
    **********************************/
    cJSON *Physical_status1 = cJSON_CreateObject();
    if (!Physical_status1) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create Physical_status1 object");
        return NULL;
    }
    cJSON_AddNumberToObject(Physical_status1, "soil_OSP", soil_OSP);
    cJSON_AddNumberToObject(Physical_status1, "soil_VSP", soil_VSP);
    cJSON_AddNumberToObject(Physical_status1, "soil_freq", soil_freq);
    cJSON_AddItemToObject(root, "L2_DS_1", Physical_status1);

    //影响因素监测

    cJSON *Influence_status = cJSON_CreateObject();
    if (!Influence_status) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create Influence_status object");
        return NULL;
    }
    cJSON_AddNumberToObject(Influence_status, "rainfall_value", rainfall_value);
    cJSON_AddNumberToObject(Influence_status, "rainfall_totalvalue", rainfall_totalvalue);
    cJSON_AddItemToObject(root, "L3_YL_1", Influence_status);
    /*********************************
    **********************************/
    cJSON_AddNumberToObject(root, "L3_QW_1", air_temp);
    cJSON_AddNumberToObject(root, "L3_TW_1", soil_temp);
    cJSON_AddNumberToObject(root, "L3_HS_1", soil_water_content);
    /*********************************
    **********************************/
    cJSON *Influence_status1 = cJSON_CreateObject();
    if (!Influence_status1) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create Influence_status1 object");
        return NULL;
    }
    cJSON_AddNumberToObject(Influence_status1, "surface_water_temp", surface_water_temp);
    cJSON_AddNumberToObject(Influence_status1, "surface_water_level", surface_water_level);
    cJSON_AddItemToObject(root, "L3_DB_1", Influence_status1);
    /*********************************
    **********************************/
    cJSON *Influence_status2 = cJSON_CreateObject();
    if (!Influence_status2) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create Influence_status2 object");
        return NULL;
    }
    cJSON_AddNumberToObject(Influence_status2, "Groundwater_temp", Groundwater_temp);
    cJSON_AddNumberToObject(Influence_status2, "Groundwater_level", Groundwater_level);
    cJSON_AddItemToObject(root, "L3_DX_1", Influence_status2);
    /*********************************
    **********************************/
    cJSON *Influence_status3 = cJSON_CreateObject();
    if (!Influence_status3) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create Influence_status3 object");
        return NULL;
    }
    cJSON_AddNumberToObject(Influence_status3, "Pore_water_temp", Pore_water_temp);
    cJSON_AddNumberToObject(Influence_status3, "Pore_water_pressure", Pore_water_pressure);
    cJSON_AddItemToObject(root, "L3_SY_1", Influence_status3);
    /*********************************
    **********************************/
    cJSON_AddNumberToObject(root, "L3_ST_1", osmotic_pressure);
    cJSON_AddNumberToObject(root, "L3_LS_1", velocity_of_flow);
    cJSON_AddNumberToObject(root, "L3_CJ_1", settlement);
    cJSON_AddNumberToObject(root, "L3_QY_1", air_pressure);

    //宏观现象监测

    cJSON_AddNumberToObject(root, "L3_NW_1", Mud_water_level);
    /*********************************
    **********************************/
    cJSON *Macro_status = cJSON_CreateObject();
    if (!Macro_status) {
        cJSON_Delete(root);
        QL_MQTT_LOG("Failed to create Macro_status object");
        return NULL;
    }
    cJSON_AddNumberToObject(Macro_status, "radar_X", radar_X);
    cJSON_AddNumberToObject(Macro_status, "radar_Y", radar_Y);
    cJSON_AddNumberToObject(Macro_status, "radar_Z", radar_Z);
    cJSON_AddNumberToObject(Macro_status, "radar_speed", radar_speed);
    cJSON_AddItemToObject(root, "L3_LD_1", Macro_status);

    //时间

    cJSON_AddStringToObject(root, "at", time);

    char *json_str = cJSON_PrintUnformatted(root);
    if (!json_str) {
        QL_MQTT_LOG("Failed to print JSON");
    }

    cJSON_Delete(root);
    return json_str; 
}

