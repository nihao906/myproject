#ifndef DATA_INIT_H
#define DATA_INIT_H

typedef struct{
    double crack_value;                                    //变形监测裂缝张开度

    char   Surface_displacement_gpsInitial[256];           //变形监测地表位移GNSS原始数据
    double Surface_displacement_gpsTotalX;                 //变形监测地表位移与初始位置的X方向位移
    double Surface_displacement_gpsTotalY;                 //变形监测地表位移与初始位置的Y方向位移
    double Surface_displacement_gpsTotalZ;                 //变形监测地表位移与初始位置的Z方向位移

    double deep_displacement_dispsX;                       //顺滑动方向随时间的累计变形量
    double deep_displacement_dispsY;                       //垂直坡面方向随时间的累计变形量

    double acceleration_gX;                                //传感器 X 轴方向加速度及瞬间冲击加速度
    double acceleration_gY;                                //传感器 Y 轴方向加速度及瞬间冲击加速度
    double acceleration_gZ;                                //传感器 Z 轴方向加速度及瞬间冲击加速度

    double dip_angle_X;                                    //传感器 X 轴方向倾斜角度
    double dip_angle_Y;                                    //传感器 Y 轴方向倾斜角度
    double dip_angle_Z;                                    //传感器 Z 轴方向倾斜角度
    double dip_angle_angle;                                //x、y 轴合成方位角上的倾斜角度大小
    double dip_angle_trend;                                //x、y 轴合成的方位角度（0°~360°）

    double vibrate_PLX;                                    //传感器 X 轴振动频率
    double vibrate_PLY;                                    //传感器 Y 轴振动频率
    double vibrate_PLZ;                                    //传感器 Z 轴振动频率
    double vibrate_value;                                  //振动幅度
    double vibrate_SJX;                                    //传感器初始位置为原点，X 轴瞬间位移
    double vibrate_SJY;                                    //传感器初始位置为原点，Y 轴瞬间位移
    double vibrate_SJZ;                                    //传感器初始位置为原点，Z 轴瞬间位移
    double vibrate_SJValue;                                //传感器初始位置为原点，合方向上瞬间位移
}Deformation_monitor;
extern Deformation_monitor Deformation_monitor_data[20];
extern Deformation_monitor last_Deformation_monitor_data[20];

typedef struct{
    double stress;                                         //岩土体内部或岩土体与防治工程之间的作用力

    double soil_pressure;                                  //土体作用在建筑物或构筑物上的力

    double infrasound_OSP;                                 //主要指由泥石流运动产生且在空气中传播的频率在20Hz以下的次声原始声压
    double infrasound_VSP;                                 //有效声压
    double infrasound_freq;                                //频率

    double Geosound_OSP;                                   //相对运动而产生的弹性波传播过程所形成的原始声压
    double Geosound_VSP;                                   //有效声压
    double Geosound_freq;                                  //频率
}Physical_field_monitor;
extern Physical_field_monitor Physical_field_monitor_data[20];
extern Physical_field_monitor last_Physical_field_monitor_data[20];

typedef struct{
    double rainfall_value;                                 //表示一次数据上报间隔内的降雨量
    double rainfall_totalValue;                            //当日雨量累积值

    double air_temp;                                       //在野外空气流通、不受太阳直射下的空气温度

    double soil_temp;                                      //地面以下所监测层位土壤中的温度

    double soil_water_content;                             //土壤中水分占有的体积和土壤总体积的比值

    double surface_water_temp;                             //陆地表面上的水的温度
    double surfave_water_level;                            //陆地表面上的水面相对于基准面的高程

    double Groundwater_temp;                               //所监测层位的地下水的温度
    double Groundwater_level;                              //所监测层位的稳定地下水面相对于基准面的高程

    double Pore_water_temp;                                //斜坡岩土体中地下水的温度
    double Pore_water_pressure;                            //斜坡岩土体中地下水的压力

    double osmotic_pressure;                               //渗流方向上水对单位体积土的压力

    double velocity_of_flow;                               //河道的水流速度

    double settlement;                                     //监测点的沉降量的测量值

    double air_pressure;                                   //监测点的气压测量值
}Influence_factor_monitor;
extern Influence_factor_monitor Influence_factor_monitor_data[20];
extern Influence_factor_monitor last_Influence_factor_monitor_data[20];

typedef struct{
    double Mud_water_level;                                //泥石流发生时沟道内泥水面相对于基准面的高程

    double radar_X;                                        //X：以雷达为原点，监测物体在 X 轴方向坐标
    double radar_Y;                                        //Y：以雷达为原点，监测物体在 Y 轴方向坐标
    double radar_Z;                                        //Z：以雷达为原点，监测物体在 Z 轴方向坐标
    double radar_speed;                                    //V：监测物体移动速度
}Macro_phenomenon_monitor;
extern Macro_phenomenon_monitor Macro_phenomenon_monitor_data[20];
extern Macro_phenomenon_monitor last_Macro_phenomenon_monitor_data[20];

typedef struct{
    char *sensor_id;                                        //传感器id
    int  sample_intv;                                       //传感器采集间隔
    int  upload_intv;                                       //传感器上传间隔
}sensor;
extern sensor sensor_time[20];
extern sensor last_sensor_time[20];

typedef struct{
    char  *sensor_id;                                       //传感器id
    float threshold;                                        //传感器阈值
    float upper_limit;                                      //传感器上限值
    float lower_limit;                                      //传感器下限值
}sensor1;
extern sensor1 sensor_attribute[20];
extern sensor1 last_sensor_attribute[20];

typedef struct{
    int  b_num;                                              //文字播报遍数
    int  b_value;                                            //具体文字内容长度
    char *b_content;                                         //具体文字内容
}speaker;
extern speaker speaker_data;
extern speaker last_speaker_data;

extern int report_flag;
extern int last_report_flag;

extern char *Time;
extern char *sensor_type[2];
extern float cmdversion;

#endif