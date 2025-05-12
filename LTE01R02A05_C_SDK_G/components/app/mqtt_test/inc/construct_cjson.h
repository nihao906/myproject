#ifndef CONSTRUCT_CJSON_H
#define CONSTRUCT_CJSON_H

int construct_data_buffer_cjson(const char *file_data, unsigned short file_len, unsigned char **buffer_out, unsigned short *buffer_len_out ,int status_or_sample);

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
    const char *time);

#endif
