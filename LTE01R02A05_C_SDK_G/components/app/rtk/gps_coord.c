#include <math.h>
#include <stdint.h>
#include <string.h>
#include <ql_fs.h>
#include "cJSON.h"
#include "coordtrans.h"
#include "rtk_gps.h"
#include "ql_api_osi.h"
#include "osi_api.h"
#include "ql_log.h"
#include "gps_coord.h"


#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "rtk", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "rtk", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "rtk", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "rtk", msg, ##__VA_ARGS__)


extern uint16_t gWordVar[];

/**
 * @brief 计算参数
 *
 */
struct gps_coord_calc
{
    double angle_offset; // GPS1(左侧Gps)旋转角度
    double x_offset;     // x偏移距离m
    double y_offset;     // y偏移距离m
};

struct target_point_calc
{
    double inputdistance; 
    double distance; 
    double angle; 
};
#define REG_GPS_CROOD_CFG 500
// struct ConfigParam *gps_coord_cfg = (struct ConfigParam*)&gWordVar[REG_GPS_CROOD_CFG];

/**
struct ConfigParam cfp = {
  .dx = 0,  // x坐标平移量(米)
  .dy = 0,  // y坐标平移量(米)
  .dz = 0,  // z坐标平移量(米)
  .wx = 0,  // x坐标旋转角度(秒)
  .wy = 0,  // y坐标旋转角度(秒)
  .wz = 0,  // z坐标旋转角度(秒)
  .k = 0,  // 尺度因子(ppm)，这里其实应该用变量m表示
  .height = 0,  // 投影面高程
  .gcsSrc = WGS84,  // 原始坐标系
  .gcsDst = WGS84,  // 结果坐标系
};
! 大地坐标系类型
enum GEODETIC_COORD_SYSTEM { BJ54 = 0, XA80, WGS84, WGS2000 };
{
  "dx": 0,
  "dy": 0,
  "dz": 0,
  "wx": 0,
  "wy": 0,
  "wz": 0,
  "k": 0,
  "height": 0,
  "gcsSrc": "WGS84",
  "gcsDst": "WGS84",
}
*/
#define COORD_CFG_FILE_PATH "/coord_cfg.json"
static struct ConfigParam gps_coord_cfg; // 转换配置参数
/**
{
  "y_offset": 0, // 单位m
  "x_offset": 0, // 单位m
  "angle_offset": 0, // 角度 度
}
*/
#define CALC_CFG_FILE_PATH "/calc_cfg.json"
static struct gps_coord_calc gps_coord_calc_cfg; // 计算配置参数
static struct target_point_calc target_point_calc_cfg; // 目标点计算配置参数
static void get_dirname(char *filename, char *dirname)
{
    memcpy(dirname, filename, strlen(filename));
    char *p = strrchr(dirname, '/');
    if (p) *p = '\0';
    else memcpy(dirname, "SD:/", sizeof("SD:/"));   
}
#if 1
/**
 * @brief 保存json配置文件
 *
 * @param json
 * @param name
 */
static void save_json_cfg_file(cJSON *json, const char *name)
{
    if (name == NULL || json == NULL)
    {
        return;
    }
    int8_t *buff_p = NULL;
    int32_t buf_len = 0;
    int32_t fileFd = -1;
    int ret = 0;
    char dirname[50] = {0};

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
    // fileFd = LSAPI_FS_Open(name, LSAPI_FS_O_RDONLY, 0x0);
    if (fileFd < 0)
    {
        LOGE("LSAPI_FS_Open failed, file:%s\n", name);
        return;
    }
    buff_p = cJSON_Print(json);
    if (buff_p == NULL)
    {
        LOGE("cJSON_Print failed, buff_p is null\n");
        goto exit0;
    }
    buf_len = strlen(buff_p);
    ret = ql_fwrite((void *)buff_p, buf_len, 1,fileFd);
    // ret = LSAPI_FS_Write(fileFd, (void *)buff_p, buf_len);
    if (ret <= 0)
    {
        LOGE("LSAPI_FS_Write failed. ret:%d\n", ret);
        goto exit1;
    }
exit1:
    free(buff_p);
exit0:
    ql_fclose(fileFd);
    // LSAPI_FS_Close(fileFd);
    return;
}
/**
 * @brief 加载json配置文件
 *
 * @param name
 * @return cJSON*
 */
