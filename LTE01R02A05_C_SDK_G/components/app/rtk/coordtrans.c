#include "coordtrans.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "coordtrans", msg, ##__VA_ARGS__)
struct Datum g_coord_p[GEODETIC_COORD_SYSTEM_MAX];  // 椭球参数集保存结构

/*! 椭球参数初始化
 * @param a 椭球长半轴
 * @param f 椭球扁率
 * @param L0 本初子午线
 */
void DatumInitialize(struct Datum *newd, double a, double f, double l0) {
  if (newd == NULL)
    return;
  double b = a - f * a;
  double e12 = (a * a - b * b) / (a * a);
  double e22 = (a * a - b * b) / (b * b);

  // local BJ54 = new Datum(6378245, 1/298.3)
  // local XA80 = new Datum(6378140, 1/298.257)
  // local WGS84 = new Datum(6378137, 1/298.257223563)
  // local WGS2000 = new Datum(6378137, 1/298.257222101)
  newd->A = a;  // 椭球长半轴
  newd->F = f;  // 椭球扁率
  newd->B = b;  // 椭球短半轴
  newd->E12 = e12;  // 椭球第一偏心率平方
  newd->E22 = e22;  // 椭球第二偏心率平方
  newd->L0 = l0;  // 本初子午线
  newd->zoneWidth = 3;  // 度带
}

/*! 设置度带
 * @param zoneWidth 度带
 */
void DatumSetZoneWidth(struct Datum *newd, int zoneWidth) {
  if (newd == NULL)
    return;
  if (zoneWidth != 6 && zoneWidth != 3) {
    printf("带宽应当为6或者为3");
  } else {
    newd->zoneWidth = zoneWidth;
  }
}

/*! 获取度带带号
 * @param l 当地经度
 * @return 度带带号
 */
int DatumGetZoneNo(struct Datum *newd, double l) {
  // 6度带
  // 这里应为向下取整
  if (newd->zoneWidth == 6) {
    return (l + 5) / 6; // 向上取整，从6开始，0-6一度带，以此类推 L0=度带*6-3
    // return floor((l + 6) / 6);
  }
  // 3度带
  return floor((l + 1.5) / 3);  // 从1.5开始，1.5-4.5一度带 L0=度带*3
}

// 根据经纬度获取所在度带的中央子午线经度
double DatumGetL0(struct Datum *newd, double l) {
  int zoneNo = DatumGetZoneNo(newd, l);
  if (newd->zoneWidth == 6) {
    return zoneNo * 6 - 3;
  }
  // 3度带
  return zoneNo * 3; 
}
/*! 椭球参数计算
 * @param val 所指定的坐标系
 * @param L0 中央子午线
 * @return 各坐标系下的椭球参数信息
 */
void DatumEllipsoidal(double L0, enum GEODETIC_COORD_SYSTEM val, struct Datum *out) {
  DatumInitialize(&g_coord_p[BJ54], 6378245, 1 / 298.3, L0);
  DatumInitialize(&g_coord_p[XA80], 6378140, 1 / 298.257, L0);
  DatumInitialize(&g_coord_p[WGS84], 6378137, 1 / 298.257223563, L0);
  DatumInitialize(&g_coord_p[WGS2000], 6378137, 1 / 298.257222101, L0);
  /*
  double A;  // 椭球长半轴
  double F;  // 椭球扁率
  double B;  // 椭球短半轴
  double E12;  // 椭球第一偏心率平方
  double E22;  // 椭球第二偏心率平方
  int L0;  // 本初子午线
  int zoneWidth;  // 度带
  */
  /*
  printf("XA80: A:%lf, B:%lf, F:%lf, E12:%lf, E22:%lf, L0:%d, zoneWidth:%d\n", 
  g_coord_p[XA80].A, g_coord_p[XA80].B, g_coord_p[XA80].F, g_coord_p[XA80].E12, 
  g_coord_p[XA80].E22, g_coord_p[XA80].L0, g_coord_p[XA80].zoneWidth);
  printf("WGS84: A:%lf, B:%lf, F:%lf, E12:%lf, E22:%lf, L0:%d, zoneWidth:%d\n", 
  g_coord_p[WGS84].A, g_coord_p[WGS84].B, g_coord_p[WGS84].F, g_coord_p[WGS84].E12, 
  g_coord_p[WGS84].E22, g_coord_p[WGS84].L0, g_coord_p[WGS84].zoneWidth);
  */
  if (out == NULL)
    return; 
  *out = g_coord_p[val];
}

