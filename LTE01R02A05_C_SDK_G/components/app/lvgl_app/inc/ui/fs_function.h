#include "lvgl.h"
#include "time.h"
#include "ql_fs.h"

#define     FILE_LETTER             'SD' //盘符
#define     CHAR_READ_LENGTH        80  //每次读取文件的长度

struct Device_Data
{
    int id;                 //设备桩点号
    int start_hour;
    int start_min;
    int start_second;
    int end_hour;
    int end_min;
    int end_second;
    float max_depth;        //设备所处的深度
    float max_V;            //设备当前喷浆量
    float average_I;          //设备在两次记录之间的平均电流
    int count;              //该id出现的次数
    int offset;             //该id第一次出现时的字节偏移量
};

struct Return_Data {
    int id_count;
    struct Device_Data dataArr[64]; // 假设数组长度为10
};

#define SAME_ID_MAX_NUM         20
struct ChartData{
	float depth[SAME_ID_MAX_NUM];
	float V[SAME_ID_MAX_NUM];
	float I[SAME_ID_MAX_NUM];
	char time[SAME_ID_MAX_NUM][10];
	int same_id_count;
};

int8_t findDataFile(uint16_t year,uint8_t month,uint8_t day);
void dealdata(char*buffer,int last_file,int last_i,struct Return_Data *newStruct);
void readDataFromFile(QFILE fd, struct Return_Data * data);
void setDataToTables(struct Return_Data* dataArray,int page,int page_count);
void setDataToChart(QFILE fd, int offset ,int current_id,struct ChartData* chart_data);