static cJSON *load_json_cfg_file(const char *name)
{
    if (name == NULL)
    {
        return NULL;
    }
    cJSON *json = NULL;
    int8_t *buff_p = NULL;
    int32_t fileFd = -1;
    int file_size = 0;
    int32_t ret = 0;
    fileFd = ql_fopen(name, "rb");
    if (fileFd < 0)
    {
        LOGI("LSAPI_FS_Open failed, file:%s\n", name);
        return NULL;
    }
    file_size = ql_fseek(fileFd, 0, QL_SEEK_END);
    ql_fseek(fileFd, 0, QL_SEEK_SET);

    buff_p = (int8_t *)malloc(file_size + 1); // byte
    if (buff_p == NULL)
    {
        // LOGE("LSAPI_OSI_Malloc failed, size:%d\n", fileStat.st_size + 1);
        LOGI("LSAPI_OSI_Malloc failed, size:%d\n", file_size + 1);
        goto exit0;
    }
    // memset(buff_p, 0x00, fileStat.st_size + 1);
    memset(buff_p, 0x00, file_size + 1);
    // ret = LSAPI_FS_Read(fileFd, (void *)buff_p, fileStat.st_size);
    ret = ql_fread((void *)buff_p, file_size, 1, fileFd);
    if (ret <= 0)
    {
        LOGI("LSAPI_FS_Read failed. ret:%d\n", ret);
        goto exit1;
    }
    // buff_p[fileStat.st_size] = '\0';
    buff_p[file_size] = '\0';
    LOGI("read json %d:%s", file_size, buff_p);
    ql_fclose(fileFd);
    json = cJSON_Parse((const char *)buff_p);
    free(buff_p);
    buff_p = NULL;
    if (json == NULL)
    {
        LOGI("JSON_Parse Err ret%s", cJSON_GetErrorPtr());
    }
    return json;
exit1:
    free(buff_p);
    buff_p = NULL;
exit0:
    ql_fclose(fileFd);
    return NULL;
}
/**
 * @brief 加载转换参数配置
 *
 * @param coord_cfg
 */
void load_coord_cfg(struct ConfigParam *coord_cfg, const char *filePath)
{
    cJSON *node, *child;
    cJSON *cfg_json = load_json_cfg_file(filePath);
    if (cfg_json != NULL)
    {
        node = cJSON_GetObjectItem(cfg_json, "dx");
        if (node != NULL)
        {
            coord_cfg->dx = node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "dy");
        if (node != NULL)
        {
            coord_cfg->dy = node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "dz");
        if (node != NULL)
        {
            coord_cfg->dz = node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "wx");
        if (node != NULL)
        {
            coord_cfg->wx = node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "wy");
        if (node != NULL)
        {
            coord_cfg->wy = node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "wz");
        if (node != NULL)
        {
            coord_cfg->wz = node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "k");
        if (node != NULL)
        {
            coord_cfg->k = node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "height");
        if (node != NULL)
        {
            coord_cfg->height = node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "gcsSrc");
        if (node != NULL)
        {
            coord_cfg->gcsSrc = node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "gcsDst");
        if (node != NULL)
        {
            coord_cfg->gcsDst = node->valueint;
        }
        LOGE("load_coord_cfg success, filePath:%s\n", filePath);
        cJSON_Delete(cfg_json);
    }
    else
    {
        LOGE("cfg_json is null, filePath:%s\n", filePath);
    }
}
/**
 * @brief 保存转换参数配置
 *
 * @param coord_cfg
 */
void save_coord_cfg(struct ConfigParam *coord_cfg, const char *filePath)
{
    cJSON *cfg_json = cJSON_CreateObject();
    if (cfg_json != NULL)
    {
        cJSON_AddNumberToObject(cfg_json, "dx", coord_cfg->dx);
        cJSON_AddNumberToObject(cfg_json, "dy", coord_cfg->dy);
        cJSON_AddNumberToObject(cfg_json, "dz", coord_cfg->dz);
        cJSON_AddNumberToObject(cfg_json, "wx", coord_cfg->wx);
        cJSON_AddNumberToObject(cfg_json, "wy", coord_cfg->wy);
        cJSON_AddNumberToObject(cfg_json, "wz", coord_cfg->wz);
        cJSON_AddNumberToObject(cfg_json, "k", coord_cfg->k);
        cJSON_AddNumberToObject(cfg_json, "height", coord_cfg->height);
        cJSON_AddNumberToObject(cfg_json, "gcsSrc", coord_cfg->gcsSrc);
        cJSON_AddNumberToObject(cfg_json, "gcsDst", coord_cfg->gcsDst);
        save_cfg_file(cfg_json, filePath);
        cJSON_Delete(cfg_json);
    }
}
/**
 * @brief 加载计算参数配置
 *
 * @param coord_cfg
 */