/*! 输出平面坐标
 * @param p 地心大地坐标参数 p.B:纬度，p.L:经度
 * @param config 要转换成坐标系下的七参数信息
 * @return 输出投影的平面坐标
 */
struct Point d2p(struct Point *p, struct ConfigParam *config) {
  struct Point zj = BLH2XYZ(p, &g_coord_p[config->gcsSrc]);
  struct Point xzj = XYZ2XYZ(&zj, config->dx, config->dy, config->dz, config->wx, config->wy, config->wz, config->k);
  struct Point xdd = XYZ2BLH(&xzj, &g_coord_p[config->gcsDst]);
  struct Point xpm = BL2xy(&xdd, &g_coord_p[config->gcsDst], config->height, NULL, NULL);
  return xpm;
}
/*! 地心大地坐标系 转换到 地心直角坐标系
 * @param pointBLH 地心大地坐标系参数 B:维度 L:经度 H:高程
 * @param datum 椭球参数
 */
struct Point BLH2XYZ(struct Point *pointBLH, struct Datum *datum) {
  double a = datum->A;
  double e12 = datum->E12;
  double radB = pointBLH->x / 180 * MATH_PI;
  double radL = pointBLH->y / 180 * MATH_PI;
  double H = pointBLH->z;
  double N = a / sqrt(1 - e12 * sin(radB) * sin(radB));  // 卯酉圈半径
  struct Point res;
  res.x = (N + H) * cos(radB) * cos(radL);
  res.y = (N + H) * cos(radB) * sin(radL);
  res.z = (N * (1 - e12) + H) * sin(radB);
  return res;
}

/*! 空间直角坐标系间的 七参数转换
 * @note: 不同椭球参数下， 地心直角坐标系之间转换
 * @param: dX, dY, dZ: 三个坐标方向的平移参数
 * @param: wX, wY, wZ: 三个方向的旋转角参数(单位为弧度)
 * @param: Kppm: 尺度参数， 单位是ppm，如果是以米为单位， 需要在传参前 除以1000000
 */
struct Point
    XYZ2XYZ(struct Point *source, double dX, double dY, double dZ, double wX, double wY, double wZ, double Kppm) {
  double X = source->x;
  double Y = source->y;
  double Z = source->z;
  // wX = wX or 0
  // wY = wY or 0
  // wZ = wZ or 0
  // Kppm = Kppm or 0
  wX = wX / 3600 / 180 * MATH_PI;
  wY = wY / 3600 / 180 * MATH_PI;
  wZ = wZ / 3600 / 180 * MATH_PI;
  // wX = wX / math.PI * 3600 * 180
  // wY = wY / math.PI * 3600 * 180
  // wZ = wZ / math.PI * 3600 * 180
  // Kppm = Kppm - 1
  struct Point res;
  res.x = X + dX + Kppm * X - wY * Z + wZ * Y;  // Kppm 公式中尺度因子为 1+Kppm
  res.y = Y + dY + Kppm * Y + wX * Z - wZ * X;
  res.z = Z + dZ + Kppm * Z - wX * Y + wY * X;
  return res;
}

/*! 地心直角坐标系 转换到 地心大地坐标系
 * @note: 用直接法2 https://wenku.baidu.com/view/30a08f9ddd88d0d233d46a50.html
 */
