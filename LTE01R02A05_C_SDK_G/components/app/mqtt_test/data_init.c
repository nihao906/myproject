#include "data_init.h"

Deformation_monitor Deformation_monitor_data[20] = {0};
Deformation_monitor last_Deformation_monitor_data[20] = {0};

Physical_field_monitor Physical_field_monitor_data[20] = {0};
Physical_field_monitor last_Physical_field_monitor_data[20] = {0};

Influence_factor_monitor Influence_factor_monitor_data[20] = {0};
Influence_factor_monitor last_Influence_factor_monitor_data[20] = {0};

Macro_phenomenon_monitor Macro_phenomenon_monitor_data[20] = {0};
Macro_phenomenon_monitor last_Macro_phenomenon_monitor_data[20] = {0};

sensor sensor_time[20] = {{"L2_LF_1", 20, 20}};
sensor last_sensor_time[20] = {0};

sensor1 sensor_attribute[20] = {{"L2_LF_1, 2, 2, 2"}};
sensor1 last_sensor_attribute[20] = {0};

speaker speaker_data = {2, 8, "播报文字内容"};
speaker last_speaker_data = {0};

char *Time;
char *sensor_type[2] = {"L1_LF_1", "L2_CS_2"};
float cmdversion = 1.1;

int report_flag = 0;
int last_report_flag = 0;