void load_calc_cfg(struct gps_coord_calc *calc_cfg, const char *filePath)
{
    cJSON *node, *child;
    cJSON *cfg_json = load_json_cfg_file(filePath);
    if (cfg_json != NULL)
    {
        node = cJSON_GetObjectItem(cfg_json, "y_offset");
        if (node != NULL)
        {
            calc_cfg->y_offset = (double)node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "x_offset");
        if (node != NULL)
        {
            calc_cfg->x_offset = (double)node->valueint;
        }
        node = cJSON_GetObjectItem(cfg_json, "angle_offset");
        if (node != NULL)
        {
            calc_cfg->angle_offset = (double)node->valueint;
        }
        LOGT("load_calc_cfg success, filePath:%s, y_offset:%lf, x_offset:%lf, angle_offset:%lf\n",
             filePath, calc_cfg->y_offset, calc_cfg->x_offset, calc_cfg->angle_offset);
        cJSON_Delete(cfg_json);
    }
    else
    {
        LOGE("cfg_json is null, filePath:%s\n", filePath);
    }
}

void gps_coord_init(const char *filePath)
{
    memset(&gps_coord_cfg, 0x00, sizeof(gps_coord_cfg));
    /**
     * @brief 加载配置 若无配置需要在远程传输后自动加载配置 可指定配置文件路径
     *
     */
    if (filePath == NULL)
    {
        load_coord_cfg(&gps_coord_cfg, COORD_CFG_FILE_PATH);
    }
    else
    {
        load_coord_cfg(&gps_coord_cfg, filePath);
    }
    // 初始化常用椭球参数信息
    DatumEllipsoidal(0, 0, NULL); // 初始化中央子午线118
}

void gps_coord_calc_init(const char *filePath)
{
    memset(&gps_coord_calc_cfg, 0x00, sizeof(gps_coord_calc_cfg));
    if (filePath == NULL)
    {
        load_calc_cfg(&gps_coord_calc_cfg, CALC_CFG_FILE_PATH);
    }
    else
    {
        load_calc_cfg(&gps_coord_calc_cfg, filePath);
    }
}
#endif
#define PI (3.1415926)
#define ANGLE_TO_RADIAN(a) ((a) * (PI) / 180) // 角度转弧度
#define ANGLE_TO_DEGREES(a) ((a)*180 / (PI))  // 角度转度
/**
 * @brief 点旋转
 *
 * @param point 指定点
 * @param angle 旋转角度
 * @param origin 原始点
 * @param res 结果点
 * @return int
 */
static int pointRotate(struct Point *point, double angle, struct Point *origin, struct Point *res)
{
    if (point == NULL || res == NULL)
    {
        return 1;
    }
    double r = ANGLE_TO_RADIAN(angle); // 角度转弧度??
    if (origin == NULL || (origin->x == 0 && origin->y == 0))
    {
        // See: https://en.wikipedia.org/wiki/Cartesian_coordinate_system#Rotation
        res->x = point->x * cos(r) - point->y * sin(r);
        res->y = point->x * sin(r) + point->y * cos(r);
        return 0;
    }
    else
    {
        // See: https://math.stackexchange.com/questions/1964905/rotation-around-non-zero-point
        struct Point p0 = {point->x - origin->x, point->y - origin->y};
        struct Point rotated;
        rotated.x = p0.x * cos(r) - p0.y * sin(r);
        rotated.y = p0.x * sin(r) + p0.y * cos(r);
        res->x = rotated.x + origin->x;
        res->y = rotated.y + origin->y;
        return 0;
    }
}
/**
 * @brief 以度数和距离为单位按角度平移点
 *
 * @param point 需要平移的原始点
 * @param angle 角度
 * @param distance 距离
 * @param res_point 结果点
 */