struct Point
    XYZ2BLH(struct Point *pointXYZ, struct Datum *datum) {
  double X = pointXYZ->x;
  double Y = pointXYZ->y;
  double Z = pointXYZ->z;
  double L = atan(Y / X);
  // 弧度转角度
  double degL = L * 180 / MATH_PI;
  // Y值为正， 东半球， 否则西半球
  if (Y > 0) {
    while (degL < 0) {
      degL = degL + 180;
    }
    while (degL > 180) {
      degL = degL - 180;
    }
  } else {
    while (degL > 0) {
      degL = degL - 180;
    }
    while (degL < -180) {
      degL = degL + 180;
    }
  }
  double a = datum->A;
  double b = datum->B;
  double e12 = datum->E12;
  double e22 = datum->E22;

  double tgU = Z / (sqrt(X * X + Y * Y) * sqrt(1 - e12));
  double U = atan(tgU);

  double tgB = (Z + b * e22 * pow(sin(U), 3)) / (sqrt(X * X + Y * Y) - a * e12 * pow(cos(U), 3));
  double B = atan(tgB);
  double degB = B * 180 / MATH_PI;  // 弧度转角度

  if (Z > 0) {  // Z值为正， 北半球， 否则南半球
    while (degB < 0) {
      degB = degB + 90;
    }
    while (degB > 90) {
      degB = degB - 90;
    }
  } else {
    while (degB > 0) {
      degB = degB - 90;
    }
    while (degB < -90) {
      degB = degB + 90;
    }
  }
  while (degB < 0) {
    degB = degB + 360;
  }
  while (degB > 360) {
    degB = degB - 360;
  }
  double N = a / sqrt(1 - e12 * sin(B) * sin(B));  // 卯酉圈半径
  double H = 0;
  // B接近极区， 在±90°附近
  if (fabs(degB) > 80) {
    H = Z / sin(B) - N * (1 - e12);
  } else {
    H = sqrt(X * X + Y * Y) / cos(B) - N;
  }
  struct Point res;
  res.x = degB;
  res.y = degL;
  res.z = H;
  return res;
}

/*! 地心大地坐标系 转换到 大地平面坐标系
 * @note: 高斯投影坐标
 * @param: prjHeight: 投影面高程
 * http://www.cnblogs.com/imeiba/p/5696967.html
 */
struct Point
    BL2xy(struct Point *pointBLH, struct Datum *datum, double prjHeight, double *offsetX_o, double *offsetY_o) {
  double a = datum->A;
  double b = datum->B;
  double e12 = datum->E12;
  double e22 = datum->E22;

  double offsetX;
  double offsetY;
  // prjHeight = prjHeight or 0
  // offsetX = offsetX or 0
  if (offsetX_o == NULL)
    offsetX = 0;
  else
    offsetX = *offsetX_o;
  // offsetY = offsetY or 500000
  if (offsetY_o == NULL)
    offsetY = 500000;
  else
    offsetY = *offsetY_o;

  double L0 = datum->L0;   // 跨度带的话，L0就不对了，应该根据x,y推出现在的度带，再得到L0
  LOGI("L0:%f", L0);
  if (L0 == 0) {
    datum->L0 = DatumGetL0(datum, pointBLH->y);
    LOGI("get L0:%f", L0);
  }
  double radL0 = (((double)L0 / (double)180 )* MATH_PI);
  double radB = pointBLH->x / 180 * MATH_PI;
  double radL = pointBLH->y / 180 * MATH_PI;
  double N = a / sqrt(1 - e12 * sin(radB) * sin(radB));  // 卯酉圈半径
  double T = tan(radB) * tan(radB);
  double C = e22 * cos(radB) * cos(radB);
  double A = (radL - radL0) * cos(radB);
  double M = a * ((1 - e12 / 4 - 3 * e12 * e12 / 64 - 5 * e12 * e12 * e12 / 256) * radB -
                  (3 * e12 / 8 + 3 * e12 * e12 / 32 + 45 * e12 * e12 * e12 / 1024) * sin(2 * radB) +
                  (15 * e12 * e12 / 256 + 45 * e12 * e12 * e12 / 1024) * sin(4 * radB) -
                  (35 * e12 * e12 * e12 / 3072) * sin(6 * radB));
  // x,y的计算公式见孔祥元等主编武汉大学出版社2002年出版的《控制测量学》的第72页书的的括号有问题，( 和 [ 应该交换
  double x = M + N * tan(radB) *
                     (A * A / 2 + (5 - T + 9 * C + 4 * C * C) * A * A * A * A / 24 +
                      (61 - 58 * T + T * T + 600 * C - 330 * e22) * A * A * A * A * A * A / 720);  // ??x和y的计算公式
  double y = N * (A + (1 - T + C) * A * A * A / 6 + (5 - 18 * T * T * T + 72 * C - 58 * e22) * A * A * A * A * A / 120);
  struct Point res;
  res.x = offsetX + x * (b + prjHeight) / b;
  res.y = offsetY + y * (b + prjHeight) / b;
  res.z = pointBLH->z;
  return res;
}

