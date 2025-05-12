#ifndef __COORDTRANS_H__
#define __COORDTRANS_H__

#include <stdio.h>
#define MATH_PI 3.14159265358979323846

/*! 大地坐标系类型
 */
enum GEODETIC_COORD_SYSTEM { BJ54 = 0, XA80, WGS84, WGS2000, CGCS2000, GEODETIC_COORD_SYSTEM_MAX };

/*! 椭球参数存储格式
 */
struct Datum {
  double A;  // 椭球长半轴
  double F;  // 椭球扁率
  double B;  // 椭球短半轴
  double E12;  // 椭球第一偏心率平方
  double E22;  // 椭球第二偏心率平方
  double L0;  // 本初子午线
  int zoneWidth;  // 度带
};

/*! 椭球参数初始化
 * @param a 椭球长半轴
 * @param f 椭球扁率
 * @param L0 本初子午线
 */
void DatumInitialize(struct Datum *newd, double a, double f, double l0);

/*! 设置度带
 * @param zoneWidth 度带
 */
void DatumSetZoneWidth(struct Datum *newd, int zoneWidth);

/*! 获取度带带号
 * @param l 当地经度
 * @return 度带带号
 */
int DatumGetZoneNo(struct Datum *newd, double l);

/*! 获取度带中央子午线经度
 * @param l 当地经度
 * @return 度带带号
 */
double DatumGetL0(struct Datum *newd, double l);
extern struct Datum g_coord_p[];  // 椭球参数集保存结构
/*! 椭球参数计算
 * @param val 所指定的坐标系
 * @param L0 中央子午线
 * @return 各坐标系下的椭球参数信息
 */
void DatumEllipsoidal(double L0, enum GEODETIC_COORD_SYSTEM val, struct Datum *out);

/*! 输入参数格式(配置参数)
 */
struct ConfigParam {
  double dx;  // x坐标平移量(米)
  double dy;  // y坐标平移量(米)
  double dz;  // z坐标平移量(米)
  double wx;  // x坐标旋转角度(秒)
  double wy;  // y坐标旋转角度(秒)
  double wz;  // z坐标旋转角度(秒)
  double k;  // 尺度因子(ppm)，这里其实应该用变量m表示
  double height;  // 投影面高程
  enum GEODETIC_COORD_SYSTEM gcsSrc;  // 原始坐标系
  enum GEODETIC_COORD_SYSTEM gcsDst;  // 结果坐标系
};

/*! 坐标点格式
 */
struct Point {
  double x;  // b // x
  double y;  // l // y
  double z;  // h // h
};

/*! 输出平面坐标
 * @param p 地心大地坐标参数 p.B:纬度，p.L:经度
 * @param config 要转换成坐标系下的七参数信息
 * @return 输出投影的平面坐标
 */
struct Point d2p(struct Point *p, struct ConfigParam *config);

/*! 地心大地坐标系 转换到 地心直角坐标系
 * @param pointBLH 地心大地坐标系参数 B:维度 L:经度 H:高程
 * @param datum 椭球参数
 */
struct Point BLH2XYZ(struct Point *pointBLH, struct Datum *datum);
/*! 空间直角坐标系间的 七参数转换
 * @note: 不同椭球参数下， 地心直角坐标系之间转换
 * @param: dX, dY, dZ: 三个坐标方向的平移参数
 * @param: wX, wY, wZ: 三个方向的旋转角参数(单位为弧度)
 * @param: Kppm: 尺度参数， 单位是ppm，如果是以米为单位， 需要在传参前 除以1000000
 */
struct Point XYZ2XYZ(struct Point *source, double dX, double dY, double dZ, double wX, double wY, double wZ,
                     double Kppm);
/*! 地心直角坐标系 转换到 地心大地坐标系
 */
struct Point XYZ2BLH(struct Point *pointXYZ, struct Datum *datum);

/*! 地心大地坐标系 转换到 大地平面坐标系
 * @note: 高斯投影坐标
 * @param: prjHeight: 投影面高程
 */
struct Point BL2xy(struct Point *pointBLH, struct Datum *datum, double prjHeight, double *offsetX_o, double *offsetY_o);

/*! 输出大地坐标
 * @param p 地心大地坐标参数 
 * @param config 要转换成坐标系下的七参数信息
 * @return 输出投影的平面坐标
 */
struct Point p2d(struct Point *pointxy, struct ConfigParam *config);

/*! 大地平面坐标系 转换到 地心大地坐标系
 * @param: prjHeight: 投影面高程
 */
struct Point xy2BL(struct Point *pointxy, struct Datum *datum, double prjHeight, double *offsetX_o, double *offsetY_o);
#endif  // __COORDTRANS_H__