static void pointTranslate(struct Point *point, double angle, double distance,
                           struct Point *res_point)
{
    if (point == NULL || res_point == NULL)
    {
        return;
    }
    double r = ANGLE_TO_RADIAN(angle);
    res_point->x = point->x + distance * cos(r);
    res_point->y = point->y + distance * sin(r);
    return;
}
static double lineAngle(struct Point *line_s, struct Point *line_e)
{
    if (line_s == NULL || line_e == NULL)
    {
        return 0;
    }
    return ANGLE_TO_DEGREES(atan2(line_e->y - line_s->y, line_e->x - line_s->x));
}
/**
 * @brief 计算出从指定线起始点为x_offset距离的点的坐标
 *
 * @param line_s 线起始点
 * @param line_e 线终点
 * @param x_offset 偏移距离
 * @param res_point 结果点
 * @return int
 */
static int lineInterpolate(struct Point *line_s, struct Point *line_e, double x_offset,
                           struct Point *res_point)
{ // line_s + line_e构成一条直线
    if (line_e == NULL || line_s == NULL)
    {
        return 1;
    }
    /**
     * @brief 以起始点line_s作为原点进行旋转
     *        旋转角度为起始点和终点的连线并且得出与水平线的夹角lineAngle(line_s, line_e)顺时针旋转
     *        最终根据偏移x_offset计算出结果点
     */
    pointTranslate(line_s, lineAngle(line_s, line_e), x_offset, res_point);
    return 0;
}

// 七参数modbus参数地址集合
#define MODBUS_GPS_COORD_DX_ADDR 1024
#define MODBUS_GPS_COORD_DY_ADDR 1026
#define MODBUS_GPS_COORD_DZ_ADDR 1028
#define MODBUS_GPS_COORD_WX_ADDR 1030
#define MODBUS_GPS_COORD_WY_ADDR 1032
#define MODBUS_GPS_COORD_WZ_ADDR 1034
#define MODBUS_GPS_COORD_K_ADDR 1036
#define MODBUS_GPS_COORD_HEIGHT_ADDR 1038
#define MODBUS_GPS_COORD_GCSSRC_ADDR 1040
#define MODBUS_GPS_COORD_GCSDST_ADDR 1041
// 计算参数modbus参数地址集合
#define MODBUS_GPS_CALC_X_ADDR 1042
#define MODBUS_GPS_CALC_Y_ADDR 1044
#define MODBUS_GPS_CALC_ANGLE_ADDR 1046

static inline void get_gps_coord_param_bymodbus(struct ConfigParam *gps_coord_cfg)
{
    if (gps_coord_cfg == NULL)
    {
        return;
    }
    gps_coord_cfg->dx =
        (double)((gWordVar[MODBUS_GPS_COORD_DX_ADDR] | gWordVar[MODBUS_GPS_COORD_DX_ADDR + 1] << 16) /
                 1000000.0); // 六位小数
    gps_coord_cfg->dy =
        (double)((gWordVar[MODBUS_GPS_COORD_DY_ADDR] | gWordVar[MODBUS_GPS_COORD_DY_ADDR + 1] << 16) /
                 1000000.0);
    gps_coord_cfg->dz =
        (double)((gWordVar[MODBUS_GPS_COORD_DZ_ADDR] | gWordVar[MODBUS_GPS_COORD_DZ_ADDR + 1] << 16) /
                 1000000.0);
    gps_coord_cfg->wx =
        (double)((gWordVar[MODBUS_GPS_COORD_WX_ADDR] | gWordVar[MODBUS_GPS_COORD_WX_ADDR + 1] << 16) /
                 1000000.0);
    gps_coord_cfg->wy =
        (double)((gWordVar[MODBUS_GPS_COORD_WY_ADDR] | gWordVar[MODBUS_GPS_COORD_WY_ADDR + 1] << 16) /
                 1000000.0);
    gps_coord_cfg->wz =
        (double)((gWordVar[MODBUS_GPS_COORD_WZ_ADDR] | gWordVar[MODBUS_GPS_COORD_WZ_ADDR + 1] << 16) /
                 1000000.0);
    gps_coord_cfg->k =
        (double)((gWordVar[MODBUS_GPS_COORD_K_ADDR] | gWordVar[MODBUS_GPS_COORD_K_ADDR + 1] << 16) /
                 1000000.0);
    gps_coord_cfg->height = (double)((gWordVar[MODBUS_GPS_COORD_HEIGHT_ADDR] |
                                      gWordVar[MODBUS_GPS_COORD_HEIGHT_ADDR + 1] << 16) /
                                     1000000.0);
    gps_coord_cfg->gcsSrc = (gWordVar[MODBUS_GPS_COORD_GCSSRC_ADDR]);
    gps_coord_cfg->gcsDst = (gWordVar[MODBUS_GPS_COORD_GCSDST_ADDR]);
}