/*! 输出大地坐标
 */
struct Point p2d(struct Point *pointxy, struct ConfigParam *config) {
  struct Point bl = xy2BL(pointxy, &g_coord_p[config->gcsDst], config->height, NULL, NULL);
  struct Point zj = BLH2XYZ(&bl, &g_coord_p[config->gcsDst]);
  struct Point xzj =
      XYZ2XYZ(&zj, -config->dx, -config->dy, -config->dz, -config->wx, -config->wy, -config->wz, -config->k);
  struct Point xdd = XYZ2BLH(&xzj, &g_coord_p[config->gcsSrc]);
  return xdd;
}

/*! 大地平面坐标系 转换到 地心大地坐标系
 * @param: prjHeight: 投影面高程
 * http://www.cnblogs.com/imeiba/p/5696967.html
 */
struct Point xy2BL(struct Point *pointxy, struct Datum *datum, double prjHeight, double *offsetX_o, double *offsetY_o) {
  double a = datum->A;
  double b = datum->B;
  double e12 = datum->E12;
  double e22 = datum->E22;
  double offsetX;
  double offsetY;
  // prjHeight = prjHeight or 0
  // offsetX = offsetX or 0
  if (offsetX_o == NULL)
    offsetX = 0;
  else
    offsetX = *offsetX_o;
  // offsetY = offsetY or 500000
  if (offsetY_o == NULL)
    offsetY = 500000;
  else
    offsetY = *offsetY_o;

  double e1 = (1 - sqrt(1 - e12)) / (1 + sqrt(1 - e12));
  double L0 = datum->L0;
  LOGI("L0:%f", L0);
  double radL0 = (((double)L0 / (double)180) * MATH_PI);
  // 带内大地坐标
  double Y = fmod(pointxy->y, 1000000.0000000); // pointxy->y % 1000000.0000000;
  double x = (pointxy->x - offsetX) * b / (b + prjHeight);
  double y = (Y - offsetY) * b / (b + prjHeight);

  double u = x / (a * (1 - e12 / 4 - 3 * e12 * e12 / 64 - 5 * e12 * e12 * e12 / 256));
  double fai = u + (3 * e1 / 2 - 27 * e1 * e1 * e1 / 32) * sin(2 * u) +
               (21 * e1 * e1 / 16 - 55 * e1 * e1 * e1 * e1 / 32) * sin(4 * u) + (151 * e1 * e1 * e1 / 96) * sin(6 * u) +
               (1097 * e1 * e1 * e1 * e1 / 512) * sin(8 * u);
  double C = e22 * cos(fai) * cos(fai);
  double T = tan(fai) * tan(fai);
  double N = a / sqrt(1 - e12 * sin(fai) * sin(fai));
  double R = a * (1 - e12) /
             sqrt((1 - e12 * sin(fai) * sin(fai)) * (1 - e12 * sin(fai) * sin(fai)) * (1 - e12 * sin(fai) * sin(fai)));
  double D = y / N;

  double L = radL0 + (D - (1 + 2 * T + C) * D * D * D / 6 +
                      (5 - 2 * C + 28 * T - 3 * C * C + 8 * e22 + 24 * T * T) * D * D * D * D * D / 120) /
                         cos(fai);
  double B = fai - (N * tan(fai) / R) *
                       (D * D / 2 - (5 + 3 * T + 10 * C - 4 * C * C - 9 * e22) * D * D * D * D / 24 +
                        (61 + 90 * T + 298 * C + 45 * T * T - 256 * e22 - 3 * C * C) * D * D * D * D * D * D / 720);
  struct Point res;
  res.x = B * 180 / MATH_PI;
  res.y = L * 180 / MATH_PI;
  res.z = pointxy->z;
  return res;
}