static inline void get_gps_calc_param_bymodbus(struct gps_coord_calc *gps_calc_cfg)
{
    if (gps_calc_cfg == NULL)
    {
        return;
    }
    gps_calc_cfg->x_offset =
        (double)((gWordVar[MODBUS_GPS_CALC_X_ADDR] | gWordVar[MODBUS_GPS_CALC_X_ADDR + 1] << 16) /
                 1000000.0); // 六位小数
    gps_calc_cfg->y_offset =
        (double)((gWordVar[MODBUS_GPS_CALC_Y_ADDR] | gWordVar[MODBUS_GPS_CALC_Y_ADDR + 1] << 16) /
                 1000000.0);
    gps_calc_cfg->angle_offset =
        (double)((gWordVar[MODBUS_GPS_CALC_ANGLE_ADDR] | gWordVar[MODBUS_GPS_CALC_ANGLE_ADDR + 1] << 16) /
                 1000000.0);
}

#if 0
/**
 * @brief 坐标转换+计算中心坐标点任务
 * 1、获取Gps1/2的原始经纬度坐标
 * 2、将经纬度坐标转换成平面坐标
 * 3、根据计算参数计算目标节点坐标
 */
/**
 * @brief
 *
 * @param gps1
 * @param gps2
 */
void gps_calc_target_point(gps_t *gps1, gps_t *gps2, struct car_param_data *cpd)
{
    if (gps1 == NULL || gps2 == NULL || cpd == NULL)
    {
        return;
    }
    if (!(gps1->LAT != 0 && gps1->LNG != 0 && gps2->LAT != 0 && gps2->LNG != 0))
    {
        return;
    }
    // get_gps_coord_param_bymodbus(&gps_coord_cfg); // 从modbus更新七参数
    // get_gps_calc_param_bymodbus(&gps_coord_calc_cfg); // 从modbus更新计算参数
    LOGD("angle_offset:%lf, x_offset:%lf, y_offset:%lf\n",
         gps_coord_calc_cfg.angle_offset, gps_coord_calc_cfg.x_offset,
         gps_coord_calc_cfg.y_offset);
    struct Point gps1_d2p_i = {(double)(gps1->LAT / 10000000.00), (double)(gps1->LNG / 10000000.00),
                               0.000000000000};
    struct Point gps1_d2p_res = d2p(&gps1_d2p_i, &gps_coord_cfg); // 大地坐标转平面坐标

    struct Point gps2_d2p_i = {(double)(gps2->LAT / 10000000.00), (double)(gps2->LNG / 10000000.00),
                               0.000000000000};
    struct Point gps2_d2p_res = d2p(&gps2_d2p_i, &gps_coord_cfg);

    LOGD("gps1: src_x:%lf, src_y:%lf, res_x:%lf, res_y:%lf\n", gps1_d2p_i.x,
         gps1_d2p_i.y, gps1_d2p_res.x, gps1_d2p_res.y);
    LOGD("gps2: src_x:%lf, src_y:%lf, res_x:%lf, res_y:%lf\n", gps2_d2p_i.x,
         gps2_d2p_i.y, gps2_d2p_res.x, gps2_d2p_res.y);
    // ## 1 旋转Gps2点
    struct Point gps2ResPoint; // 旋转后Gps2的坐标
    // pointRotate(&gps2_d2p_res, gps_coord_calc_cfg.angle_offset, &gps1_d2p_res, &gps2ResPoint);
    // cpd->angle = lineAngle(&gps1_d2p_res, &gps2ResPoint) - 90;  // 保留当前车体角度
    // ## 2 平移Gps1点(x_offset)
    struct Point gps1ResPoint; // 平移后Gps1的坐标
    lineInterpolate(&gps1_d2p_res, &gps2ResPoint, gps_coord_calc_cfg.x_offset, &gps1ResPoint);
    // ## 3 平移Gps1点(y_offset) // 旋转+平移
    struct Point gps2_ResPoint; // 旋转后Gps2`的坐标
    double angle_offset_tmp = 0;
    if (gps_coord_calc_cfg.y_offset >= 0)
    {
        angle_offset_tmp = -90; // 目标点在gps1_d2p_res与gps2ResPoint之间的直线右侧
    }
    else
    {
        angle_offset_tmp = 90; // 目标点在gps1_d2p_res与gps2ResPoint之间的直线左侧
    }
    pointRotate(&gps2ResPoint, angle_offset_tmp, &gps1ResPoint,
                &gps2_ResPoint); // 以gps1ResPoint为起点
    struct Point flat_res_point;
    lineInterpolate(&gps1ResPoint, &gps2_ResPoint, gps_coord_calc_cfg.y_offset,
                    &flat_res_point); // 以gps1ResPoint为起点
    // ## end 将平面坐标转换成大地坐标(上面的结果为平面坐标)
    // cpd->target = p2d(&flat_res_point, &gps_coord_cfg);  // 大地坐标转平面坐标
    // log_printf(LDEBUG, LOG_TAG "res_point: x:%lf, y:%lf, angle:%lf\n", cpd->target.x, cpd->target.y,
    //            cpd->angle);
    return;
}
#endif 

typedef struct gps_write_data
{
	uint16_t magic;
	uint16_t ellipsoidal_datum;	// 椭圆基准
	uint8_t reg[4];

    int16_t x0;		// 卫星接收器点坐标（坐标系以基站为原点建立）
	int16_t y0;
	int16_t x1;		// 待求点坐标
	int16_t y1;

	uint64_t central_meridian;	// 中央子午线
	uint64_t elevation;			// 投影高程

	int64_t dx;		// x轴平移
	int64_t ax;		// x轴旋转

	int64_t dy;		// y轴平移
	int64_t ay;		// y轴旋转

	int64_t dz;		// z轴平移
	int64_t az;		// z轴旋转

	int64_t k;		// 缩放比例
} gps_write_data_t;
extern gps_write_data_t *gpsWriteDataP = (gps_write_data_t *)&gWordVar[REG_GPS_WRITE_DATA];
void modbus_write_gps_data_cb(uint16_t *addr, int len)
{
    // LOGI("modbus_write_gps_data_cb");
    LOGI("gps addr[%d]=0x%04x", REG_GPS_WRITE_DATA, gWordVar[REG_GPS_WRITE_DATA]);
    if (addr == &gWordVar[REG_GPS_WRITE_DATA] && *addr == WRITE_SAVE_MAGIC)   // 写入gps_write_data_t
    {
        LOGI("write gps crood param");
        double scale1 = 100000000.0;
        gps_coord_cfg.dx = gpsWriteDataP->dx / scale1;
        gps_coord_cfg.dy = gpsWriteDataP->dy / scale1;
        gps_coord_cfg.dz = gpsWriteDataP->dz / scale1;
        gps_coord_cfg.wx = gpsWriteDataP->ax / scale1;
        gps_coord_cfg.wy = gpsWriteDataP->ay / scale1;
        gps_coord_cfg.wz = gpsWriteDataP->az / scale1;
        gps_coord_cfg.k  = gpsWriteDataP->k  / scale1;
        gps_coord_cfg.gcsDst = gpsWriteDataP->ellipsoidal_datum;    // 参照椭圆
        
        gps_coord_cfg.height = gpsWriteDataP->elevation / scale1;    // 投影高程
        double L0 = gpsWriteDataP->central_meridian / scale1;

        gWordVar[REG_GPS_WRITE_DATA] = 0x0000;
        zb_ModBusWordWriteHook(REG_GPS_WRITE_DATA,1);
        DatumEllipsoidal(L0, 0, NULL); // 初始化中央子午线
        save_gps_point_coord();
    }
}

// int ql_nvm_fwrite(const char *config_file_name, void *buffer, size_t size, size_t num);
// int ql_nvm_fread(const char *config_file_name, void *buffer, size_t size, size_t num);

void gps_point_coord_init(void)
{   
    int err;
    err = ql_nvm_fread("gps_coord_cfg", &gps_coord_cfg, sizeof(gps_coord_cfg), 1);
    if( err != sizeof(gps_coord_cfg))
    {
        gps_coord_cfg.dx = 0;
        gps_coord_cfg.dy = 0;
        gps_coord_cfg.dz = 0;
        gps_coord_cfg.wx = 0;
        gps_coord_cfg.wy = 0;
        gps_coord_cfg.wz = 0;
        gps_coord_cfg.k = 0;
        gps_coord_cfg.height = 0;
        gps_coord_cfg.gcsSrc = WGS84;
        gps_coord_cfg.gcsDst = XA80;
    }
    LOGI("gps_coord_cfg.gcsSrc=%d,gcsDst=%d",gps_coord_cfg.gcsSrc,gps_coord_cfg.gcsDst);
    // 初始化常用椭球参数信息
    DatumEllipsoidal(0, 0, NULL); // 初始化中央子午线118
    
}
void save_gps_point_coord(void)
{
    int err;
    err = ql_nvm_fwrite("gps_coord_cfg", &gps_coord_cfg, sizeof(gps_coord_cfg), 1);
}


struct Point target_point;
struct Point target_point_d;
struct Point result_point;
struct Point target_res_point;
static void target_point_cal(struct Point *point, double angle, double distance,
                           struct Point *res_point)
{
    if (point == NULL || res_point == NULL)
    {
        return;
    }
    double r = ANGLE_TO_RADIAN(angle);
    res_point->x = point->x + distance * cos(r) ;
    res_point->y = point->y + distance * sin(r) ;
    return;
}
//定义哪一个方向为正方向，比如直角坐标系的x轴的正方向为东，y轴的正方向为北
static void target_point_revolve(struct Point *point, double angle, struct Point *mid_point,struct Point *res_point)
{
    if( point == NULL || res_point == NULL)
    {
        return;
    }
    double revole_angle = 360 - angle;
    double r = ANGLE_TO_RADIAN(revole_angle);
    //点绕点旋转
    // x1=(x-xb)cosθ - (y-yb)sinθ + xb
    // y1=(y-yb)cosθ + (x-xb)sinθ + yb
    struct Point p0 = {point->x - mid_point->x, point->y - mid_point->y};
    struct Point rotated;
    rotated.x = p0.x * cos(r) - p0.y * sin(r);
    rotated.y = p0.x * sin(r) + p0.y * cos(r);
    res_point->x = rotated.x ;
    res_point->y = rotated.y ;
    return 0;

}
void gps_cale_point(gps_t *gps1)
{
    if(gps1 == NULL)
    {
        return;
    }
    if(gps1->LAT == 0 || gps1->LNG == 0)
    {
        return;
    }
    
    struct Point gps1_d2p_i = {(double)(gps1->LAT / 1000000000.00), (double)(gps1->LNG / 1000000000.00),
                               0.000000000000};
    struct Point gps1_d2p_res = d2p(&gps1_d2p_i, &gps_coord_cfg); // 大地坐标转平面坐标

    LOGI("gps1: src_x:%f, src_y:%f, res_x:%f, res_y:%f\n", gps1_d2p_i.x,
         gps1_d2p_i.y, gps1_d2p_res.x, gps1_d2p_res.y);

    //计算目标点的直角坐标
    // struct Point target_point;
    gps1->baseline = gps1->baseline/2;
    target_point_cal(&gps1_d2p_res, gps1->pitch, gps1->baseline, &target_point);
    LOGI("target_point.x:%f, y:%f\n", target_point.x,target_point.y);

    struct Point gps1_d = {(gps1_d2p_res.x + gps1->baseline), gps1_d2p_res.y, 0.000000000000};
    target_point_revolve(&gps1_d2p_res, gps1->Heading, &gps1_d, &target_point_d);
    LOGI("target_point_d.x:%f, y:%f\n", target_point_d.x,target_point_d.y);

    result_point.x = (target_point_d.x + target_point.x);
    result_point.y = (target_point_d.y + target_point.y);
    result_point.z = 0.000000000000;
    
    //计算目标点的经纬度
    // struct Point target_res_point;
    target_res_point = p2d(&result_point, &gps_coord_cfg); // 平面坐标转大地坐标
    LOGI("target_res_point.x:%f, y:%f\n", target_res_point.x,target_res_point.y);
}
