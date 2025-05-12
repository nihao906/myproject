/**
 * @file lv_demo_mcxa.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_mcxa.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "osi_api.h"

#include "../../../lv_drv_conf.h"
#include "../../../lvgl/src/core/lv_obj.h"
#include "../../../lvgl/src/extra/widgets/chart/lv_chart.h"
#include "../../../lvgl/src/widgets/lv_canvas.h"
#include "../../lv_demo.h"
#include "user_icon_font.h"
//#include "ql_log.h"
user_data_t g_ui_user_data;               // 界面接收数据
user_data_t g_ui_dis_data;                // 界面接收数据
#if !USE_SDL_SIM

osiThread_t *thread_data = NULL; // 后台ui事件处理线程

static t2n_report_t g_ui_cure_data;       // cure界面数据
static config_data_t *g_ui_cfg_data;      // 后台配置数据存储指针
static char *g_sys_imei_data;             // imei数据
static sys_data_t *g_sys_data;            // 系统数据
#endif
/**
 * @brief ui version
 *
 */
#define UI_VERSION_MAJOR 1
#define UI_VERSION_MINOR 0
#define UI_VERSION_PATCH 0
#define UI_VERSION_INFO "develop" // release

#pragma pack(2)
typedef struct system_cfg
{ // 当前系统配置参数结构
  /**
   * @brief 默认值均为0
   */
  uint8_t operation_start_btn : 1; // 启动按钮 0:关闭 1:启动
  uint8_t operation_pause_btn : 1;
  uint8_t operation_stop_btn : 1;
  uint32_t record_calendar_select; // 选择的日期

  uint8_t setting_depth_dir : 1;
  uint8_t setting_depth_mode : 1;
  uint32_t setting_depth_threshold;   // 深度计阀值
  uint32_t setting_depth_coefficient; // 深度计系数
  uint8_t setting_flow_mode : 1;
  uint16_t setting_flow_pin;

  uint8_t setting_theme_mode : 1; // 主题模式 0:浅色 1:深色
} system_cfg_t;
#pragma pack()
system_cfg_t g_sys_cfg;
/**********************
 *      MACROS
 **********************/
/*! @note: 公用配置 */
#define THEME_COLOR_LIGHT_MODE_DEF 0xFFFFFF
#define THEME_COLOR_DARK_MODE_DEF 0x292831

#define THEME_COLOR_DODER_BLUE_DEF 0x1E90FF
#define THEME_COLOR_WHITE_DEF 0xFFFFFF
#define THEME_COLOR_BLACK_DEF 0x000000
#define THEME_COLOR_SLATE_GREY_DEF 0x708090
#define THEME_COLOR_SPRING_GREEN_DEF 0x00FF7F
#define THEME_COLOR_FIRE_BRICK1_DEF 0xFF3030
#define THEME_COLOR_FIRE_BRICK3_DEF 0xCD2626
#define THEME_COLOR_YELLOW_DEF 0xFFFF00
#define THEME_COLOR_GRAY81_DEF 0xCFCFCF
#define THEME_COLOR_SEA_GREEN1_DEF 0x54FF9F
#define THEME_COLOR_BTN_BLUE_DEF 0x2196F3
#define THEME_COLOR_SNOW3_DEF 0xCDC9C9
#define THEME_COLOR_SNOW4_DEF 0x8B8989
#define THEME_COLOR_SEAGREEN2_DEF 0x4EEE94
#define THEME_COLOR_SEAGREEN2_DEF 0x43CD80
#define THEME_COLOR_GOLD3_DEF 0xCDAD00
/*! @note: 主页界面 */
#define MONITOR_WIDTH MONITOR_HOR_RES
#define MONITOR_HEIGHT MONITOR_VER_RES
#define TABVIEW_TAB_H 30
/*! @note: 作业界面 */
#define LV_GRID_MEM_X_OFFSET 64 // 40
#define LV_GRID_MEM_Y_OFFSET 3
#define LV_GRID_MEM_J_OFFSET 3
/*! @note: 导航栏界面 */
#define TABVIEW_CNT_NUM 4
#define LV_LIST_BTN_WIDTH 40
#define LV_LIST_BTN_HEIGHT (LV_LIST_BTN_WIDTH * 4)
#define LV_LIST_MEM_J_OFFSET 2 // 按钮区域与测量值区域的间距
/*! @note: 倾角状态界面 */
#define INTER_ADJUS_ZHUANGDIAN_NUM "0"
#define INTER_ADJUS_ZHUANGDIAN_NUM_NAME "桩点号"
#define INTER_ADJUS_JINGWEIDU_NUM_NAME "经纬度"
#define INTER_ADJUS_DX_DSC_NUM "DX:"
#define INTER_ADJUS_DX_NUM "0.00"
#define INTER_ADJUS_DY_DSC_NUM "DY:"
#define INTER_ADJUS_DY_NUM "0.00"
#define INTER_ADJUS_INCLINATION_NAME "倾角状态"
#define INTER_ADJUS_X_GEGREE_NUM "x:0.00"
#define INTER_ADJUS_Y_GEGREE_NUM "y:0.00"
#define ADJ_LEFT_MEM_WIDTH (MONITOR_WIDTH / 3)
#define CANVAS_WIDTH ((MONITOR_WIDTH - ADJ_LEFT_MEM_WIDTH) - LV_GRID_MEM_J_OFFSET * 2)
#define CANVAS_HEIGHT (MONITOR_HEIGHT - TABVIEW_TAB_H - LV_GRID_MEM_J_OFFSET)
#define CANVAS_CUR_POINT_SIZE 16
/*! @note: 曲线界面 */
#define CURVE_X_TAG_NAME "料\n量\n  L"
#define CURVE_Y_TAG_NAME "深度(M)"
#define CURVE_Y_AXIS_MIN_VAL (0)
#define CURVE_Y_AXIS_MAX_VAL (200)
#define CURVE_X_AXIS_MIN_VAL (-1) // -0.4m
#define CURVE_X_AXIS_MAX_VAL (25)
#define CURVE_POINT_NUM 260
/*! @note: 作业记录界面 */
#define TABLE_ROW_CNT 7 // table行数
#define TABLE_COL_CNT 7 // table列数
/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
  MEA_SPEED_I = 0,
  MEA_HOLE_DEPTH_I,    // 深度
  FLOW_SS1_LABLE, // 1通道瞬时流量
  FLOW_SS2_LABLE,
  FLOW_10CM1_LABLE, // 1通道瞬时流量
  FLOW_10CM2_LABLE,
  FLOW_LJ1_LABLE, // 1通道累计流量
  FLOW_LJ2_LABLE,

} g_measurements_t;
enum
{
  TABVIEW_ID1 = 0,
  TABVIEW_ID2 = 1,
  TABVIEW_ID3 = 2,
  TABVIEW_ID4 = 3,
  TABVIEW_ID_MAX
}; // tabview页面id
enum
{
  NAV_RIGHT_ID = 0,
  NAV_UP_ID = 1,
  NAV_DOWN_ID = 2,
  NAV_LIST_ID = 3
}; // 左侧按键-id
enum
{
  NAV_LEFT_ID = 0,
  NAV_PAUSE_ID = 1,
  NAV_STOP_ID = 2,
  NAV_PLAY_ID = 3
}; // 右侧按键-id
/**********************
 *  STATIC VARIABLES/FUNCTION
 **********************/
static void navbar_create(lv_obj_t *parent);
static void operation_create(lv_obj_t *parent);
static void operation_delete(lv_obj_t *parent);
static void adjustment_create(lv_obj_t *parent);
static void adjustment_delete(lv_obj_t *parent);
static void curve_create(lv_obj_t *parent);
static void curve_delete(lv_obj_t *parent);
static void record_create(lv_obj_t *parent);
static void record_delete(lv_obj_t *parent);

static void event_handler_tabview(lv_event_t *e);

static void event_handler_list(lv_event_t *e);
static void event_handler_up(lv_event_t *e);
static void event_handler_down(lv_event_t *e);
static void event_handler_left(lv_event_t *e);
static void event_handler_play(lv_event_t *e);
static void event_handler_pause(lv_event_t *e);
static void event_handler_stop(lv_event_t *e);
static void event_handler_right(lv_event_t *e);

static void navbar_foreground(bool left_en, bool right_en);
static void navbar_background(bool left_en, bool right_en);
static void setting_create(lv_obj_t *parent);
static void setting_delete(lv_obj_t *parent);
/*! @note: 主页界面 */
typedef void (*lv_interface_cb_t)(lv_obj_t *d);
static uint8_t g_interface_id = 0; // 界面id默认从0开始
enum
{
  IF_USER_OPERATION = 0,
  IF_USER_ADJUSTMENT,
  IF_USER_CURVE,
  IF_USER_RECORD,
  IF_USER_MAX
};
// static lv_interface_cb_t g_user_interface[] = {operation_create, adjustment_create, curve_create,
//                                                record_create};
static lv_interface_cb_t g_user_interface[] = {operation_create};
// static lv_interface_cb_t g_user_interface_del[] = {operation_delete, adjustment_delete,
//                                                    curve_delete, record_delete};
static lv_interface_cb_t g_user_interface_del[] = {operation_delete};
static char *user_if_head_dsc[] = {"作业界面", "倾角界面", "曲线界面", "作业记录界面"};
static lv_obj_t *left_btn;
static lv_obj_t *right_btn;

static lv_obj_t *gnet_dsc_label;
static lv_obj_t *g_gnet_label;
static lv_obj_t *g_sim_con_label;
static lv_obj_t *g_net_con_label;
static lv_obj_t *g_server_con_label;
static lv_obj_t *g_gps1_csq_label;
static lv_obj_t *g_gps2_csq_label;
static lv_timer_t *g_sys_status_icon_timer;

static lv_obj_t *current_bar;
static lv_obj_t *current_value;
// static lv_obj_t *depth_bar;
// static lv_obj_t *depth_value;

/*! @note: 作业界面 */
static lv_obj_t *g_measurements_label[8]; // 6个测量值lable
static char *g_measurements_dsc[8] = {
    "速度(m/min)", "深度(m)", "1#瞬时流量(L/min)","2#瞬时流量(L/min)",
    "1#10CM流量(L)", "2#10CM流量(L)", "1#累计流量(L)", "2#累计流量(L)"}; // 6个测量值的描述
static lv_style_t bar_style;

static lv_style_t g_measurements_lable_style;
static lv_style_t g_measurements_dsc_style;
static lv_obj_t *g_state_green_led;
static lv_style_t g_state_green_led_style;
static lv_obj_t *user_if_head;
static lv_obj_t *operation_bk_area;
static lv_timer_t *g_measurements_timer;
static lv_obj_t *g_stop_btn;
static lv_obj_t *g_pause_btn;
static lv_obj_t *g_play_btn;
static lv_style_t g_play_btn_style; // 默认风格(作业界面按钮)
/*! @note: 导航栏界面 */
static lv_obj_t *g_slider;
static lv_obj_t *g_light_label;
static lv_obj_t *g_light_dsc_label;
static lv_obj_t *g_left_list; // 左侧按钮区域
static lv_obj_t *g_light_lay;
static uint8_t g_light_lay_up_state = 0;   // 0：不显示
static uint8_t g_light_lay_down_state = 0; // 0：不显示
static uint8_t g_light_lay_sw_state = 0;   // 0：未弹出
static lv_obj_t *g_right_list;             // 右侧按钮区域
static lv_obj_t *g_left_list_btn[4];       // 左侧导航栏4个btn
static lv_obj_t *g_right_list_btn[4];      // 右侧导航栏4个btn
char *lbtn_icon[] = {LV_SYMBOL_LEFT, LV_SYMBOL_UP, LV_SYMBOL_DOWN,
                     LV_SYMBOL_LIST}; // 左侧默认图标集合
char *rbtn_icon[] = {LV_SYMBOL_RIGHT, LV_SYMBOL_PAUSE, LV_SYMBOL_STOP,
                     LV_SYMBOL_PLAY}; // 右侧默认图标集合

lv_event_cb_t lbtn_event[] = {event_handler_left, event_handler_up, event_handler_down,
                              event_handler_list}; // 事件触发集合
lv_event_cb_t rbtn_event[] = {event_handler_right, event_handler_pause, event_handler_stop,
                              event_handler_play};
lv_timer_t *g_left_list_bk_timer;
lv_timer_t *g_right_list_bk_timer;
static lv_obj_t *kb;
static lv_obj_t *login_mbox;
static lv_obj_t *pwd_ta;
static uint8_t g_login_msgbox_state = 0;    // 默认不显示
const char *user_login_passwd_def = "1234"; // 默认密码
/*! @note: 倾角状态界面 */
static lv_obj_t *g_zhuangdian_num;
static lv_obj_t *g_zhuangdian_num_lable;
static lv_obj_t *g_jingwei_num;
static lv_obj_t *g_zhuangdian_num_dx_lable;
static lv_obj_t *g_zhuangdian_num_dy_lable;
static lv_obj_t *g_x_degree_lable;
static lv_obj_t *g_y_degree_lable;
static lv_obj_t *g_sys_imei_lable;
static lv_obj_t *g_point_cur;
static lv_style_t g_point_cur_style;

static lv_obj_t *left_obj;
static lv_obj_t *right_obj;
lv_timer_t *g_adjustment_timer;
/*! @note: 曲线界面 */
static lv_obj_t *g_chart;
static lv_chart_series_t *g_ser1;
static lv_chart_series_t *g_ser2;
static lv_chart_cursor_t *g_cursor;
static lv_obj_t *x_axis_tag;
static lv_obj_t *y_axis_tag;
lv_timer_t *g_curve_timer;
static lv_obj_t *line1;
/*! @note: 作业记录界面 */
static lv_obj_t *g_table_cur_obj = NULL;
static lv_obj_t *g_table_list;
static lv_style_t table_cur_style;
static lv_style_t table_def_style;
static uint32_t g_table_list_cur_row_num = 0;
static lv_obj_t *g_table;
static char *g_table_cell_dsc[7] = {"序号", "桩点号", "时间\n(hh:mm)", "深度\n(m)",
                                    "料量\n(L/min)", "夯击次数", "---\n(L)"}; // 表头信息
lv_obj_t *g_table_list_row_buf[TABLE_ROW_CNT];                                // 对象行保存缓冲区
lv_obj_t *g_table_list_col_buf[TABLE_ROW_CNT][TABLE_COL_CNT];                 // 对象列保存缓冲区

static lv_obj_t *table_list_head;
static lv_timer_t *g_table_timer;
static lv_obj_t *cal_mbox; // 日历消息对话框
static lv_obj_t *cal_mbox_dsc_lable;
static uint8_t g_cal_mbox_state = 0; // 默认关闭
static lv_obj_t *calendar;           //
/*! @note: 设置界面 */
static lv_obj_t *g_setting_title_head;
static uint8_t g_setting_if_state = 0; // 默认不显示

/*! @note: 设置界面 */
#define SETTING_X_OFFSET 3
#define SETTING_Y_OFFSET 3
#define SETTING_CTLWID_AREA_WIDTH (MONITOR_WIDTH)
#define SETTING_CTLWID_AREA_HEIGHT (MONITOR_HEIGHT - (TABVIEW_TAB_H * 2))

#define SETTING_BTN_HEIGHT 100
#define SETTING_BTN_WIDTH 140

static char *g_setting_btn_icon[] = {ICON_SET_DEPTH_24, ICON_SET_FLOW_24, ICON_SET_CAL_24,
                                     ICON_SET_SYS_24, ICON_SET_USER_24, ICON_SET_ABOUT_24};
static char *g_setting_btn_dsc[] = {"深度计", "流量计", "校准",
                                    "系统", "用户", "关于"}; // 菜单名称

static lv_obj_t *g_depth_reporting_spinbox;
static lv_obj_t *g_slist_config_spinbox;
/**********************
 *      FUNCTION
 **********************/
/*! @note: 通用界面 */
static lv_obj_t *g_msg_mbox;
static uint8_t g_msg_mbox_state; // 窗口是否打开
/*! @note:info消息窗口事件处理
 */
// static void info_msg_event_cb(lv_event_t *e)
// {
//   lv_obj_t *obj_c = lv_event_get_current_target(e);
//   LV_LOG_USER("Button %s clicked", lv_msgbox_get_active_btn_text(obj_c));
//   if (!strcmp(lv_msgbox_get_active_btn_text(obj_c), "Close"))
//   {
//     if (g_msg_mbox != NULL)
//     {
//       lv_msgbox_close(g_msg_mbox);
//       g_msg_mbox = NULL;
//       g_msg_mbox_state = 0;
//     }
//   }
// }
// /*! @note:创建info消息窗口
//  */
// static void create_info_msgbox(const char *info, const char *mbox_btns, bool add_close_btn)
// {
//   if (mbox_btns == NULL)
//     return;
//   g_msg_mbox = lv_msgbox_create(lv_scr_act(), "Message", "Input passwd is error, please check.",
//                                 mbox_btns, add_close_btn);
//   lv_obj_add_event_cb(g_msg_mbox, info_msg_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
//   lv_obj_center(g_msg_mbox);
//   g_msg_mbox_state = 1;
//   navbar_foreground(true, true);
// }
// /*! @note:关闭info消息窗口
//  */
static void close_info_msgbox(void)
{
  if (g_msg_mbox != NULL)
  {
    lv_msgbox_close(g_msg_mbox);
    g_msg_mbox = NULL;
    g_msg_mbox_state = 0;
  }
}
/*! @note: 主页界面 */
/*! @note: 作业界面 */
char speed_buf[12] = {0};
char depth_buf[12] = {0};
char current_buf[12] = {0};
char flow_ss1_buf[12] = {0};
char flow_ss2_buf[12] = {0};
char flow_lj1_buf[12] = {0};
char flow_lj2_buf[12] = {0};
char flow_10cm_1_buf[12] = {0};
char flow_10cm_2_buf[12] = {0};
static void measurements_timer_cb(lv_timer_t *e)
{
  // lv_obj_t *slider = lv_event_get_target(e);
  // srand((unsigned int)time(NULL));
#if !USE_SDL_SIM
  // char buf[12] = {0};
  // static user_data_t g_ui_user_data; // 界面接收数据
  // memset(buf, 0, 12);
  if (g_ui_dis_data.speed != g_ui_user_data.speed)
  {
    g_ui_dis_data.speed = g_ui_user_data.speed;
    sprintf(speed_buf, "%.1f", g_ui_user_data.speed / 10.0);
    if (g_measurements_label[MEA_SPEED_I] != NULL)
      lv_label_set_text_static(g_measurements_label[MEA_SPEED_I], speed_buf);
  }
  if (g_ui_dis_data.depth != g_ui_user_data.depth)
  {
    g_ui_dis_data.depth = g_ui_user_data.depth;
    // if (depth_bar != NULL)
    // {
    //   lv_bar_set_value(depth_bar, g_ui_user_data.depth / 10, LV_ANIM_OFF);
    // }
    sprintf(depth_buf, "%.2f", (g_ui_user_data.depth / 10) / 100.0);
    // if(depth_value != NULL){
    //   lv_label_set_text_static(depth_value, depth_buf);
    // }
    if (g_measurements_label[MEA_HOLE_DEPTH_I] != NULL)
      lv_label_set_text_static(g_measurements_label[MEA_HOLE_DEPTH_I], depth_buf);
  }
  if(g_ui_dis_data.Ia != g_ui_user_data.Ia)
  {
    g_ui_dis_data.Ia = g_ui_user_data.Ia;
    if (current_bar != NULL)
    {
      lv_bar_set_value(current_bar, g_ui_user_data.Ia, LV_ANIM_OFF);
    }
    sprintf(current_buf, "%.2f", g_ui_user_data.Ia / 10.0);
    if(current_value != NULL){
      lv_label_set_text_static(current_value, current_buf);
    }
    // if (g_measurements_label[MEA_CURRENT_I] != NULL)
    //   lv_label_set_text_static(g_measurements_label[MEA_CURRENT_I], current_buf);
  }
  if(g_ui_dis_data.ss_1 != g_ui_user_data.ss_1)
  {
    g_ui_dis_data.ss_1 = g_ui_user_data.ss_1;
    sprintf(flow_ss1_buf, "%.02f", g_ui_user_data.ss_1 / 100.0); // 1通道瞬时流量
    if (g_measurements_label[FLOW_SS1_LABLE] != NULL)
      lv_label_set_text_static(g_measurements_label[FLOW_SS1_LABLE], flow_ss1_buf);
  }
  if(g_ui_dis_data.ss_2 != g_ui_user_data.ss_2)
  {
    g_ui_dis_data.ss_2 = g_ui_user_data.ss_2;
    sprintf(flow_ss2_buf, "%.02f", g_ui_user_data.ss_2 / 100.0); // 2通道瞬时流量
    if (g_measurements_label[FLOW_SS2_LABLE] != NULL)
      lv_label_set_text_static(g_measurements_label[FLOW_SS2_LABLE], flow_ss2_buf);
  }
  if(g_ui_dis_data.flow_10cm_1 != g_ui_user_data.flow_10cm_1)
  {
    g_ui_dis_data.flow_10cm_1 = g_ui_user_data.flow_10cm_1;
    sprintf(flow_10cm_1_buf, "%.2f", g_ui_user_data.flow_10cm_1 / 100.0);
    if (g_measurements_label[FLOW_10CM1_LABLE] != NULL)
      lv_label_set_text_static(g_measurements_label[FLOW_10CM1_LABLE], flow_10cm_1_buf);
  }
  if(g_ui_dis_data.flow_10cm_2 != g_ui_user_data.flow_10cm_2)
  {
    g_ui_dis_data.flow_10cm_2 = g_ui_user_data.flow_10cm_2;
    sprintf(flow_10cm_2_buf, "%.2f", g_ui_user_data.flow_10cm_2 / 100.0);
    if (g_measurements_label[FLOW_10CM2_LABLE] != NULL)
      lv_label_set_text_static(g_measurements_label[FLOW_10CM2_LABLE], flow_10cm_2_buf);
  }
  if(g_ui_dis_data.ll_1 != g_ui_user_data.ll_1)
  {
    g_ui_dis_data.ll_1 = g_ui_user_data.ll_1;
    sprintf(flow_lj1_buf, "%.1f", g_ui_user_data.ll_1 / 100.0);
    if (g_measurements_label[FLOW_LJ1_LABLE] != NULL)
      lv_label_set_text_static(g_measurements_label[FLOW_LJ1_LABLE], flow_lj1_buf);
  }
  if(g_ui_dis_data.ll_2 != g_ui_user_data.ll_2)
  {
    g_ui_dis_data.ll_2 = g_ui_user_data.ll_2;
    sprintf(flow_lj2_buf, "%.1f", g_ui_user_data.ll_2 / 100.0);
    if (g_measurements_label[FLOW_LJ2_LABLE] != NULL)
      lv_label_set_text_static(g_measurements_label[FLOW_LJ2_LABLE], flow_lj2_buf);
  }

  // memset(&g_ui_dis_data, 0xff, sizeof(g_ui_dis_data));
#else
//  lv_label_set_text_fmt(g_measurements_label[MEA_SPEED_I], "%d", rand() % 254 + 1);
//  lv_label_set_text_fmt(g_measurements_label[MEA_HOLE_DEPTH_I], "%d", rand() % 252 + 1);
//  lv_label_set_text_fmt(g_measurements_label[MEA_1_FEED_AMOUNT_I], "%d", rand() % 250 + 1);
//  lv_label_set_text_fmt(g_measurements_label[MEA_2_FEED_AMOUNT_I], "%d", rand() % 253 + 1);
//  lv_label_set_text_fmt(g_measurements_label[MEA_1_CUM_MAT_QUA_I], "%d", rand() % 258 + 1);
//  lv_label_set_text_fmt(g_measurements_label[MEA_2_CUM_MAT_QUA_I], "%d", rand() % 255 + 1);
#endif
}

/*! @note: 导航栏界面 */
// static void light_slider_event_cb(lv_event_t *e)
// {
//   // lv_obj_t *slider = lv_event_get_target(e);
//   lv_label_set_text_fmt(g_light_dsc_label, "%d", (int)lv_slider_get_value(g_slider));
//   lv_obj_align_to(g_light_dsc_label, g_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
// }
// static void ta_event_cb(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   lv_obj_t *ta = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED)
//   {
//     /*Focus on the clicked text area*/
//     if (kb != NULL)
//       lv_keyboard_set_textarea(kb, ta);
//     LV_LOG_USER("Clicked, code:%d", code);
//   }
//   else if (code == LV_EVENT_READY)
//   {
//     LV_LOG_USER("Ready, current text: %s", lv_textarea_get_text(ta));
//     if (!strncmp(lv_textarea_get_text(ta), user_login_passwd_def, 4))
//     {
//       // 登陆成功，删除登录界面+删除tabview界面
//       // 1
//       if (login_mbox != NULL)
//         lv_msgbox_close(login_mbox); // 关闭msgbox
//       if (kb != NULL)
//         lv_obj_del(kb); // 关闭keyboard
//       g_login_msgbox_state = 0;
//       // 2
//       if (g_user_interface_del[g_interface_id] != NULL)
//         g_user_interface_del[g_interface_id](lv_scr_act());
//       // 创建设置界面 TODO:
//       // setting_create(lv_scr_act());
//     }
//     else
//     {
//       static const char *login_mbox_btns[] = {"Close", ""};
//       const char *info = "Input passwd is error, please check.";
//       if (g_msg_mbox_state == 0)
//         create_info_msgbox(info, login_mbox_btns, false);
//     }
//   }
// }

// static void login_mbox_event_cb(lv_event_t *e)
// {
//   lv_obj_t *obj_c = lv_event_get_current_target(e);
//   LV_LOG_USER("Button %s clicked", lv_msgbox_get_active_btn_text(obj_c));
// }

static void event_handler_list(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    if (g_login_msgbox_state == 1)
    { // login登录界面
      if (login_mbox != NULL)
        lv_msgbox_close(login_mbox);
      if (kb != NULL)
        lv_obj_del(kb);
      close_info_msgbox();
      g_login_msgbox_state = 0;
      return;
    }
    else if (g_cal_mbox_state == 1)
    { // 日历选择界面
      if (calendar != NULL)
        lv_obj_del(calendar);
      if (cal_mbox != NULL)
        lv_msgbox_close(cal_mbox);
      g_cal_mbox_state = 0;
      return;
    }
    if (g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    // lv_obj_t *parent = (lv_obj_t *)lv_event_get_user_data(e);
    if (g_login_msgbox_state == 0)
    {
      // 创建msgbox
      // login_mbox = lv_msgbox_create(lv_scr_act(), "Setting Login", NULL, NULL, false);
      // lv_obj_add_event_cb(login_mbox, login_mbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
      // lv_obj_align_to(login_mbox, lv_scr_act(), LV_ALIGN_TOP_MID, 0, 5);
      // /*Create a label and position it above the text box*/
      // lv_obj_t *user_label = lv_label_create(login_mbox);
      // lv_label_set_text(user_label, "user:");
      // lv_obj_set_width(user_label, lv_pct(40));
      // /*Create a label and position it above the text box*/
      // lv_obj_t *pwd_label = lv_label_create(login_mbox);
      // lv_label_set_text(pwd_label, "passwd:");
      // lv_obj_set_width(pwd_label, lv_pct(40));
      // /*Create the user box*/
      // lv_obj_t *user_ta = lv_textarea_create(login_mbox);
      // lv_textarea_set_text(user_ta, "admin");
      // lv_textarea_set_one_line(user_ta, true);
      // lv_obj_set_width(user_ta, lv_pct(40));
      // lv_obj_align_to(user_ta, login_mbox, LV_ALIGN_CENTER, 0, -(lv_pct(40) / 2));
      // lv_obj_add_event_cb(user_ta, ta_event_cb, LV_EVENT_ALL, NULL);
      // /*Create the password box*/
      // pwd_ta = lv_textarea_create(login_mbox);
      // lv_textarea_set_text(pwd_ta, "");
      // lv_textarea_set_password_mode(pwd_ta, true);
      // lv_textarea_set_one_line(pwd_ta, true);
      // lv_obj_set_width(pwd_ta, lv_pct(40));
      // lv_obj_align_to(pwd_ta, login_mbox, LV_ALIGN_CENTER, 0, (lv_pct(40) / 2));
      // lv_obj_add_event_cb(pwd_ta, ta_event_cb, LV_EVENT_ALL, NULL);
      // /*Create a keyboard*/
      // kb = lv_keyboard_create(lv_scr_act());
      // lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 2);
      // lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
      // lv_keyboard_set_textarea(kb, pwd_ta);
      // g_login_msgbox_state = 1;
      // navbar_foreground(true, true);
    }
  }
}
// static void anim_x_cb(void *var, int32_t v)
// {
//   lv_obj_set_x(var, v);
// }
// static lv_timer_t *g_up_timer;
// static void up_timer_cb(lv_timer_t *e)
// {
//   if ((g_light_lay_down_state == 0 || g_light_lay_up_state == 0) && g_light_lay_sw_state == 1)
//   {
//     lv_anim_t a;
//     lv_anim_init(&a);
//     lv_anim_set_var(&a, g_light_lay);
//     lv_anim_set_values(&a, lv_obj_get_x(g_light_lay),
//                        -(LV_LIST_BTN_WIDTH / 2 + 3) - lv_obj_get_width(g_light_lay) / 2);
//     lv_anim_set_time(&a, 500);
//     lv_anim_set_exec_cb(&a, anim_x_cb);
//     lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
//     lv_anim_start(&a);
//     g_light_lay_sw_state = 0;
//     g_light_lay_up_state = 0;
//   }
//   lv_timer_reset(g_up_timer);
// }
static void event_handler_up(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  g_light_lay_up_state = 0;
  if (code == LV_EVENT_CLICKED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 ||
        g_setting_if_state == 1)
    { // 登录消息box已显示
      return;
    }
    if (g_light_lay_sw_state == 1)
    { // 说明light界面被触发
      int32_t cur_val = lv_slider_get_value(g_slider);
      lv_slider_set_value(g_slider, cur_val + 2, LV_ANIM_ON);
      lv_label_set_text_fmt(g_light_dsc_label, "%d", (int)lv_slider_get_value(g_slider));
      lv_obj_align_to(g_light_dsc_label, g_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
      g_light_lay_up_state = 1;
    }
  }
  else if (code == LV_EVENT_LONG_PRESSED_REPEAT)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_light_lay_sw_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      int32_t cur_val = lv_slider_get_value(g_slider);
      lv_slider_set_value(g_slider, cur_val + 5, LV_ANIM_ON);
      lv_label_set_text_fmt(g_light_dsc_label, "%d", (int)lv_slider_get_value(g_slider));
      lv_obj_align_to(g_light_dsc_label, g_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
      g_light_lay_up_state = 1;
    }
  }
  // else if (code == LV_EVENT_PRESSED)
  // {
  //   if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
  //   {
  //     return;
  //   }
  //   if (g_light_lay_sw_state != 1)
  //   {
  //     lv_obj_move_foreground(g_light_lay); // 移动至前台
  //     lv_anim_t a;
  //     lv_anim_init(&a);
  //     lv_anim_set_var(&a, g_light_lay);
  //     lv_anim_set_values(&a, lv_obj_get_x(g_light_lay), LV_LIST_BTN_WIDTH / 2 + 3);
  //     lv_anim_set_time(&a, 500);
  //     lv_anim_set_exec_cb(&a, anim_x_cb);
  //     lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
  //     lv_anim_start(&a);
  //     g_up_timer = lv_timer_create(up_timer_cb, 3000, NULL);
  //     g_light_lay_sw_state = 1;
  //   }
  // }
}
static void event_handler_left(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    lv_event_send(left_btn, LV_EVENT_CLICKED, NULL);
    if (g_user_interface_del[g_interface_id] != NULL)
      g_user_interface_del[g_interface_id](lv_scr_act());
    if (g_interface_id <= 0)
    {
      g_interface_id = IF_USER_MAX - 1;
    }
    else
    {
      g_interface_id--;
    }
    if (g_user_interface[g_interface_id] != NULL)
      g_user_interface[g_interface_id](lv_scr_act());
    navbar_foreground(true, true);
    // 导航栏界面显示
    switch (g_interface_id)
    {
    case TABVIEW_ID1:
      break;
    case TABVIEW_ID2:
      break;
    case TABVIEW_ID3:
      break;
    case TABVIEW_ID4:
      break;
    }
  }
}
static void event_handler_down(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_light_lay_sw_state == 1)
    { // 说明light界面被触发
      int32_t cur_val = lv_slider_get_value(g_slider);
      lv_slider_set_value(g_slider, cur_val - 2, LV_ANIM_ON);
      lv_label_set_text_fmt(g_light_dsc_label, "%d", (int)lv_slider_get_value(g_slider));
      lv_obj_align_to(g_light_dsc_label, g_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
      g_light_lay_down_state = 1;
    }
  }
  else if (code == LV_EVENT_LONG_PRESSED_REPEAT)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_light_lay_sw_state == 1)
    {
      int32_t cur_val = lv_slider_get_value(g_slider);
      lv_slider_set_value(g_slider, cur_val - 5, LV_ANIM_ON);
      lv_label_set_text_fmt(g_light_dsc_label, "%d", (int)lv_slider_get_value(g_slider));
      lv_obj_align_to(g_light_dsc_label, g_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
      g_light_lay_down_state = 1;
    }
  }
  // else if (code == LV_EVENT_PRESSED)
  // {
  //   if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
  //   {
  //     return;
  //   }
  //   if (g_light_lay_sw_state != 1)
  //   {
  //     lv_obj_move_foreground(g_light_lay); // 移动至前台
  //     lv_anim_t a;
  //     lv_anim_init(&a);
  //     lv_anim_set_var(&a, g_light_lay);
  //     lv_anim_set_values(&a, lv_obj_get_x(g_light_lay), LV_LIST_BTN_WIDTH / 2);
  //     lv_anim_set_time(&a, 500);
  //     lv_anim_set_exec_cb(&a, anim_x_cb);
  //     lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
  //     lv_anim_start(&a);
  //     // g_up_timer = lv_timer_create(up_timer_cb, 3000, NULL);
  //     g_light_lay_sw_state = 1;
  //   }
  // }
}

// static void calendar_event_handler(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   lv_obj_t *obj_c = lv_event_get_current_target(e);
//   if (code == LV_EVENT_VALUE_CHANGED)
//   {
//     lv_calendar_date_t date;
//     if (lv_calendar_get_pressed_date(obj_c, &date))
//     {
//       LV_LOG_USER("select date: %02d.%02d.%d", date.day, date.month, date.year);
//       char date_val[32] = {0};
//       sprintf(date_val, "%d.%02d.%02d", date.year, date.month, date.day);
//       lv_label_set_text(cal_mbox_dsc_lable, date_val);
//     }
//   }
// }
// static void cal_mbox_btnm_event_handler(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_VALUE_CHANGED)
//   {
//     uint32_t id = lv_btnmatrix_get_selected_btn(obj);
//     const char *txt = lv_btnmatrix_get_btn_text(obj, id);
//     if (!strcmp(txt, "Close"))
//     {
//       if (calendar != NULL)
//       {
//         lv_obj_del(calendar);
//         calendar = NULL;
//       }
//       if (cal_mbox != NULL)
//       {
//         lv_msgbox_close(cal_mbox);
//         cal_mbox = NULL;
//       }
//       LV_LOG_USER("close cal select mbox interface\n");
//     }
//     else if (!strcmp(txt, "Apply"))
//     {
//       LV_LOG_USER("apply cal select mbox interface\n");
//     }
//   }
// }

static void event_handler_play(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_interface_id == TABVIEW_ID1)
    {
      lv_event_send(g_play_btn, LV_EVENT_CLICKED, NULL);
    }
    else if (g_interface_id == TABVIEW_ID4)
    { // 打开message box
      // 创建msgbox
      cal_mbox = lv_msgbox_create(lv_scr_act(), "Calendar Select", NULL, NULL, false);
      lv_obj_align_to(cal_mbox, lv_scr_act(), LV_ALIGN_TOP_MID, 0, 5);
      lv_obj_set_height(cal_mbox, 120);
      lv_obj_t *date_label = lv_label_create(cal_mbox);
      lv_label_set_text(date_label, "date:");
      lv_obj_set_width(date_label, lv_pct(20));
      cal_mbox_dsc_lable = lv_label_create(cal_mbox);
      lv_label_set_text(cal_mbox_dsc_lable, "0000.00.00");
      lv_obj_set_width(cal_mbox_dsc_lable, lv_pct(50));
      static const char *btnm_map[] = {"Apply", "Close", ""};
      lv_obj_t *cal_mbox_btnm = lv_btnmatrix_create(cal_mbox);
      lv_btnmatrix_set_map(cal_mbox_btnm, btnm_map);
      lv_obj_set_width(cal_mbox_btnm, lv_pct(30 * 2));
      lv_obj_set_height(cal_mbox_btnm, 20);
      // lv_obj_add_event_cb(cal_mbox_btnm, cal_mbox_btnm_event_handler, LV_EVENT_ALL, NULL);
      calendar = lv_calendar_create(lv_scr_act());
      lv_obj_set_width(calendar, MONITOR_WIDTH - LV_LIST_BTN_WIDTH * 2);
      lv_obj_align_to(calendar, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
      // lv_obj_add_event_cb(calendar, calendar_event_handler, LV_EVENT_ALL, NULL);

      lv_calendar_set_today_date(calendar, 2021, 11, 9); // 设置档期内时间
      lv_calendar_set_showed_date(calendar, 2021, 02);
      /*Highlight a few days*/
      static lv_calendar_date_t
          highlighted_days[3]; /*Only its pointer will be saved so should be static*/
      highlighted_days[0].year = 2021;
      highlighted_days[0].month = 02;
      highlighted_days[0].day = 6;
      highlighted_days[1].year = 2021;
      highlighted_days[1].month = 02;
      highlighted_days[1].day = 11;
      highlighted_days[2].year = 2022;
      highlighted_days[2].month = 02;
      highlighted_days[2].day = 22;
      lv_calendar_set_highlighted_dates(calendar, highlighted_days, 3);
#if LV_USE_CALENDAR_HEADER_DROPDOWN
      lv_calendar_header_dropdown_create(calendar);
#elif LV_USE_CALENDAR_HEADER_ARROW
      lv_calendar_header_arrow_create(calendar);
#endif
      lv_calendar_set_showed_date(calendar, 2021, 10);
      g_cal_mbox_state = 1;
      navbar_foreground(true, true);
    }
  }
  else if (code == LV_EVENT_PRESSED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_interface_id == TABVIEW_ID1)
      lv_event_send(g_play_btn, LV_EVENT_PRESSED, NULL);
  }
  else if (code == LV_EVENT_RELEASED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_interface_id == TABVIEW_ID1)
      lv_event_send(g_play_btn, LV_EVENT_RELEASED, NULL);
  }
}

static void event_handler_pause(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    if (g_setting_if_state == 1 || g_login_msgbox_state == 1 || g_cal_mbox_state == 1)
    {
      return;
    }
    {
      if (g_interface_id == TABVIEW_ID1)
        lv_event_send(g_pause_btn, LV_EVENT_CLICKED, NULL);
    }
  }
  else if (code == LV_EVENT_PRESSED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_interface_id == TABVIEW_ID1)
      lv_event_send(g_pause_btn, LV_EVENT_PRESSED, NULL);
  }
  else if (code == LV_EVENT_RELEASED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_interface_id == TABVIEW_ID1)
      lv_event_send(g_pause_btn, LV_EVENT_RELEASED, NULL);
  }
}

static void event_handler_stop(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_interface_id == TABVIEW_ID1)
      lv_event_send(g_stop_btn, LV_EVENT_CLICKED, NULL);
  }
  else if (code == LV_EVENT_PRESSED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_interface_id == TABVIEW_ID1)
      lv_event_send(g_stop_btn, LV_EVENT_PRESSED, NULL);
  }
  else if (code == LV_EVENT_RELEASED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    if (g_interface_id == TABVIEW_ID1)
      lv_event_send(g_stop_btn, LV_EVENT_RELEASED, NULL);
  }
}
static void event_handler_right(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    if (g_login_msgbox_state == 1 || g_cal_mbox_state == 1 || g_setting_if_state == 1)
    {
      return;
    }
    // 移动界面
    lv_event_send(right_btn, LV_EVENT_CLICKED, NULL);
    if (g_user_interface_del[g_interface_id] != NULL)
      g_user_interface_del[g_interface_id](lv_scr_act());
    if (g_interface_id >= IF_USER_MAX - 1)
    {
      g_interface_id = 0;
    }
    else
    {
      g_interface_id++;
    }
    if (g_user_interface[g_interface_id] != NULL)
      g_user_interface[g_interface_id](lv_scr_act()); // 创建新界面
    navbar_foreground(true, true);
    // 导航栏界面显示
    switch (g_interface_id)
    {
    case TABVIEW_ID1:
      break;
    case TABVIEW_ID2:
      break;
    case TABVIEW_ID3:
      break;
    case TABVIEW_ID4:
      break;
    }
  }
}

/*!
 * @note: 导航栏移动至前台
 */
static void navbar_foreground(bool left_en, bool right_en)
{
  if (g_left_list != NULL && left_en == true)
  {
    lv_obj_move_foreground(g_left_list);
    lv_obj_move_foreground(g_light_lay);
  }
  if (g_right_list != NULL && right_en == true)
  {
    lv_obj_move_foreground(g_right_list);
  }
}
/*!
 * @note: 导航栏移动至后台
 */
static void navbar_background(bool left_en, bool right_en)
{
  if (g_left_list != NULL && left_en == true)
  {
    lv_obj_move_background(g_left_list);
    lv_obj_move_background(g_light_lay);
  }
  if (g_right_list != NULL && right_en == true)
  {
    lv_obj_move_background(g_right_list);
  }
}
/*! @note: 倾角状态界面 */
/*!
 * @note: adjustment_create 调整界面 数据显示 定时刷新
 */
// static void adjustment_timer_cb(lv_timer_t *timer)
// {
//   srand((unsigned int)time(NULL));
// #define CANVAS_OFFSET_X (-2)
// #define CANVAS_OFFSET_Y (-2)
// // 值范围：x:0-90,y:0-90
// #define CANVAS_X_MAX 12
// #define CANVAS_X_MIN -12
// #define CANVAS_Y_MAX 12
// #define CANVAS_Y_MIN -12
// // x轴长度:CANVAS_WIDTH - 20,y轴长度:CANVAS_HEIGHT - 20
// #define X_0_POINT \
//   ((CANVAS_WIDTH / 2) - (CANVAS_CUR_POINT_SIZE / 2) + CANVAS_OFFSET_X) // 画布原点x坐标
// #define Y_0_POINT \
//   ((CANVAS_HEIGHT / 2) - (CANVAS_CUR_POINT_SIZE / 2) + CANVAS_OFFSET_Y) // 画布原点y坐标
// #define X_COV_VAL(v) \
//   (X_0_POINT + ((((CANVAS_WIDTH / 2) * v) / (CANVAS_X_MAX)))) // 俯仰角角度转x坐标
// #define Y_COV_VAL(v) \
//   (Y_0_POINT - ((((CANVAS_HEIGHT / 2) * v) / (CANVAS_Y_MAX)))) // 横滚角角度转y坐标
// #if !USE_SDL_SIM
//   // static user_data_t g_ui_user_data; // 界面接收数据
//   char buf[32] = {0};
//   sprintf(buf, "x:%.1f", g_ui_user_data.angle_x / 10.0);
//   lv_label_set_text(g_x_degree_lable, buf);
//   memset(buf, 0x00, 32);
//   sprintf(buf, "y:%.1f", g_ui_user_data.angle_y / 10.0);
//   lv_label_set_text(g_y_degree_lable, buf);
//   memset(buf, 0x00, 32);
//   sprintf(buf, "%d", 0);
//   lv_label_set_text(g_zhuangdian_num_lable, buf);
//   memset(buf, 0x00, 32);
//   sprintf(buf, "%.6f", g_ui_user_data.dx / 1000000.00);
//   lv_label_set_text(g_zhuangdian_num_dx_lable, buf);
//   memset(buf, 0x00, 32);
//   sprintf(buf, "%.6f", g_ui_user_data.dy / 1000000.00);
//   lv_label_set_text(g_zhuangdian_num_dy_lable, buf);
//   if (g_sys_imei_data != NULL && g_sys_imei_lable != NULL)
//   {
//     lv_label_set_text(g_sys_imei_lable, g_sys_imei_data);
//   }

//   lv_obj_set_pos(g_point_cur, X_COV_VAL(g_ui_user_data.angle_x / 100),
//                  Y_COV_VAL(g_ui_user_data.angle_y / 100)); // 更新点坐标值
// #else
//   // 更新参数
//   uint16_t i = 1;
//   char buf[32] = {0};
//   sprintf(buf, "x:%d", (rand() % (250 + i)));
//   lv_label_set_text(g_x_degree_lable, buf);
//   memset(buf, 0x00, 32);
//   sprintf(buf, "y:%d", (rand() % (255 + i)));
//   lv_label_set_text(g_y_degree_lable, buf);
//   memset(buf, 0x00, 32);
//   sprintf(buf, "%d", (rand() % (259 + i)));
//   lv_label_set_text(g_zhuangdian_num_lable, buf);
//   memset(buf, 0x00, 32);
//   sprintf(buf, "%d", (rand() % (254 + i)));
//   lv_label_set_text(g_zhuangdian_num_dx_lable, buf);
//   memset(buf, 0x00, 32);
//   sprintf(buf, "%d", (rand() % (253 + i)));
//   lv_label_set_text(g_zhuangdian_num_dy_lable, buf);
//   // 更新点位置
//   srand((int)time(NULL));
//   int x_r = lv_rand(
//       0, 10); //(rand() % (CANVAS_X_MAX - (CANVAS_X_MIN) + 1)) + (CANVAS_X_MIN);  // 随机x坐标
//   srand((int)time(NULL));
//   int y_r = lv_rand(
//       0, 10);                                                  // (rand() % (CANVAS_Y_MAX - (CANVAS_Y_MIN) + 6)) + (CANVAS_Y_MIN);  // 随机y坐标
//   lv_obj_set_pos(g_point_cur, X_COV_VAL(x_r), Y_COV_VAL(y_r)); // 更新点坐标值
// #endif
// }
/*! @note: 曲线界面 */
// static void chart_event_cb(lv_event_t *e) {
//   static int32_t last_id = -1;
//   lv_event_code_t code = lv_event_get_code(e);
//   lv_obj_t *obj = lv_event_get_target(e);

//   if (code == LV_EVENT_VALUE_CHANGED) {
//     last_id = lv_chart_get_pressed_point(obj);
//     if (last_id != LV_CHART_POINT_NONE) {
//       lv_chart_set_cursor_point(obj, g_cursor, NULL, last_id);
//     }
//   } else if (code == LV_EVENT_DRAW_PART_END) {
//     lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
//     // if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_CURSOR))
//     //   return;
//     if (dsc->p1 == NULL || dsc->p2 == NULL || dsc->p1->y != dsc->p2->y || last_id < 0)
//       return;

//     lv_coord_t *data_array = lv_chart_get_y_array(g_chart, g_ser);
//     lv_coord_t v = data_array[last_id];
//     char buf[16];
//     lv_snprintf(buf, sizeof(buf), "%d", v);

//     lv_point_t size;
//     lv_txt_get_size(&size, buf, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

//     lv_area_t a;
//     a.y2 = dsc->p1->y - 5;
//     a.y1 = a.y2 - size.y - 10;
//     a.x1 = dsc->p1->x + 10;
//     a.x2 = a.x1 + size.x + 10;

//     lv_draw_rect_dsc_t draw_rect_dsc;
//     lv_draw_rect_dsc_init(&draw_rect_dsc);
//     draw_rect_dsc.bg_color = lv_palette_main(LV_PALETTE_BLUE);
//     draw_rect_dsc.radius = 3;

//     lv_draw_rect(&a, dsc->clip_area, &draw_rect_dsc);

//     lv_draw_label_dsc_t draw_label_dsc;
//     lv_draw_label_dsc_init(&draw_label_dsc);
//     draw_label_dsc.color = lv_color_white();
//     a.x1 += 5;
//     a.x2 -= 5;
//     a.y1 += 5;
//     a.y2 -= 5;
//     lv_draw_label(&a, dsc->clip_area, &draw_label_dsc, buf, NULL);
//   }
// }
#define CURUE_DATA_SIZE CURVE_POINT_NUM
static lv_coord_t g_curue_data1[CURUE_DATA_SIZE] = {0};
#if USE_SDL_SIM
static int16_t g_curue_data1_cnt = 0;
static int8_t g_curue_data1_flag = 100;
static int16_t cnt1 = -1000; // mm
#endif

static lv_coord_t g_curue_data2[CURUE_DATA_SIZE] = {0};
#if USE_SDL_SIM
static int16_t g_curue_data2_cnt = 0;
static int8_t g_curue_data2_flag = 100;
static int16_t cnt2 = -1000; // mm
#endif
static int16_t g_curue_data1_last = -1;
static int16_t g_curue_data2_last = -1;
static uint8_t g_curue_view_enable = 0;
/*!
 * @note: adjustment_create 调整界面 数据显示 定时刷新
 */
// static void curve_timer_cb(lv_timer_t *timer)
// {
//   if (!g_curue_view_enable)
//   {
//     return;
//   }
// #if !USE_SDL_SIM
// #define CURE_DATA_DIFF_VAL 10 // 数据采集间隔10cm
//   /**
//    * @brief 转换方法
//    * |        10个负值              |           250个正值           |
//    * -100cm -90cm -80cm ++++ -10cm 0cm 10cm 20cm ++++ 2490cm 2500cm
//    * depth(mm)
//    * (depth > 0)
//    *    id={((depth/10).cm / 10(间隔值)) + 10(负值个数)} // 在数组中的id值
//    * (depth < 0)
//    *    depth ！= 100
//    *      id={10 - ((depth/10).cm)/10)/10} // 在数组中的id值
//    *    else
//    *      id=0
//    * (depth == 0)
//    *    id=10
//    */
//   uint16_t get_cur_flow1_val = lv_rand(
//       50, 60);                                     // g_ui_cure_data.flow1;  // 采集到的当前1通道数据 // for test lv_rand(50, 60);
//   int16_t get_cur_depth_mm = g_ui_cure_data.depth; // 获取到当前深度值
//   // LV_LOG_USER("cure depth data(1) depth:%d(mm), flow:%d(L)\n", get_cur_depth_mm,
//   // get_cur_flow1_val);
//   if (((get_cur_depth_mm / 10) < -100) || ((get_cur_depth_mm / 10) > 2500))
//   {
//     LV_LOG_ERROR("cure depth data(1) is error, please check. depth:%d(mm)\n", get_cur_depth_mm);
//     return;
//   }
//   // 计算id值
//   uint16_t data1_id = 0;
//   if (get_cur_depth_mm > 0)
//   {
//     data1_id = ((get_cur_depth_mm / 10) / CURE_DATA_DIFF_VAL) + 10;
//   }
//   else if (get_cur_depth_mm < 0)
//   {
//     if (get_cur_depth_mm != -1000)
//     {
//       data1_id = (10 - abs((get_cur_depth_mm / 10)) / CURE_DATA_DIFF_VAL);
//     }
//     else
//     {
//       data1_id = 0;
//     }
//   }
//   else if (get_cur_depth_mm == 0)
//   {
//     data1_id = 10;
//   }
//   if ((data1_id - g_curue_data1_last) == 0)
//   {
//     // LV_LOG_WARN("cure depth data(1) on out, no show new\n");
//     return;
//   }
//   LV_LOG_USER("cure depth data(1) id:%d, depth:%d(mm), flow:%d(L)\n", data1_id, get_cur_depth_mm,
//               get_cur_flow1_val);
//   // 矫正数据
//   if (abs(data1_id - g_curue_data1_last) > 1)
//   { // 数据采集间隔绝对值大于1情况
//     int8_t ival = abs(data1_id - g_curue_data1_last);
//     int16_t c_data_cnt = g_curue_data1_last;
//     for (uint8_t i = 1; i < ival; i++)
//     { // 只填充缺少的数据间隔点
//       if ((data1_id - g_curue_data1_last) >= 0)
//       { // 正向递增
//         if (g_chart != NULL && g_ser1 != NULL)
//           lv_chart_set_value_by_id(g_chart, g_ser1, c_data_cnt + i, get_cur_flow1_val);
//       }
//       else if ((data1_id - g_curue_data1_last) < 0)
//       { // 反向递增
//         if (g_chart != NULL && g_ser1 != NULL)
//           lv_chart_set_value_by_id(g_chart, g_ser1, c_data_cnt - i, get_cur_flow1_val);
//       }
//     }
//     // TODO: 交叉递增
//   }
//   // 绘制图像
//   if (g_chart != NULL && g_ser1 != NULL)
//   {
//     lv_chart_set_value_by_id(g_chart, g_ser1, data1_id, get_cur_flow1_val);
//     lv_point_t p_out; // 当前点坐标
//     lv_chart_get_point_pos_by_id(g_chart, g_ser1, data1_id, &p_out);
//     static lv_style_t style_line_point;
//     lv_style_init(&style_line_point);
//     lv_style_set_line_width(&style_line_point, 2);
//     lv_style_set_line_color(&style_line_point, lv_palette_main(LV_PALETTE_GREEN));
//     lv_style_set_line_rounded(&style_line_point, true);

//     lv_obj_t *line1 = lv_line_create(g_chart);
//     static lv_point_t line_points_point[2] = {0};
//     line_points_point[0].x = p_out.x - 10;
//     line_points_point[0].y = p_out.y + 200;
//     line_points_point[1].x = p_out.x - 10;
//     line_points_point[1].y = p_out.y - 200;
//     lv_line_set_points(line1, line_points_point, 2); /*Set the points*/
//     lv_obj_add_style(line1, &style_line_point, 0);
//     // lv_obj_center(line1);
//     lv_chart_refresh(g_chart);
//   }
//   g_curue_data1_last = data1_id;
//   // 矫正图像
//   if (data1_id >= CURUE_DATA_SIZE - 1)
//   {
//     // g_curue_data1_flag = -100;
//     memset(g_curue_data1, 0x00, sizeof(g_curue_data1));
//     if (g_chart != NULL && g_ser1 != NULL)
//       lv_chart_set_all_value(g_chart, g_ser1, -CURVE_Y_AXIS_MAX_VAL);
//   }
//   else if (data1_id <= 0)
//   {
//     // g_curue_data1_flag = 100;
//     memset(g_curue_data1, 0x00, sizeof(g_curue_data1));
//     if (g_chart != NULL && g_ser1 != NULL)
//       lv_chart_set_all_value(g_chart, g_ser1, -CURVE_Y_AXIS_MAX_VAL);
//   }
//   // 通道2
//   uint16_t get_cur_flow2_val = lv_rand(
//       80, 90);                             // g_ui_cure_data.flow1;  // 采集到的当前1通道数据 // for test lv_rand(80, 90)
//   get_cur_depth_mm = g_ui_cure_data.depth; // 获取到当前深度值
//   // LV_LOG_USER("cure depth data(2) depth:%d(mm), flow:%d(L)\n", get_cur_depth_mm,
//   // get_cur_flow1_val);
//   if (((get_cur_depth_mm / 10) < -100) || ((get_cur_depth_mm / 10) > 2500))
//   {
//     LV_LOG_ERROR("cure depth data(2) is error, please check. depth:%d(mm)\n", get_cur_depth_mm);
//     return;
//   }
//   // 计算id值
//   uint16_t data2_id = 0;
//   if (get_cur_depth_mm > 0)
//   {
//     data2_id = ((get_cur_depth_mm / 10) / CURE_DATA_DIFF_VAL) + 10;
//   }
//   else if (get_cur_depth_mm < 0)
//   {
//     if (get_cur_depth_mm != -1000)
//     {
//       data2_id = (10 - abs((get_cur_depth_mm / 10)) / CURE_DATA_DIFF_VAL);
//     }
//     else
//     {
//       data2_id = 0;
//     }
//   }
//   else if (get_cur_depth_mm == 0)
//   {
//     data2_id = 10;
//   }
//   if ((data2_id - g_curue_data2_last) == 0)
//   {
//     // LV_LOG_WARN("cure depth data(2) on out, no show new\n");
//     return;
//   }
//   LV_LOG_USER("cure depth data(2) id:%d, depth:%d(mm), flow:%d(L)\n", data2_id, get_cur_depth_mm,
//               get_cur_flow2_val);
//   // 矫正数据
//   if (abs(data2_id - g_curue_data2_last) > 1)
//   { // 数据采集间隔绝对值大于1情况
//     int8_t ival = abs(data2_id - g_curue_data2_last);
//     int16_t c_data_cnt = g_curue_data2_last;
//     for (uint8_t i = 1; i < ival; i++)
//     { // 只填充缺少的数据间隔点
//       if ((data2_id - g_curue_data2_last) >= 0)
//       { // 正向递增
//         if (g_chart != NULL && g_ser2 != NULL)
//           lv_chart_set_value_by_id(g_chart, g_ser2, c_data_cnt + i, get_cur_flow2_val);
//       }
//       else if ((data2_id - g_curue_data2_last) < 0)
//       { // 反向递增
//         if (g_chart != NULL && g_ser2 != NULL)
//           lv_chart_set_value_by_id(g_chart, g_ser2, c_data_cnt - i, get_cur_flow2_val);
//       }
//     }
//     // TODO: 交叉递增
//   }
//   // 绘制图像
//   if (g_chart != NULL && g_ser2 != NULL)
//   {
//     lv_chart_set_value_by_id(g_chart, g_ser2, data2_id, get_cur_flow2_val);
//     lv_chart_refresh(g_chart);
//   }
//   g_curue_data2_last = data2_id;
//   // 矫正图像
//   if (data2_id >= CURUE_DATA_SIZE - 1)
//   {
//     // g_curue_data2_flag = -100;
//     memset(g_curue_data2, 0x00, sizeof(g_curue_data2));
//     if (g_chart != NULL && g_ser2 != NULL)
//       lv_chart_set_all_value(g_chart, g_ser2, -CURVE_Y_AXIS_MAX_VAL);
//   }
//   else if (data2_id <= 0)
//   {
//     // g_curue_data2_flag = 100;
//     memset(g_curue_data2, 0x00, sizeof(g_curue_data2));
//     if (g_chart != NULL && g_ser2 != NULL)
//       lv_chart_set_all_value(g_chart, g_ser2, -CURVE_Y_AXIS_MAX_VAL);
//   }
// #else
//   /**
//    * @brief 测试部分
//    *
//    */
// #define CURE_DATA_DIFF_VAL 10 // 数据采集间隔10cm
//   /**
//    * @brief 转换方法
//    * |        10个负值              |           250个正值           |
//    * -100cm -90cm -80cm ++++ -10cm 0cm 10cm 20cm ++++ 2490cm 2500cm
//    * depth(mm)
//    * (depth > 0)
//    *    id={((depth/10).cm / 10(间隔值)) + 10(负值个数)} // 在数组中的id值
//    * (depth < 0)
//    *    depth ！= 100
//    *      id={10 - ((depth/10).cm)/10)/10} // 在数组中的id值
//    *    else
//    *      id=0
//    * (depth == 0)
//    *    id=10
//    */
//   uint16_t get_cur_flow1_val = lv_rand(50, 60); // g_ui_cure_data.flow1;  // 采集到的当前1通道数据
//   int16_t get_cur_depth_mm = cnt1;              // g_ui_cure_data.depth; // 获取到当前深度值
//   cnt1 = cnt1 + g_curue_data1_flag;
//   // LV_LOG_USER("cure depth data(1) depth:%d(mm), flow:%d(L)\n", get_cur_depth_mm,
//   // get_cur_flow1_val);
//   if (((get_cur_depth_mm / 10) < -100) || ((get_cur_depth_mm / 10) > 2500))
//   {
//     LV_LOG_USER("cure depth data(1) is error, please check. depth:%d(mm)\n", get_cur_depth_mm);
//     return;
//   }
//   // 计算id值
//   uint16_t data1_id = 0;
//   if (get_cur_depth_mm > 0)
//   {
//     data1_id = ((get_cur_depth_mm / 10) / CURE_DATA_DIFF_VAL) + 10;
//   }
//   else if (get_cur_depth_mm < 0)
//   {
//     if (get_cur_depth_mm != -1000)
//     {
//       data1_id = (10 - abs((get_cur_depth_mm / 10)) / CURE_DATA_DIFF_VAL);
//     }
//     else
//     {
//       data1_id = 0;
//     }
//   }
//   else if (get_cur_depth_mm == 0)
//   {
//     data1_id = 10;
//   }
//   if ((data1_id - g_curue_data1_last) == 0)
//   {
//     LV_LOG_USER("cure depth data(1) on out, no show new\n");
//     return;
//   }
//   LV_LOG_USER("cure depth data(1) id:%d, depth:%d(mm), flow:%d(L)\n", data1_id, get_cur_depth_mm,
//               get_cur_flow1_val);
//   // 矫正数据
//   if (abs(data1_id - g_curue_data1_last) > 1)
//   { // 数据采集间隔绝对值大于1情况
//     int8_t ival = abs(data1_id - g_curue_data1_last);
//     int16_t c_data_cnt = g_curue_data1_last;
//     for (uint8_t i = 1; i < ival; i++)
//     { // 只填充缺少的数据间隔点
//       if ((data1_id - g_curue_data1_last) >= 0)
//       { // 正向递增
//         if (g_chart != NULL && g_ser1 != NULL)
//           lv_chart_set_value_by_id(g_chart, g_ser1, c_data_cnt + i, get_cur_flow1_val);
//       }
//       else if ((data1_id - g_curue_data1_last) < 0)
//       { // 反向递增
//         if (g_chart != NULL && g_ser1 != NULL)
//           lv_chart_set_value_by_id(g_chart, g_ser1, c_data_cnt - i, get_cur_flow1_val);
//       }
//     }
//     // TODO: 交叉递增
//   }
//   // 绘制图像
//   if (g_chart != NULL && g_ser1 != NULL)
//   {
//     lv_chart_set_value_by_id(g_chart, g_ser1, data1_id, get_cur_flow1_val);
//     lv_point_t p_out; // 当前点坐标
//     lv_chart_get_point_pos_by_id(g_chart, g_ser1, data1_id, &p_out);

//     static lv_point_t line_points_point[2] = {0};
//     line_points_point[0].x = p_out.x - 12;
//     line_points_point[0].y = p_out.y + 200;
//     line_points_point[1].x = p_out.x - 12;
//     line_points_point[1].y = p_out.y - 200;
//     if (line1 != NULL)
//     {
//       lv_line_set_points(line1, line_points_point, 2); /*Set the points*/
//     }
//     lv_chart_refresh(g_chart);
//   }
//   g_curue_data1_last = data1_id;
//   // 矫正图像
//   if (data1_id >= CURUE_DATA_SIZE - 1)
//   {
//     g_curue_data1_flag = -100;
//     memset(g_curue_data1, 0x00, sizeof(g_curue_data1));
//     if (g_chart != NULL && g_ser1 != NULL)
//       lv_chart_set_all_value(g_chart, g_ser1, -CURVE_Y_AXIS_MAX_VAL);
//   }
//   else if (data1_id <= 0)
//   {
//     g_curue_data1_flag = 100;
//     memset(g_curue_data1, 0x00, sizeof(g_curue_data1));
//     if (g_chart != NULL && g_ser1 != NULL)
//       lv_chart_set_all_value(g_chart, g_ser1, -CURVE_Y_AXIS_MAX_VAL);
//   }
//   // 通道2
//   uint16_t get_cur_flow2_val = lv_rand(80, 90); // g_ui_cure_data.flow1;  // 采集到的当前1通道数据
//   get_cur_depth_mm = cnt2;                      // g_ui_cure_data.depth; // 获取到当前深度值
//   cnt2 = cnt2 + g_curue_data2_flag;
//   // LV_LOG_USER("cure depth data(2) depth:%d(mm), flow:%d(L)\n", get_cur_depth_mm,
//   // get_cur_flow1_val);
//   if (((get_cur_depth_mm / 10) < -100) || ((get_cur_depth_mm / 10) > 2500))
//   {
//     LV_LOG_USER("cure depth data(2) is error, please check. depth:%d(mm)\n", get_cur_depth_mm);
//     return;
//   }
//   // 计算id值
//   uint16_t data2_id = 0;
//   if (get_cur_depth_mm > 0)
//   {
//     data2_id = ((get_cur_depth_mm / 10) / CURE_DATA_DIFF_VAL) + 10;
//   }
//   else if (get_cur_depth_mm < 0)
//   {
//     if (get_cur_depth_mm != -1000)
//     {
//       data2_id = (10 - abs((get_cur_depth_mm / 10)) / CURE_DATA_DIFF_VAL);
//     }
//     else
//     {
//       data2_id = 0;
//     }
//   }
//   else if (get_cur_depth_mm == 0)
//   {
//     data2_id = 10;
//   }
//   if ((data2_id - g_curue_data2_last) == 0)
//   {
//     LV_LOG_USER("cure depth data(2) on out, no show new\n");
//     return;
//   }
//   LV_LOG_USER("cure depth data(2) id:%d, depth:%d(mm), flow:%d(L)\n", data2_id, get_cur_depth_mm,
//               get_cur_flow2_val);
//   // 矫正数据
//   if (abs(data2_id - g_curue_data2_last) > 1)
//   { // 数据采集间隔绝对值大于1情况
//     int8_t ival = abs(data2_id - g_curue_data2_last);
//     int16_t c_data_cnt = g_curue_data2_last;
//     for (uint8_t i = 1; i < ival; i++)
//     { // 只填充缺少的数据间隔点
//       if ((data2_id - g_curue_data2_last) >= 0)
//       { // 正向递增
//         if (g_chart != NULL && g_ser2 != NULL)
//           lv_chart_set_value_by_id(g_chart, g_ser2, c_data_cnt + i, get_cur_flow2_val);
//       }
//       else if ((data2_id - g_curue_data2_last) < 0)
//       { // 反向递增
//         if (g_chart != NULL && g_ser2 != NULL)
//           lv_chart_set_value_by_id(g_chart, g_ser2, c_data_cnt - i, get_cur_flow2_val);
//       }
//     }
//     // TODO: 交叉递增
//   }
//   // 绘制图像
//   if (g_chart != NULL && g_ser2 != NULL)
//   {
//     lv_chart_set_value_by_id(g_chart, g_ser2, data2_id, get_cur_flow2_val);
//     lv_chart_refresh(g_chart);
//   }
//   g_curue_data2_last = data2_id;
//   // 矫正图像
//   if (data2_id >= CURUE_DATA_SIZE - 1)
//   {
//     g_curue_data2_flag = -100;
//     memset(g_curue_data2, 0x00, sizeof(g_curue_data2));
//     if (g_chart != NULL && g_ser2 != NULL)
//       lv_chart_set_all_value(g_chart, g_ser2, -CURVE_Y_AXIS_MAX_VAL);
//   }
//   else if (data2_id <= 0)
//   {
//     g_curue_data2_flag = 100;
//     memset(g_curue_data2, 0x00, sizeof(g_curue_data2));
//     if (g_chart != NULL && g_ser2 != NULL)
//       lv_chart_set_all_value(g_chart, g_ser2, -CURVE_Y_AXIS_MAX_VAL);
//   }
// #endif
// }

#if !USE_SDL_SIM
// static void data_config_timer_cb(lv_timer_t *timer)
// { // 前后台配置数据同步
//   if (g_ui_cfg_data == NULL)
//     return;
//   // TODO: 互斥锁上锁
//   g_ui_cfg_data->dir = g_sys_cfg.setting_depth_dir;
//   g_ui_cfg_data->encode_mode = g_sys_cfg.setting_depth_mode;
//   g_ui_cfg_data->enc_threshold = g_sys_cfg.setting_depth_threshold;
//   g_ui_cfg_data->cof = g_sys_cfg.setting_depth_coefficient;
//   g_ui_cfg_data->traffic_mode = g_sys_cfg.setting_flow_mode;
//   g_ui_cfg_data->encode_pluse_dir_pin_select = g_sys_cfg.setting_flow_pin;
//   // TODO: 互斥锁解锁
// }
// 系统状态图标同步
static void sys_status_icon_timer_cb(lv_timer_t *timer)
{
  if (g_sys_data != NULL && left_btn != NULL)
  {
    // SIM卡连接状态
    if (g_sim_con_label != NULL)
    {
      switch (g_sys_data->snts.sim_state)
      {
      case LSAPI_SIM_ABSENT:
        lv_label_set_text_static(g_sim_con_label, ICON_SIM_OFF_STATE_24);
        break;
      case LSAPI_SIM_NORMAL:
        lv_label_set_text_static(g_sim_con_label, ICON_SIM_ON_STATE_24);
        lv_obj_align_to(g_sim_con_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 2, -100);
        break;
      case LSAPI_SIM_ABNORMAL:
        lv_label_set_text_static(g_sim_con_label, ICON_SIM_WARN_STATE_24);
        break;
      default:
        lv_label_set_text_static(g_sim_con_label, ICON_SIM_WARN_STATE_24);
        break;
      }
    }
    // 4G信号强度
    if (g_gnet_label != NULL && gnet_dsc_label != NULL)
    {
      if (g_sys_data->snts.sim_state == LSAPI_SIM_NORMAL)
      { // SIM卡正常的情况下才会显示信号强度
        lv_obj_align_to(gnet_dsc_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
        lv_obj_align_to(g_gnet_label, gnet_dsc_label, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
      }
      else
      {
        lv_obj_align_to(gnet_dsc_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 0, -100);
        lv_obj_align_to(g_gnet_label, gnet_dsc_label, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
      }
      if (g_sys_data->snts.csq <= 2)
      {
        lv_label_set_text_static(g_gnet_label, ICON_GNET_CSQ_0_24);
      }
      else if (g_sys_data->snts.csq > 2 && g_sys_data->snts.csq <= 30)
      {
        lv_label_set_text_static(g_gnet_label, ICON_GNET_CSQ_1_24);
      }
      else if (g_sys_data->snts.csq > 30 && g_sys_data->snts.csq <= 51)
      {
        lv_label_set_text_static(g_gnet_label, ICON_GNET_CSQ_2_24);
      }
      else if (g_sys_data->snts.csq > 51 && g_sys_data->snts.csq <= 109)
      {
        lv_label_set_text_static(g_gnet_label, ICON_GNET_CSQ_3_24);
      }
      else if (g_sys_data->snts.csq > 109)
      {
        lv_label_set_text_static(g_gnet_label, ICON_GNET_CSQ_4_24);
      }
      else
      {
        lv_label_set_text_static(g_gnet_label, ICON_GNET_CSQ_0_24);
      }
    }
    // 网络连接状态
    if (g_net_con_label != NULL)
    {
      if (g_sys_data->snts.att_state == 1)
      {
        lv_label_set_text_static(g_net_con_label, ICON_NET_ON_STATE_24);
      }
      else if (g_sys_data->snts.att_state == 0)
      {
        lv_label_set_text_static(g_net_con_label, ICON_NET_OFF_STATE_24);
      }
      else
      {
        lv_label_set_text_static(g_net_con_label, ICON_NET_WARN_STATE_24);
      }
      lv_obj_align_to(g_net_con_label, g_gnet_label, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
    }
    // 服务器连接状态
    if (g_server_con_label != NULL)
    {
      if (g_sys_data->snts.server_state == SYS_STA_SER_LOGIN_FAILED)
      {
        lv_label_set_text_static(g_server_con_label, ""); // 没有登录成功的状态时都不显示图标
      }
      else
      { // 登陆成功时才显示图标并且判断是否通信数据
        if (g_sys_data->snts.server_state == SYS_STA_SER_UPLOAD_DATA)
        {
          lv_label_set_text_static(g_server_con_label, ICON_SERVER_CON_UPLOAD_STATE_24);
        }
        else if (g_sys_data->snts.server_state == SYS_STA_SER_DOWNLOAD_DATA)
        {
          lv_label_set_text_static(g_server_con_label, ICON_SERVER_CON_DOWNLOAD_STATE_24);
        }
        else if (g_sys_data->snts.server_state == SYS_STA_SER_UPLOAD_COMPLETE_DATA)
        {
          lv_label_set_text_static(g_server_con_label, ICON_SERVER_CON_NO_STATE_24);
        }
        else if (g_sys_data->snts.server_state == SYS_STA_SER_DOWNLOAD_COMPLETE_DATA)
        {
          lv_label_set_text_static(g_server_con_label, ICON_SERVER_CON_NO_STATE_24);
        }
        else
        {
          lv_label_set_text_static(g_server_con_label, ICON_SERVER_CON_NO_STATE_24);
        }
      }
      lv_obj_align_to(g_server_con_label, g_net_con_label, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
    }
    // gps1信号强度
    /* 映射关系: 0-黑色,4-绿色,5-蓝色,other-黄色 */
    if (g_gps1_csq_label != NULL)
    {
      switch (g_sys_data->sgs1.con_state)
      {
      case 0:
        lv_obj_set_style_text_color(g_gps1_csq_label, lv_color_hex(THEME_COLOR_BLACK_DEF), 0);
        break;
      case 5:
        lv_obj_set_style_text_color(g_gps1_csq_label, lv_color_hex(THEME_COLOR_BTN_BLUE_DEF), 0);
        break;
      case 4:
        lv_obj_set_style_text_color(g_gps1_csq_label, lv_color_hex(THEME_COLOR_SEAGREEN2_DEF), 0);
        break;
      default:
        lv_obj_set_style_text_color(g_gps1_csq_label, lv_color_hex(THEME_COLOR_GOLD3_DEF), 0);
        break;
      }
    }
    // gps2信号强度
    if (g_gps2_csq_label != NULL)
    {
      switch (g_sys_data->sgs2.con_state)
      {
      case 0:
        lv_obj_set_style_text_color(g_gps2_csq_label, lv_color_hex(THEME_COLOR_BLACK_DEF), 0);
        break;
      case 5:
        lv_obj_set_style_text_color(g_gps2_csq_label, lv_color_hex(THEME_COLOR_BTN_BLUE_DEF), 0);
        break;
      case 4:
        lv_obj_set_style_text_color(g_gps2_csq_label, lv_color_hex(THEME_COLOR_SEAGREEN2_DEF), 0);
        break;
      default:
        lv_obj_set_style_text_color(g_gps2_csq_label, lv_color_hex(THEME_COLOR_GOLD3_DEF), 0);
        break;
      }
    }
  }
}
#endif

/*! @note: 作业记录界面 */

/********************************************************************主页界面******************************************************************/
/*!
 * @function: demo main
 * @note:
 */
void lv_demo_mcxa(void)
{
  //QL_LOG(QL_LOG_LEVEL_INFO,"lvgl","enter lv_demo_mcxa");
  // TODO: 从配置文件中读取
  memset(&g_sys_cfg, 0x00, sizeof(g_sys_cfg)); // 当前对配置初始化为0
  memset(&g_ui_dis_data,0xff,sizeof(g_ui_dis_data)); // 当前显示数据初始化为0xff
  g_sys_cfg.setting_theme_mode = LV_THEME_DEFAULT_DARK;
#if !USE_SDL_SIM
  memset(&g_ui_user_data, 0x00, sizeof(g_ui_user_data)); // 初始化后台数据缓冲区
#endif
  lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
#if LV_USE_THEME_DEFAULT
  lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                        g_sys_cfg.setting_theme_mode, &lv_font_montserrat_12);
#endif
  operation_create(lv_scr_act());
#if !USE_SDL_SIM
  // g_curve_timer = lv_timer_create(curve_timer_cb, 200, NULL); // 对于曲线界面需要一直启用定时器
  // lv_timer_create(data_config_timer_cb, 500, NULL);           // 前后台配置数据同步
#endif
  navbar_create(lv_scr_act()); // 导航栏不依附于任何一个界面
}
/********************************************************************导航栏界面******************************************************************/
/*!
 * @function: navbar_create 导航栏
 */
static void navbar_create(lv_obj_t *parent)
{
  // -- 添加左侧按钮区域
  g_left_list = lv_obj_create(parent);
  lv_obj_clear_flag(g_left_list, LV_OBJ_FLAG_SCROLLABLE);
  static lv_style_t g_nav_list_style;
  lv_style_init(&g_nav_list_style);
  lv_obj_add_style(g_left_list, &g_nav_list_style, 0);
#if !USE_SDL_SIM
  lv_style_set_opa(&g_nav_list_style, 0); // 设置透明度为0 // TODO: 使能
#endif
  lv_obj_set_size(g_left_list, LV_LIST_BTN_WIDTH - LV_GRID_MEM_J_OFFSET, LV_LIST_BTN_HEIGHT);
  lv_obj_align_to(g_left_list, parent, LV_ALIGN_CENTER,
                  -((MONITOR_WIDTH - LV_LIST_BTN_WIDTH * 2) / 2 + LV_LIST_MEM_J_OFFSET +
                    (LV_LIST_BTN_WIDTH - LV_GRID_MEM_J_OFFSET) / 2),
                  0);
  lv_obj_t *btn_last = NULL;
  uint32_t i;
  for (i = 0; i < 4; i++)
  {
    g_left_list_btn[i] = lv_obj_create(g_left_list);
    lv_obj_clear_flag(g_left_list_btn[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_left_list_btn[i], LV_OBJ_FLAG_CLICKABLE); // 添加clickable属性
    lv_obj_set_size(g_left_list_btn[i], LV_LIST_BTN_WIDTH - LV_GRID_MEM_J_OFFSET,
                    LV_LIST_BTN_HEIGHT / 4);
    if (i == 0)
    {
      lv_obj_align_to(g_left_list_btn[i], g_left_list, LV_ALIGN_OUT_TOP_MID, 0,
                      LV_LIST_BTN_HEIGHT / 4 + 2);
    }
    else
    {
      lv_obj_align_to(g_left_list_btn[i], btn_last, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    }
    btn_last = g_left_list_btn[i];
    if (i == 0 || i == 1 || i == 2)
    {
      lv_obj_add_flag(g_left_list_btn[i], LV_OBJ_FLAG_CHECKABLE);
    }
    lv_obj_add_event_cb(g_left_list_btn[i], lbtn_event[i], LV_EVENT_ALL, NULL); // 事件触发
  }
  // -- 添加右侧按钮区域
  g_right_list = lv_obj_create(parent);
  lv_obj_clear_flag(g_right_list, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_style(g_right_list, &g_nav_list_style, 0);
#if !USE_SDL_SIM
  lv_style_set_opa(&g_nav_list_style, 0); // TODO: 使能
#endif
  lv_obj_set_size(g_right_list, LV_LIST_BTN_WIDTH - LV_GRID_MEM_J_OFFSET, LV_LIST_BTN_HEIGHT);
  lv_obj_align_to(g_right_list, parent, LV_ALIGN_CENTER,
                  (MONITOR_WIDTH - LV_LIST_BTN_WIDTH * 2) / 2 + LV_LIST_MEM_J_OFFSET +
                      (LV_LIST_BTN_WIDTH - LV_GRID_MEM_J_OFFSET) / 2,
                  0);
  btn_last = NULL;
  for (i = 0; i < 4; i++)
  {
    g_right_list_btn[i] = lv_obj_create(g_right_list);
    lv_obj_clear_flag(g_right_list_btn[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_right_list_btn[i], LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(g_right_list_btn[i], LV_LIST_BTN_WIDTH - LV_GRID_MEM_J_OFFSET,
                    LV_LIST_BTN_HEIGHT / 4);
    if (i == 0)
    {
      lv_obj_align_to(g_right_list_btn[i], g_right_list, LV_ALIGN_OUT_TOP_MID, 0,
                      LV_LIST_BTN_HEIGHT / 4 + 2);
    }
    else
    {
      lv_obj_align_to(g_right_list_btn[i], btn_last, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    }
    btn_last = g_right_list_btn[i];
    if (i == 0 || i == 1 || i == 2)
    {
      lv_obj_add_flag(g_right_list_btn[i], LV_OBJ_FLAG_CHECKABLE);
    }
    lv_obj_add_event_cb(g_right_list_btn[i], rbtn_event[i], LV_EVENT_ALL, NULL); // 事件触发
    lv_group_remove_obj(g_right_list_btn[i]);
  }
  // -- 添加亮度调节界面
  //  g_light_lay = lv_obj_create(parent);
  //  lv_obj_clear_flag(g_light_lay, LV_OBJ_FLAG_SCROLLABLE);
  //  lv_obj_set_size(g_light_lay, 40, 240);
  //  static lv_style_t g_light_lay_style;
  //  lv_style_init(&g_light_lay_style);
  //  lv_style_set_radius(&g_light_lay_style, 40 / 2);
  //  lv_obj_add_style(g_light_lay, &g_light_lay_style, 0);
  //  lv_obj_align_to(g_light_lay, g_left_list, LV_ALIGN_OUT_LEFT_MID, -20, 0);
  //  // 添加亮度slider
  //  g_slider = lv_slider_create(g_light_lay);
  //  lv_obj_set_size(g_slider, 10, 140);
  //  lv_obj_align_to(g_slider, g_light_lay, LV_ALIGN_CENTER, 0, -10);
  //  lv_obj_add_event_cb(g_slider, light_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  //  // 添加亮度图标
  //  g_light_label = lv_label_create(g_light_lay);
  //  static lv_style_t g_light_lable_style;
  //  lv_style_init(&g_light_lable_style);
  //  lv_style_set_text_font(&g_light_lable_style, &light_icon_font_20);
  //  lv_obj_add_style(g_light_label, &g_light_lable_style, 0);
  //  lv_label_set_text(g_light_label, ICON_LIGHT_20);
  //  lv_obj_align_to(g_light_label, g_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
  //  // 添加亮度值
  //  g_light_dsc_label = lv_label_create(g_light_lay);
  //  static lv_style_t g_light_dsc_lable_style;
  //  lv_style_init(&g_light_dsc_lable_style);
  //  lv_style_set_text_font(&g_light_dsc_lable_style, &lv_font_montserrat_14);
  //  lv_obj_add_style(g_light_dsc_label, &g_light_dsc_lable_style, 0);
  //  lv_label_set_text(g_light_dsc_label, "0");
  //  lv_obj_align_to(g_light_dsc_label, g_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
  //  lv_obj_move_background(g_light_lay);
}
/********************************************************************界面标识******************************************************************/
/*!
 * @note: 根据界面id创建界面标识头
 */
static lv_obj_t *user_interface_title(lv_obj_t *parent, uint8_t if_id)
{
  // 创建背景区域
  lv_obj_t *uih = lv_obj_create(parent);
  if (uih == NULL)
  {
    LV_LOG_ERROR("uih is NULL, please check.");
    return NULL;
  }
  lv_obj_clear_flag(uih, LV_OBJ_FLAG_SCROLLABLE); // 禁止滚动条
  lv_obj_set_size(uih, MONITOR_WIDTH, TABVIEW_TAB_H);
  lv_obj_set_style_radius(uih, 0, 0);

  // 创建描述lable
  lv_obj_t *user_if_head_label = lv_label_create(uih);
  if (user_if_head_label == NULL)
  {
    LV_LOG_ERROR("user_if_head_label is NULL, please check.");
    return NULL;
  }
  static lv_style_t user_if_head_label_style; // 必须是static属性
  lv_style_init(&user_if_head_label_style);
  lv_style_set_text_font(&user_if_head_label_style, &user_interface_font_16);
  lv_obj_add_style(user_if_head_label, &user_if_head_label_style, 0);
  lv_label_set_text(user_if_head_label, user_if_head_dsc[if_id]);
  lv_obj_center(user_if_head_label);

  // 创建左按钮
  left_btn = lv_obj_create(uih);
  // left_btn = lv_btn_create(uih);
  static lv_style_t uih_left_btn_style;
  lv_style_init(&uih_left_btn_style);
  lv_style_set_text_font(&uih_left_btn_style, &lv_font_montserrat_16);
  lv_style_set_bg_color(&uih_left_btn_style, lv_color_hex(THEME_COLOR_BTN_BLUE_DEF));
  lv_style_set_text_color(&uih_left_btn_style, lv_color_hex(THEME_COLOR_WHITE_DEF));
  lv_obj_add_style(left_btn, &uih_left_btn_style, 0);
  lv_obj_set_size(left_btn, TABVIEW_TAB_H, TABVIEW_TAB_H);
  lv_obj_align_to(left_btn, uih, LV_ALIGN_OUT_LEFT_MID, (TABVIEW_TAB_H), 0);
  lv_obj_set_style_radius(left_btn, 0, 0);
  lv_obj_t *left_btn_dsc_label = lv_label_create(left_btn);
  lv_obj_add_style(left_btn_dsc_label, &uih_left_btn_style, 0); // 设置描述lable样式与left_btn一样
  lv_label_set_text(left_btn_dsc_label, LV_SYMBOL_LEFT);
  lv_obj_center(left_btn_dsc_label);
  lv_obj_clear_flag(left_btn, LV_OBJ_FLAG_SCROLLABLE);
  // 创建4G信号强度
  static lv_style_t sys_status_dsc_style;
  lv_style_init(&sys_status_dsc_style);
  lv_style_set_text_font(&sys_status_dsc_style, &lv_font_montserrat_12);
  static lv_style_t sys_status_style;
  lv_style_init(&sys_status_style);
  lv_style_set_text_font(&sys_status_style, &system_status_icon_24);
  // 创建SIM卡连接状态
  g_sim_con_label = lv_label_create(uih);
  lv_obj_add_style(g_sim_con_label, &sys_status_style, 0);
  lv_obj_align_to(g_sim_con_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
  /**
   typedef enum {
      LSAPI_SIM_ABSENT = 0x00, // Sim Status is SIM_ABSENT
      LSAPI_SIM_NORMAL = 0x01, // Sim Status is SIM_NORMAL
      LSAPI_SIM_TEST = 0x02, // Sim Status is SIM_TEST
      LSAPI_SIM_ABNORMAL = 0x03, // Sim Status is SIM_ABNORMAL
      LSAPI_SIM_STATUS_END = 0x04, // Sim Status is SIM_STATUS_END
      LSAPI_SIM_TYPE_SOFT = 0x05, // Sim Status is SIM_TYPE_SOFT
      LSAPI_SIM_STATUS_ENUM_FILL = 0x7fffffff // Sim Status is SIM_STATUS_ENUM_FILL
    } LSAPI_SIM_STATUS;
   */
#if !USE_SDL_SIM
  if (g_sys_data != NULL)
  {
    switch (g_sys_data->snts.sim_state)
    {
    case LSAPI_SIM_ABSENT:
      lv_label_set_text(g_sim_con_label, ICON_SIM_OFF_STATE_24);
      lv_obj_align_to(g_sim_con_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
      break;
    case LSAPI_SIM_NORMAL:
      lv_label_set_text(g_sim_con_label, ICON_SIM_ON_STATE_24);
      // lv_obj_del(g_sim_con_label);
      lv_obj_align_to(g_sim_con_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 2, -100);
      break;
    case LSAPI_SIM_ABNORMAL:
      lv_label_set_text(g_sim_con_label, ICON_SIM_WARN_STATE_24);
      lv_obj_align_to(g_sim_con_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
      break;
    default:
      lv_label_set_text(g_sim_con_label, ICON_SIM_WARN_STATE_24);
      lv_obj_align_to(g_sim_con_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
      break;
    }
  }
  else
  {
    lv_label_set_text(g_sim_con_label, ICON_SIM_OFF_STATE_24);
  }
#else
  lv_label_set_text(g_sim_con_label, ICON_SIM_WARN_STATE_24);
#endif
  // 4G网络信号强度
  gnet_dsc_label = lv_label_create(uih);
  lv_obj_add_style(gnet_dsc_label, &sys_status_dsc_style, 0);
  lv_label_set_text(gnet_dsc_label, "4G");
  g_gnet_label = lv_label_create(uih);
  lv_obj_add_style(g_gnet_label, &sys_status_style, 0);
#if !USE_SDL_SIM
  if (g_sys_data != NULL)
  {
    if (g_sys_data->snts.sim_state == LSAPI_SIM_NORMAL)
    { // SIM卡正常的情况下才会显示信号强度
      lv_obj_align_to(gnet_dsc_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
      lv_obj_align_to(g_gnet_label, gnet_dsc_label, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    }
    else
    {
      lv_obj_align_to(gnet_dsc_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 0, -100);
      lv_obj_align_to(g_gnet_label, gnet_dsc_label, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    }
  }
  else
  {
    lv_obj_align_to(gnet_dsc_label, left_btn, LV_ALIGN_OUT_RIGHT_MID, 0, -100);
    lv_obj_align_to(g_gnet_label, gnet_dsc_label, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  }
  if (g_sys_data != NULL)
  {
    //  -113dBm + (rssi * 2)
    int cur_csq = -113 + (g_sys_data->snts.csq * 2);
    if (cur_csq <= -105)
    {
      lv_label_set_text(g_gnet_label, ICON_GNET_CSQ_0_24);
    }
    else if (cur_csq > -105 && cur_csq <= -100)
    {
      lv_label_set_text(g_gnet_label, ICON_GNET_CSQ_1_24);
    }
    else if (cur_csq > -100 && cur_csq <= -95)
    {
      lv_label_set_text(g_gnet_label, ICON_GNET_CSQ_2_24);
    }
    else if (cur_csq > -95 && cur_csq <= -90)
    {
      lv_label_set_text(g_gnet_label, ICON_GNET_CSQ_3_24);
    }
    else if (cur_csq > -85)
    {
      lv_label_set_text(g_gnet_label, ICON_GNET_CSQ_4_24);
    }
    else
    {
      lv_label_set_text(g_gnet_label, ICON_GNET_CSQ_0_24);
    }
  }
  else
  {
    lv_label_set_text(g_gnet_label, ICON_GNET_CSQ_0_24);
  }
#else
  lv_obj_align_to(g_gnet_label, gnet_dsc_label, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  lv_label_set_text(g_gnet_label, ICON_GNET_CSQ_4_24);
#endif
  // 创建网络连接状态
  g_net_con_label = lv_label_create(uih);
  lv_obj_add_style(g_net_con_label, &sys_status_style, 0);
  lv_obj_align_to(g_net_con_label, g_gnet_label, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
#if !USE_SDL_SIM
  if (g_sys_data != NULL)
  {
    if (g_sys_data->snts.att_state == 1)
    {
      lv_label_set_text(g_net_con_label, ICON_NET_ON_STATE_24);
    }
    else if (g_sys_data->snts.att_state == 0)
    {
      lv_label_set_text(g_net_con_label, ICON_NET_OFF_STATE_24);
    }
    else
    {
      lv_label_set_text(g_net_con_label, ICON_NET_WARN_STATE_24);
    }
  }
  else
  {
    lv_label_set_text(g_net_con_label, ICON_NET_OFF_STATE_24);
  }
#else
  lv_label_set_text(g_net_con_label, ICON_NET_WARN_STATE_24);
#endif
  // 创建服务器连接状态
  g_server_con_label = lv_label_create(uih);
  lv_obj_add_style(g_server_con_label, &sys_status_style, 0);
  lv_label_set_text(g_server_con_label, ICON_SERVER_CON_ON_STATE_24);
  lv_obj_align_to(g_server_con_label, g_net_con_label, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
#if !USE_SDL_SIM
  if (g_sys_data != NULL)
  {
    if (g_sys_data->snts.server_state == SYS_STA_SER_LOGIN_FAILED)
    {
      lv_label_set_text(g_server_con_label, ""); // 没有登录成功的状态时都不显示图标
    }
    else
    { // 登陆成功时才显示图标并且判断是否通信数据
      if (g_sys_data->snts.server_state == SYS_STA_SER_UPLOAD_DATA)
      {
        lv_label_set_text(g_server_con_label, ICON_SERVER_CON_UPLOAD_STATE_24);
      }
      else if (g_sys_data->snts.server_state == SYS_STA_SER_DOWNLOAD_DATA)
      {
        lv_label_set_text(g_server_con_label, ICON_SERVER_CON_DOWNLOAD_STATE_24);
      }
      else if (g_sys_data->snts.server_state == SYS_STA_SER_UPLOAD_COMPLETE_DATA)
      {
        lv_label_set_text(g_server_con_label, ICON_SERVER_CON_NO_STATE_24);
      }
      else if (g_sys_data->snts.server_state == SYS_STA_SER_DOWNLOAD_COMPLETE_DATA)
      {
        lv_label_set_text(g_server_con_label, ICON_SERVER_CON_NO_STATE_24);
      }
      else
      {
        lv_label_set_text(g_server_con_label, ICON_SERVER_CON_NO_STATE_24);
      }
    }
  }
  else
  {
    lv_label_set_text(g_server_con_label, "");
  }
#else
  lv_label_set_text(g_server_con_label, ICON_SERVER_CON_NO_STATE_24);
#endif
  // 创建右按钮
  right_btn = lv_obj_create(uih);
  // right_btn = lv_btn_create(uih);
  lv_obj_add_style(right_btn, &uih_left_btn_style, 0);
  lv_obj_set_size(right_btn, TABVIEW_TAB_H, TABVIEW_TAB_H);
  lv_obj_align_to(right_btn, uih, LV_ALIGN_OUT_RIGHT_MID, -(TABVIEW_TAB_H), 0);
  lv_obj_t *right_btn_dsc_label = lv_label_create(right_btn);
  lv_obj_add_style(right_btn_dsc_label, &uih_left_btn_style, 0);
  lv_label_set_text(right_btn_dsc_label, LV_SYMBOL_RIGHT);
  lv_obj_center(right_btn_dsc_label);
  lv_obj_clear_flag(right_btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_radius(right_btn, 0, 0);
  // 创建GPS1信号强度
  g_gps1_csq_label = lv_label_create(uih);
  lv_obj_add_style(g_gps1_csq_label, &sys_status_style, 0);
  lv_label_set_text(g_gps1_csq_label, ICON_GPS_CSQ_24);
#if !USE_SDL_SIM
  if (g_sys_data != NULL)
  {
    switch (g_sys_data->sgs1.con_state)
    {
    case 0:
      lv_obj_set_style_text_color(g_gps1_csq_label, lv_color_hex(THEME_COLOR_BLACK_DEF), 0);
      break;
    case 5:
      lv_obj_set_style_text_color(g_gps1_csq_label, lv_color_hex(THEME_COLOR_BTN_BLUE_DEF), 0);
      break;
    case 4:
      lv_obj_set_style_text_color(g_gps1_csq_label, lv_color_hex(THEME_COLOR_SEAGREEN2_DEF), 0);
      break;
    default:
      lv_obj_set_style_text_color(g_gps1_csq_label, lv_color_hex(THEME_COLOR_GOLD3_DEF), 0);
      break;
    }
  }
  else
  {
    lv_obj_set_style_text_color(g_gps1_csq_label, lv_color_hex(THEME_COLOR_BLACK_DEF), 0);
  }
#else
  lv_obj_set_style_text_color(g_gps1_csq_label, lv_color_hex(THEME_COLOR_BLACK_DEF), 0);
#endif
  lv_obj_align_to(g_gps1_csq_label, right_btn, LV_ALIGN_OUT_LEFT_MID, 0, 0);
  lv_obj_t *gps1_dsc_label = lv_label_create(uih);
  lv_obj_add_style(gps1_dsc_label, &sys_status_dsc_style, 0);
  lv_label_set_text(gps1_dsc_label, "GPS");
  lv_obj_align_to(gps1_dsc_label, g_gps1_csq_label, LV_ALIGN_OUT_LEFT_MID, -2, 4);
//   // 创建GPS2信号强度
//   g_gps2_csq_label = lv_label_create(uih);
//   lv_obj_add_style(g_gps2_csq_label, &sys_status_style, 0);
//   lv_label_set_text(g_gps2_csq_label, ICON_GPS_CSQ_24);
// #if !USE_SDL_SIM
//   if (g_sys_data != NULL) {
//     switch (g_sys_data->sgs2.con_state) {
//     case 0:
//       lv_obj_set_style_text_color(g_gps2_csq_label, lv_color_hex(THEME_COLOR_BLACK_DEF), 0);
//       break;
//     case 5:
//       lv_obj_set_style_text_color(g_gps2_csq_label, lv_color_hex(THEME_COLOR_BTN_BLUE_DEF), 0);
//       break;
//     case 4:
//       lv_obj_set_style_text_color(g_gps2_csq_label, lv_color_hex(THEME_COLOR_SEAGREEN2_DEF), 0);
//       break;
//     default:
//       lv_obj_set_style_text_color(g_gps2_csq_label, lv_color_hex(THEME_COLOR_GOLD3_DEF), 0);
//       break;
//     }
//   } else {
//     lv_obj_set_style_text_color(g_gps2_csq_label, lv_color_hex(THEME_COLOR_BLACK_DEF), 0);
//   }
// #else
//   lv_obj_set_style_text_color(g_gps2_csq_label, lv_color_hex(THEME_COLOR_BLACK_DEF), 0);
// #endif
//   lv_obj_align_to(g_gps2_csq_label, gps1_dsc_label, LV_ALIGN_OUT_LEFT_MID, -5, -4);
//   lv_obj_t *gps2_dsc_label = lv_label_create(uih);
//   lv_obj_add_style(gps2_dsc_label, &sys_status_dsc_style, 0);
//   lv_label_set_text(gps2_dsc_label, "G2");
//   lv_obj_align_to(gps2_dsc_label, g_gps2_csq_label, LV_ALIGN_OUT_LEFT_MID, -2, 4);
#if !USE_SDL_SIM
  // 创建系统状态图标同步定时器
  // g_sys_status_icon_timer = lv_timer_create(sys_status_icon_timer_cb, 1000, NULL);
#endif
  return uih;
}
/********************************************************************作业界面******************************************************************/
static void play_btn_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    LV_LOG_USER("play btn clicked");
    lv_obj_remove_style(g_state_green_led, &g_state_green_led_style, 0);
    lv_style_set_bg_color(&g_state_green_led_style, lv_color_hex(THEME_COLOR_SPRING_GREEN_DEF));
    lv_obj_add_style(g_state_green_led, &g_state_green_led_style, 0);
    g_sys_cfg.operation_start_btn = 1;
    g_sys_cfg.operation_stop_btn = 0;
    g_sys_cfg.operation_pause_btn = 0;
#if !USE_SDL_SIM
    if (thread_data != NULL)
    {
      osiEvent_t send_event;
      send_event.id = UI_EVENT_SEND_DATA;
      send_event.param1 = 12;
      send_event.param2 = 1;
      osiEventSend(thread_data, &send_event);
    }
#endif
  }
  else if (code == LV_EVENT_PRESSED)
  {
    // lv_obj_remove_style(g_play_btn, &g_play_btn_style, 0);
    // lv_style_set_bg_color(&g_play_btn_style, lv_color_hex(THEME_COLOR_GRAY81_DEF));
    // lv_obj_add_style(g_play_btn, &g_play_btn_style, 0);
  }
  else if (code == LV_EVENT_RELEASED)
  {
    // lv_obj_remove_style(g_play_btn, &g_play_btn_style, 0);
    // lv_style_set_bg_color(&g_play_btn_style, lv_color_hex(THEME_COLOR_WHITE_DEF));
    // lv_obj_add_style(g_play_btn, &g_play_btn_style, 0);
  }
}

static void stop_btn_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    LV_LOG_USER("stop btn clicked");
    lv_obj_remove_style(g_state_green_led, &g_state_green_led_style, 0);
    lv_style_set_bg_color(&g_state_green_led_style, lv_color_hex(THEME_COLOR_FIRE_BRICK1_DEF));
    lv_obj_add_style(g_state_green_led, &g_state_green_led_style, 0);

    g_sys_cfg.operation_start_btn = 0;
    g_sys_cfg.operation_stop_btn = 1;
    g_sys_cfg.operation_pause_btn = 0;
#if !USE_SDL_SIM
    if (thread_data != NULL)
    {
      osiEvent_t send_event;
      send_event.id = UI_EVENT_SEND_DATA;
      send_event.param1 = 12;
      send_event.param2 = 0;
      osiEventSend(thread_data, &send_event);
    }
#endif
  }
  else if (code == LV_EVENT_PRESSED)
  {
    // lv_obj_remove_style(g_stop_btn, &g_play_btn_style, 0);
    // lv_style_set_bg_color(&g_play_btn_style, lv_color_hex(THEME_COLOR_GRAY81_DEF));
    // lv_obj_add_style(g_stop_btn, &g_play_btn_style, 0);
  }
  else if (code == LV_EVENT_RELEASED)
  {
    // lv_obj_remove_style(g_stop_btn, &g_play_btn_style, 0);
    // lv_style_set_bg_color(&g_play_btn_style, lv_color_hex(THEME_COLOR_WHITE_DEF));
    // lv_obj_add_style(g_stop_btn, &g_play_btn_style, 0);
  }
}

static void pause_btn_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    LV_LOG_USER("pause btn clicked");
    lv_obj_remove_style(g_state_green_led, &g_state_green_led_style, 0);
    lv_style_set_bg_color(&g_state_green_led_style, lv_color_hex(THEME_COLOR_GRAY81_DEF));
    lv_obj_add_style(g_state_green_led, &g_state_green_led_style, 0);

    g_sys_cfg.operation_start_btn = 0;
    g_sys_cfg.operation_stop_btn = 0;
    g_sys_cfg.operation_pause_btn = 1;
#if !USE_SDL_SIM
    if (thread_data != NULL)
    {
      osiEvent_t send_event;
      send_event.id = UI_EVENT_PAUSE_BTN_ID;
      send_event.param1 = (uint32_t)0;
      osiEventSend(thread_data, &send_event);
    }
#endif
  }
  else if (code == LV_EVENT_PRESSED)
  {
    // lv_obj_remove_style(g_pause_btn, &g_play_btn_style, 0);
    // lv_style_set_bg_color(&g_play_btn_style, lv_color_hex(THEME_COLOR_GRAY81_DEF));
    // lv_obj_add_style(g_pause_btn, &g_play_btn_style, 0);
  }
  else if (code == LV_EVENT_RELEASED)
  {
    // lv_obj_remove_style(g_pause_btn, &g_play_btn_style, 0);
    // lv_style_set_bg_color(&g_play_btn_style, lv_color_hex(THEME_COLOR_WHITE_DEF));
    // lv_obj_add_style(g_pause_btn, &g_play_btn_style, 0);
  }
}

/*!
 * @function: operation_create 作业界面
 */
static void operation_create(lv_obj_t *parent)
{
  // 创建界面标识区域
  user_if_head = user_interface_title(parent, TABVIEW_ID1);
  // 添加测量值显示区域
#define OPER_OBJ_WIDTH (MONITOR_WIDTH - LV_GRID_MEM_X_OFFSET) // 宽度=屏幕宽度-按钮宽度
#define OPER_OBJ_HEIGHT             \
  (MONITOR_HEIGHT - TABVIEW_TAB_H - \
   LV_GRID_MEM_Y_OFFSET * 2)                     // 高度=屏幕高度-tab界面标识栏高度-2倍的y间隔
  static lv_coord_t col_dsc[] = {180, 180, 180}; // 列
  static lv_coord_t row_dsc[] = {67,67,67,67}; // 排
  operation_bk_area = lv_obj_create(parent);             // 添加背景区域
  lv_obj_clear_flag(operation_bk_area, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(operation_bk_area, OPER_OBJ_WIDTH, OPER_OBJ_HEIGHT);
  lv_obj_align_to(operation_bk_area, user_if_head, LV_ALIGN_OUT_BOTTOM_MID,
                  -LV_GRID_MEM_X_OFFSET / 2 + LV_GRID_MEM_J_OFFSET, LV_GRID_MEM_Y_OFFSET);
  struct mem_val_obj
  {
    uint8_t align_t;
    uint32_t offset_x;
    uint32_t offset_y;
  };
  struct mem_val_obj mvo[] = {
      // 各个子对象之间的相对距离
      {LV_ALIGN_OUT_TOP_LEFT, LV_GRID_MEM_J_OFFSET + 43, row_dsc[0] + LV_GRID_MEM_J_OFFSET},
      {LV_ALIGN_OUT_RIGHT_MID, LV_GRID_MEM_J_OFFSET, 0},
      {LV_ALIGN_OUT_BOTTOM_MID, 0, LV_GRID_MEM_J_OFFSET},
      {LV_ALIGN_OUT_RIGHT_MID, LV_GRID_MEM_J_OFFSET, 0},
      {LV_ALIGN_OUT_BOTTOM_MID, 0, LV_GRID_MEM_J_OFFSET},
      {LV_ALIGN_OUT_RIGHT_MID, LV_GRID_MEM_J_OFFSET, 0},
      {LV_ALIGN_OUT_BOTTOM_MID, 0, LV_GRID_MEM_J_OFFSET},
      {LV_ALIGN_OUT_RIGHT_MID, LV_GRID_MEM_J_OFFSET, 0}};
  uint32_t i;
  lv_obj_t *obj_last = NULL;
  lv_obj_t *obj_last_o = NULL;
  lv_style_init(&g_measurements_dsc_style);
  lv_style_set_text_font(&g_measurements_dsc_style, &user_interface_font_16);
  lv_style_init(&g_measurements_lable_style); // 初始化测量区域中相应部分的风格
  lv_style_set_text_font(&g_measurements_lable_style,
                         &lv_font_montserrat_46); // lv_font_montserrat_46大字体

  lv_obj_t *current_lable = lv_label_create(operation_bk_area); // 添加描述lable
  lv_label_set_text(current_lable, "电流");
  lv_obj_add_style(current_lable, &g_measurements_dsc_style, 0);

  current_value = lv_label_create(operation_bk_area); // 添加描述lable
  lv_label_set_text(current_value, "0.0");
  lv_obj_add_style(current_value, &g_measurements_dsc_style, LV_PART_INDICATOR);

  // lv_style_set_text_font(&g_play_btn_style, &lv_font_montserrat_20);

  lv_style_init(&bar_style); // 初始化状态灯风格
  lv_style_set_bg_color(&bar_style, lv_color_hex(0x000000));
  // lv_style_set_

  current_bar = lv_bar_create(operation_bk_area);
  lv_obj_set_size(current_bar, 32, OPER_OBJ_HEIGHT - 50);
  lv_bar_set_mode(current_bar, LV_BAR_MODE_RANGE);
  lv_obj_align_to(current_bar, operation_bk_area, LV_ALIGN_LEFT_MID, -12, 0);
  lv_obj_set_style_pad_all(current_bar, 0, 0);
  lv_obj_align_to(current_lable, current_bar, LV_ALIGN_OUT_TOP_MID, 0, 0);
  lv_obj_align_to(current_value, current_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
  lv_obj_add_style(current_bar, &bar_style, LV_PART_MAIN);
  lv_bar_set_range(current_bar, 0, 15000);
  lv_bar_set_value(current_bar, 0, LV_ANIM_OFF);

  // lv_obj_t *depth_lable = lv_label_create(operation_bk_area); // 添加描述lable
  // lv_label_set_text(depth_lable, "深度");
  // lv_obj_add_style(depth_lable, &g_measurements_dsc_style, 0);

  //depth_value = lv_label_create(operation_bk_area); // 添加描述lable
  //lv_label_set_text(depth_value, "0.00");
  //lv_obj_add_style(depth_value, &g_measurements_dsc_style, 0);

  // depth_bar = lv_bar_create(operation_bk_area);
  // lv_obj_set_size(depth_bar, 32, OPER_OBJ_HEIGHT - 50);
  // lv_obj_set_style_pad_all(depth_bar, 0, 0);
  // lv_bar_set_mode(depth_bar, LV_BAR_MODE_SYMMETRICAL);
  // lv_obj_align_to(depth_bar, operation_bk_area, LV_ALIGN_RIGHT_MID, 12, 0);
  // // lv_obj_center(bar1);
  // lv_bar_set_value(depth_bar, 0, LV_ANIM_OFF);
  // lv_bar_set_range(depth_bar, 0, 1200);
  // lv_obj_align_to(depth_lable, depth_bar, LV_ALIGN_OUT_TOP_MID, 0, 0);
  // lv_obj_align_to(depth_value, depth_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
  // lv_obj_add_style(depth_bar, &bar_style, 0);

  for (i = 0; i < 8; i++)
  {
    lv_obj_t *obj_i = lv_obj_create(operation_bk_area); // 添加各子部分背景区域
    lv_obj_clear_flag(obj_i, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(obj_i, col_dsc[i / 3], row_dsc[i % 3]);
    if (i == 0)
    {
      lv_obj_align_to(obj_i, operation_bk_area, mvo[i].align_t, mvo[i].offset_x, mvo[i].offset_y);
      obj_last_o = obj_i;
    }
    else if (!(i % 2))
    {
      lv_obj_align_to(obj_i, obj_last_o, mvo[i].align_t, mvo[i].offset_x, mvo[i].offset_y);
      obj_last_o = obj_i;
    }
    else
    {
      lv_obj_align_to(obj_i, obj_last, mvo[i].align_t, mvo[i].offset_x, mvo[i].offset_y);
    }
    obj_last = obj_i;
    lv_obj_t *dsc_label = lv_label_create(obj_i); // 添加描述lable
    lv_obj_add_style(dsc_label, &g_measurements_dsc_style, 0);
    lv_label_set_text(dsc_label, g_measurements_dsc[i]);
    g_measurements_label[i] = lv_label_create(obj_i); // 添加测量值lable
    lv_obj_add_style(g_measurements_label[i], &g_measurements_lable_style, 0);
    lv_obj_align(g_measurements_label[i], LV_ALIGN_CENTER, 0, 12);
    lv_obj_align(dsc_label, LV_ALIGN_TOP_MID, 0, -12);
    lv_label_set_text(g_measurements_label[i], "0");
  }
  g_measurements_timer =
      lv_timer_create(measurements_timer_cb, 100, NULL); // 添加定时器动态更新数值 100 ms
  // 添加状态灯
  lv_style_init(&g_state_green_led_style); // 初始化状态灯风格
  lv_style_set_radius(&g_state_green_led_style, 0);
  lv_style_set_bg_opa(&g_state_green_led_style, LV_OPA_COVER);

  if (g_sys_cfg.operation_pause_btn == 1)
  {
    lv_style_set_bg_color(&g_state_green_led_style,
                          lv_color_hex(THEME_COLOR_GRAY81_DEF)); // 初始化背景灰色
  }
  else if (g_sys_cfg.operation_start_btn == 1)
  {
    lv_style_set_bg_color(&g_state_green_led_style, lv_color_hex(THEME_COLOR_SPRING_GREEN_DEF));
  }
  else if (g_sys_cfg.operation_stop_btn == 1)
  {
    lv_style_set_bg_color(&g_state_green_led_style, lv_color_hex(THEME_COLOR_FIRE_BRICK1_DEF));
  }
  else
  {
    lv_style_set_bg_color(&g_state_green_led_style, lv_color_hex(THEME_COLOR_GRAY81_DEF));
  }

  g_state_green_led = lv_obj_create(parent);
  lv_obj_clear_flag(g_state_green_led, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(
      g_state_green_led,
      (LV_GRID_MEM_X_OFFSET - LV_GRID_MEM_J_OFFSET * 2),   // 宽度=剩余宽度-2倍x间隔
      ((OPER_OBJ_HEIGHT) / 4 - (LV_GRID_MEM_J_OFFSET * 5)) // 高度=将按钮和状态灯均分5份
  );
  lv_obj_add_style(g_state_green_led, &g_state_green_led_style, 0);
  lv_obj_align_to(g_state_green_led, operation_bk_area, LV_ALIGN_OUT_RIGHT_TOP,
                  LV_GRID_MEM_J_OFFSET, 3);

  lv_style_init(&g_play_btn_style); // 按钮样式
  lv_style_set_radius(&g_play_btn_style, 0);
  lv_style_set_text_font(&g_play_btn_style, &lv_font_montserrat_20);

  static lv_style_t style_btn_pressed; // 自定义按钮按压样式
  lv_style_init(&style_btn_pressed);
  lv_style_set_bg_color(&style_btn_pressed, lv_palette_lighten(LV_PALETTE_GREY, 3));

  // 添加暂停按钮
  g_pause_btn = lv_obj_create(parent);
  lv_obj_clear_flag(g_pause_btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(g_pause_btn, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_style(g_pause_btn, &g_play_btn_style, 0); // 与play相同的样式
  lv_obj_set_size(g_pause_btn, (LV_GRID_MEM_X_OFFSET - (LV_GRID_MEM_J_OFFSET * 2)),
                  (LV_GRID_MEM_X_OFFSET - (LV_GRID_MEM_J_OFFSET * 2)));
  lv_obj_align_to(g_pause_btn, g_state_green_led, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  lv_obj_add_style(g_pause_btn, &style_btn_pressed, LV_STATE_PRESSED);
  lv_obj_set_style_text_color(g_pause_btn, lv_color_hex(THEME_COLOR_BLACK_DEF), LV_STATE_PRESSED);

  lv_obj_t *g_pause_btn_dsc_label = lv_label_create(g_pause_btn);
  lv_obj_add_style(g_pause_btn_dsc_label, &g_play_btn_style, 0);
  lv_label_set_text(g_pause_btn_dsc_label, LV_SYMBOL_PAUSE);
  lv_obj_center(g_pause_btn_dsc_label);
  lv_obj_add_event_cb(g_pause_btn, pause_btn_event_cb, LV_EVENT_ALL, NULL);
  // 添加停止按钮
  g_stop_btn = lv_obj_create(parent);
  lv_obj_clear_flag(g_stop_btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(g_stop_btn, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_style(g_stop_btn, &g_play_btn_style, 0); // 与play相同的样式
  lv_obj_set_size(g_stop_btn, (LV_GRID_MEM_X_OFFSET - (LV_GRID_MEM_J_OFFSET * 2)),
                  (LV_GRID_MEM_X_OFFSET - (LV_GRID_MEM_J_OFFSET * 2)));
  lv_obj_align_to(g_stop_btn, g_pause_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
  lv_obj_add_style(g_stop_btn, &style_btn_pressed, LV_STATE_PRESSED);
  lv_obj_set_style_text_color(g_stop_btn, lv_color_hex(THEME_COLOR_BLACK_DEF), LV_STATE_PRESSED);

  lv_obj_t *g_stop_btn_dsc_label = lv_label_create(g_stop_btn);
  lv_obj_add_style(g_stop_btn_dsc_label, &g_play_btn_style, 0);
  lv_label_set_text(g_stop_btn_dsc_label, LV_SYMBOL_STOP);
  lv_obj_center(g_stop_btn_dsc_label);
  lv_obj_add_event_cb(g_stop_btn, stop_btn_event_cb, LV_EVENT_ALL, NULL);
  // 添加play按钮
  g_play_btn = lv_obj_create(parent);
  lv_obj_clear_flag(g_play_btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(g_play_btn, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_style(g_play_btn, &g_play_btn_style, 0);
  lv_obj_set_size(g_play_btn, (LV_GRID_MEM_X_OFFSET - (LV_GRID_MEM_J_OFFSET * 2)),
                  (LV_GRID_MEM_X_OFFSET - (LV_GRID_MEM_J_OFFSET * 2)));
  lv_obj_align_to(g_play_btn, g_stop_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
  lv_obj_add_style(g_play_btn, &style_btn_pressed, LV_STATE_PRESSED);
  lv_obj_set_style_text_color(g_play_btn, lv_color_hex(THEME_COLOR_BLACK_DEF), LV_STATE_PRESSED);

  lv_obj_t *g_play_btn_dsc_label = lv_label_create(g_play_btn);
  lv_obj_add_style(g_play_btn_dsc_label, &g_play_btn_style, 0);
  lv_label_set_text(g_play_btn_dsc_label, LV_SYMBOL_PLAY);
  lv_obj_center(g_play_btn_dsc_label);
  lv_obj_add_event_cb(g_play_btn, play_btn_event_cb, LV_EVENT_ALL, NULL);

  // 导航栏移至前台
  // navbar_foreground(true, true);
}

static void operation_delete(lv_obj_t *parent)
{
#if !USE_SDL_SIM
  lv_timer_del(g_sys_status_icon_timer);
#endif
  lv_timer_del(g_measurements_timer);
  lv_obj_del(user_if_head);
  lv_obj_del(operation_bk_area);
  lv_obj_del(g_state_green_led);
  lv_obj_del(g_stop_btn);
  lv_obj_del(g_pause_btn);
  lv_obj_del(g_play_btn);
}
/********************************************************************倾角状态界面******************************************************************/

/*!
 * @function: adjustment_create 调整界面
 */
// static void adjustment_create(lv_obj_t *parent)
// {
//   user_if_head = user_interface_title(parent, TABVIEW_ID2);
//   // 创建参数显示区域
//   left_obj = lv_obj_create(parent);
//   lv_obj_clear_flag(left_obj, LV_OBJ_FLAG_SCROLLABLE); // 禁止滚动条
//   lv_obj_set_size(left_obj, ADJ_LEFT_MEM_WIDTH,
//                   MONITOR_HEIGHT - TABVIEW_TAB_H - LV_GRID_MEM_J_OFFSET);
//   lv_obj_align_to(left_obj, parent, LV_ALIGN_OUT_TOP_LEFT, LV_GRID_MEM_J_OFFSET,
//                   (MONITOR_HEIGHT - TABVIEW_TAB_H - LV_GRID_MEM_Y_OFFSET) + LV_GRID_MEM_Y_OFFSET +
//                       TABVIEW_TAB_H);
//   g_zhuangdian_num = lv_obj_create(left_obj);
//   lv_obj_clear_flag(g_zhuangdian_num, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_set_size(
//       g_zhuangdian_num, ADJ_LEFT_MEM_WIDTH - LV_GRID_MEM_J_OFFSET * 2,
//       (MONITOR_HEIGHT - TABVIEW_TAB_H - LV_GRID_MEM_J_OFFSET - LV_GRID_MEM_J_OFFSET * 2) / 2);
//   lv_obj_align_to(
//       g_zhuangdian_num, left_obj, LV_ALIGN_CENTER, 0,
//       -(MONITOR_HEIGHT - TABVIEW_TAB_H - LV_GRID_MEM_J_OFFSET - LV_GRID_MEM_J_OFFSET * 2) / 4);
//   g_zhuangdian_num_lable = lv_label_create(g_zhuangdian_num);
//   lv_obj_set_style_text_font(g_zhuangdian_num_lable, &lv_font_montserrat_42, 0);
// #if !USE_SDL_SIM
//   lv_label_set_text_fmt(g_zhuangdian_num_lable, "%d",
//                         g_ui_user_data.id); // 实际使用场景，使用采集到的数据显示
// #else
//   lv_label_set_text(g_zhuangdian_num_lable, INTER_ADJUS_ZHUANGDIAN_NUM);
// #endif
//   lv_obj_align(g_zhuangdian_num_lable, LV_ALIGN_CENTER, 0, 10);

//   lv_obj_t *g_zhuangdian_num_dsc = lv_label_create(g_zhuangdian_num);
//   lv_obj_set_style_text_font(g_zhuangdian_num_dsc, &user_interface_font_16, 0);
//   lv_label_set_text(g_zhuangdian_num_dsc, INTER_ADJUS_ZHUANGDIAN_NUM_NAME);
//   lv_obj_align_to(g_zhuangdian_num_dsc, g_zhuangdian_num_lable, LV_ALIGN_OUT_TOP_MID, 0, 0);

//   g_jingwei_num = lv_obj_create(left_obj);
//   lv_obj_clear_flag(g_jingwei_num, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_set_size(
//       g_jingwei_num, ADJ_LEFT_MEM_WIDTH - LV_GRID_MEM_J_OFFSET * 2,
//       (MONITOR_HEIGHT - TABVIEW_TAB_H - LV_GRID_MEM_J_OFFSET - LV_GRID_MEM_J_OFFSET * 2) / 2);
//   lv_obj_align_to(
//       g_jingwei_num, left_obj, LV_ALIGN_CENTER, 0,
//       (MONITOR_HEIGHT - TABVIEW_TAB_H - LV_GRID_MEM_J_OFFSET - LV_GRID_MEM_J_OFFSET * 2) / 4);
//   lv_obj_t *g_zhuangdian_num_dx_dsc_lable = lv_label_create(g_jingwei_num);
//   lv_obj_set_style_text_font(g_zhuangdian_num_dx_dsc_lable, &lv_font_montserrat_20, 0);
//   lv_label_set_text(g_zhuangdian_num_dx_dsc_lable, INTER_ADJUS_DX_DSC_NUM);
//   lv_obj_align_to(g_zhuangdian_num_dx_dsc_lable, g_jingwei_num, LV_ALIGN_CENTER, 0, -22);

//   g_zhuangdian_num_dx_lable = lv_label_create(g_jingwei_num);
//   lv_obj_set_style_text_font(g_zhuangdian_num_dx_lable, &lv_font_montserrat_20, 0);
// #if !USE_SDL_SIM
//   lv_label_set_text_fmt(g_zhuangdian_num_dx_lable, "%d",
//                         g_ui_user_data.dx); // 实际使用场景，使用采集到的数据显示
// #else
//   lv_label_set_text(g_zhuangdian_num_dx_lable, INTER_ADJUS_DX_NUM);
// #endif
//   // lv_obj_align_to(g_zhuangdian_num_dx_lable, g_jingwei_num, LV_ALIGN_CENTER, 0, -2);
//   lv_obj_align_to(g_zhuangdian_num_dx_lable, g_jingwei_num, LV_ALIGN_LEFT_MID, -8, -2);

//   lv_obj_t *g_zhuangdian_num_dy_dsc_lable = lv_label_create(g_jingwei_num);
//   lv_obj_set_style_text_font(g_zhuangdian_num_dy_dsc_lable, &lv_font_montserrat_20, 0);
//   lv_label_set_text(g_zhuangdian_num_dy_dsc_lable, INTER_ADJUS_DY_DSC_NUM);
//   lv_obj_align_to(g_zhuangdian_num_dy_dsc_lable, g_jingwei_num, LV_ALIGN_CENTER, 0, 18);

//   g_zhuangdian_num_dy_lable = lv_label_create(g_jingwei_num);
//   lv_obj_set_style_text_font(g_zhuangdian_num_dy_lable, &lv_font_montserrat_20, 0);
// #if !USE_SDL_SIM
//   lv_label_set_text_fmt(g_zhuangdian_num_dy_lable, "%d",
//                         g_ui_user_data.dy); // 实际使用场景，使用采集到的数据显示
// #else
//   lv_label_set_text(g_zhuangdian_num_dy_lable, INTER_ADJUS_DY_NUM);
// #endif
//   // lv_obj_align_to(g_zhuangdian_num_dy_lable, g_jingwei_num, LV_ALIGN_CENTER, 0, 35);
//   lv_obj_align_to(g_zhuangdian_num_dy_lable, g_jingwei_num, LV_ALIGN_LEFT_MID, -8, 35);

//   lv_obj_t *g_zhuangdian_xy_num_dsc = lv_label_create(g_jingwei_num);
//   lv_obj_set_style_text_font(g_zhuangdian_xy_num_dsc, &user_interface_font_16, 0);
//   lv_label_set_text(g_zhuangdian_xy_num_dsc, INTER_ADJUS_JINGWEIDU_NUM_NAME);
//   lv_obj_align_to(g_zhuangdian_xy_num_dsc, g_jingwei_num, LV_ALIGN_TOP_MID, 0, 0);

//   right_obj = lv_obj_create(parent);
//   lv_obj_clear_flag(right_obj, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_set_style_radius(right_obj, 0, 0);
//   lv_obj_set_size(right_obj, (MONITOR_WIDTH - ADJ_LEFT_MEM_WIDTH) - LV_GRID_MEM_J_OFFSET * 2,
//                   MONITOR_HEIGHT - TABVIEW_TAB_H - LV_GRID_MEM_J_OFFSET);
//   lv_obj_align_to(right_obj, left_obj, LV_ALIGN_OUT_RIGHT_MID, LV_GRID_MEM_J_OFFSET, 0);
//   lv_obj_set_style_pad_top(right_obj, 0, 0);
//   lv_obj_set_style_pad_bottom(right_obj, 0, 0);
//   lv_obj_set_style_pad_left(right_obj, 0, 0);
//   lv_obj_set_style_pad_right(right_obj, 0, 0);
//   // 添加倾角状态图
//   // size : ((LV_COLOR_SIZE / 8) * (480 - (480 / 3)) - 3 * 2 * 320 - 30 - 3) LV_COLOR_SIZE=32

//   // static lv_color_t cbuf[LV_IMG_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];
//   // lv_obj_t *canvas = lv_canvas_create(right_obj);
//   // lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
//   // lv_obj_center(canvas);
//   // if (g_sys_cfg.setting_theme_mode == 0) {
//   //   lv_canvas_fill_bg(canvas, lv_color_hex(THEME_COLOR_WHITE_DEF), LV_OPA_COVER);
//   // } else if (g_sys_cfg.setting_theme_mode == 1) {  // 深色模式下
//   //   lv_canvas_fill_bg(canvas, lv_color_hex(THEME_COLOR_DARK_MODE_DEF), LV_OPA_COVER);
//   // }
//   // lv_draw_line_dsc_t line_dsc_x;
//   // lv_draw_line_dsc_init(&line_dsc_x);
//   // line_dsc_x.color = lv_color_hex(THEME_COLOR_SLATE_GREY_DEF);
//   // line_dsc_x.width = 2;
//   // // line_dsc_x.dash_width = 3;
//   // // line_dsc.dash_gap = 0;
//   // // line_dsc.opa = 0;
//   // // line_dsc.blend_mode = 0;
//   // // line_dsc.round_start = 0;
//   // // line_dsc.round_end = 0;
//   // // line_dsc.raw_end = 0;
//   // lv_point_t lp_x[2] = {0};
//   // lp_x[0].x = 10;
//   // lp_x[0].y = CANVAS_HEIGHT / 2;
//   // lp_x[1].x = CANVAS_WIDTH - 10;
//   // lp_x[1].y = CANVAS_HEIGHT / 2;
//   // lv_canvas_draw_line(canvas, lp_x, 2, &line_dsc_x);

//   // lv_draw_line_dsc_t line_dsc_y;
//   // lv_draw_line_dsc_init(&line_dsc_y);
//   // line_dsc_y.color = lv_color_hex(THEME_COLOR_SLATE_GREY_DEF);
//   // line_dsc_y.width = 2;
//   // lv_point_t lp_y[2] = {0};
//   // lp_y[0].x = CANVAS_WIDTH / 2;
//   // lp_y[0].y = 10;
//   // lp_y[1].x = CANVAS_WIDTH / 2;
//   // lp_y[1].y = CANVAS_HEIGHT - 10;
//   // lv_canvas_draw_line(canvas, lp_y, 2, &line_dsc_y);

//   // lv_draw_arc_dsc_t arc_dsc_1;
//   // lv_draw_arc_dsc_init(&arc_dsc_1);
//   // arc_dsc_1.color = lv_color_hex(THEME_COLOR_SLATE_GREY_DEF);  // 0x4EEE94
//   // arc_dsc_1.width = 2;
//   // lv_canvas_draw_arc(canvas, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2, CANVAS_WIDTH * 0.35 / 2, 0,
//   // 360,
//   //                    &arc_dsc_1);

//   // lv_draw_arc_dsc_t arc_dsc_2;
//   // lv_draw_arc_dsc_init(&arc_dsc_2);
//   // arc_dsc_2.color = lv_color_hex(THEME_COLOR_SLATE_GREY_DEF);  // 0xEE2C2C
//   // arc_dsc_2.width = 2;
//   // lv_canvas_draw_arc(canvas, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2, CANVAS_WIDTH * 0.7 / 2, 0, 360,
//   //                    &arc_dsc_2);

//   static lv_point_t line_points1[] = {{5, 0}, {CANVAS_WIDTH - 5, 0}};

//   /*Create style*/
//   static lv_style_t style_line;
//   lv_style_init(&style_line);
//   lv_style_set_line_width(&style_line, 2);
//   lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_BLUE));
//   lv_style_set_line_rounded(&style_line, true);

//   lv_obj_t *line1;
//   line1 = lv_line_create(right_obj);
//   lv_line_set_points(line1, line_points1, 2); /*Set the points*/
//   lv_obj_add_style(line1, &style_line, 0);
//   lv_obj_center(line1);

//   static lv_point_t line_points2[] = {{0, 10}, {0, CANVAS_HEIGHT - 10}};
//   lv_obj_t *line2;
//   line2 = lv_line_create(right_obj);
//   lv_line_set_points(line2, line_points2, 2); /*Set the points*/
//   lv_obj_add_style(line2, &style_line, 0);
//   lv_obj_align_to(line2, right_obj, LV_ALIGN_CENTER, 0, -10);

//   static lv_style_t style;
//   lv_style_init(&style);
//   lv_style_set_arc_width(&style, 2);
//   lv_style_set_bg_color(&style, lv_color_hex(THEME_COLOR_SNOW3_DEF));

//   lv_obj_t *arc_dsc_1 = lv_arc_create(right_obj);
//   lv_obj_set_size(arc_dsc_1, ((CANVAS_WIDTH * 5) / 12) * 2, ((CANVAS_WIDTH * 5) / 12) * 2);
//   lv_arc_set_rotation(arc_dsc_1, 0);
//   lv_arc_set_bg_angles(arc_dsc_1, 0, 360);
//   lv_obj_remove_style(arc_dsc_1, NULL, LV_PART_KNOB);  /*Be sure the knob is not displayed*/
//   lv_obj_clear_flag(arc_dsc_1, LV_OBJ_FLAG_CLICKABLE); /*To not allow adjusting by click*/
//   lv_obj_center(arc_dsc_1);
//   lv_arc_set_value(arc_dsc_1, 0);
//   lv_obj_add_style(arc_dsc_1, &style, 0);
//   // lv_obj_set_style_bg_color(arc_dsc_1, lv_palette_main(LV_PALETTE_BLUE),0);

//   lv_obj_t *arc_dsc_1_lable = lv_label_create(right_obj);
//   lv_obj_set_style_text_font(arc_dsc_1_lable, &lv_font_montserrat_12, 0);
//   lv_label_set_text(arc_dsc_1_lable, "10");
//   lv_obj_set_pos(arc_dsc_1_lable, 258, 228);

//   static lv_style_t style2;
//   lv_style_init(&style2);
//   lv_style_set_arc_width(&style2, 3);
//   lv_style_set_bg_color(&style2, lv_palette_main(LV_PALETTE_YELLOW));

//   lv_obj_t *arc_dsc_2 = lv_arc_create(right_obj);
//   lv_obj_set_size(arc_dsc_2, ((CANVAS_WIDTH * 5) / 24) * 2, ((CANVAS_WIDTH * 5) / 24) * 2);
//   lv_arc_set_rotation(arc_dsc_2, 0);
//   lv_arc_set_bg_angles(arc_dsc_2, 0, 360);
//   lv_obj_remove_style(arc_dsc_2, NULL, LV_PART_KNOB);  /*Be sure the knob is not displayed*/
//   lv_obj_clear_flag(arc_dsc_2, LV_OBJ_FLAG_CLICKABLE); /*To not allow adjusting by click*/
//   lv_obj_center(arc_dsc_2);
//   lv_arc_set_value(arc_dsc_2, 0);
//   lv_obj_add_style(arc_dsc_2, &style2, 0);
//   // lv_obj_set_style_bg_color(arc_dsc_2, lv_palette_main(THEME_COLOR_YELLOW_DEF),0);

//   lv_obj_t *arc_dsc_2_lable = lv_label_create(right_obj);
//   lv_obj_set_style_text_font(arc_dsc_2_lable, &lv_font_montserrat_12, 0);
//   lv_label_set_text(arc_dsc_2_lable, "5");
//   lv_obj_set_pos(arc_dsc_2_lable, 200, 190);

//   g_point_cur = lv_obj_create(right_obj);
//   lv_obj_clear_flag(g_point_cur, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_set_size(g_point_cur, CANVAS_CUR_POINT_SIZE, CANVAS_CUR_POINT_SIZE);
//   lv_style_init(&g_point_cur_style);
//   lv_style_set_radius(&g_point_cur_style, 0);
//   lv_style_set_bg_opa(&g_point_cur_style, LV_OPA_COVER);
//   lv_style_set_bg_color(&g_point_cur_style, lv_color_hex(THEME_COLOR_SEA_GREEN1_DEF));
//   lv_style_set_radius(&g_point_cur_style, 8);
//   lv_obj_add_style(g_point_cur, &g_point_cur_style, 0);
//   lv_obj_set_pos(g_point_cur, 0, 0); // 初始化0点
//   lv_obj_t *g_point_lable = lv_label_create(g_point_cur);
//   lv_obj_set_style_text_font(g_point_lable, &lv_font_montserrat_42, 0);
//   lv_label_set_text(g_point_lable, ".");
//   lv_obj_align_to(g_point_lable, g_point_cur, LV_ALIGN_CENTER, 0, -12);

//   // 名称
//   lv_obj_t *inclination_state = lv_obj_create(right_obj);
//   lv_obj_clear_flag(inclination_state, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_set_size(inclination_state, 86, 30);
//   lv_obj_align_to(inclination_state, right_obj, LV_ALIGN_TOP_RIGHT, -10, 10);
//   lv_obj_t *inclination_state_lable = lv_label_create(inclination_state);
//   lv_label_set_text(inclination_state_lable, INTER_ADJUS_INCLINATION_NAME);
//   lv_obj_set_style_text_font(inclination_state_lable, &user_interface_font_18, 0);
//   lv_obj_center(inclination_state_lable);

//   // x度数
//   g_x_degree_lable = lv_label_create(right_obj);
//   lv_label_set_text(g_x_degree_lable, INTER_ADJUS_X_GEGREE_NUM);
//   lv_obj_set_style_text_font(g_x_degree_lable, &lv_font_montserrat_14, 0);
//   lv_obj_align_to(g_x_degree_lable, right_obj, LV_ALIGN_TOP_LEFT, 20, 20);
//   // y度数
//   g_y_degree_lable = lv_label_create(right_obj);
//   lv_label_set_text(g_y_degree_lable, INTER_ADJUS_Y_GEGREE_NUM);
//   lv_obj_set_style_text_font(g_y_degree_lable, &lv_font_montserrat_14, 0);
//   lv_obj_align_to(g_y_degree_lable, right_obj, LV_ALIGN_TOP_LEFT, 20, 35);
//   // imei号
//   g_sys_imei_lable = lv_label_create(parent);
//   lv_obj_set_style_text_font(g_sys_imei_lable, &lv_font_montserrat_14, 0);
// #if !USE_SDL_SIM
//   if (g_sys_imei_data == NULL)
//   {
//     lv_label_set_text(g_sys_imei_lable, "0000000000000000");
//   }
//   else
//   {
//     lv_label_set_text(g_sys_imei_lable, g_sys_imei_data);
//   }
// #else
//   lv_label_set_text(g_sys_imei_lable, "0000000000000000");
// #endif
//   lv_obj_align_to(g_sys_imei_lable, parent, LV_ALIGN_BOTTOM_LEFT, 18, -10);

//   g_adjustment_timer = lv_timer_create(adjustment_timer_cb, 500, NULL);
//   navbar_foreground(true, true);
// }
// static void adjustment_delete(lv_obj_t *parent)
// {
// #if !USE_SDL_SIM
//   lv_timer_del(g_sys_status_icon_timer);
// #endif
//   lv_timer_del(g_adjustment_timer);
//   lv_obj_del(user_if_head);
//   lv_obj_del(g_sys_imei_lable);
//   lv_obj_del(left_obj);
//   lv_obj_del(right_obj);
// }
/********************************************************************曲线显示界面******************************************************************/

/*!
 * @function: operation_create 曲线界面
 */
// static void curve_create(lv_obj_t *parent)
// {
//   // 创建界面标识
//   user_if_head = user_interface_title(parent, TABVIEW_ID3);
//   // 创建曲线界面显示区域
//   g_chart = lv_chart_create(parent);
//   lv_chart_set_type(g_chart, LV_CHART_TYPE_LINE); // LV_CHART_TYPE_SCATTER LV_CHART_TYPE_LINE
//   lv_obj_set_style_text_font(g_chart, &lv_font_montserrat_10, 0);
//   lv_obj_set_size(g_chart, MONITOR_WIDTH - 70, MONITOR_HEIGHT - TABVIEW_TAB_H - 60);
//   lv_obj_align_to(g_chart, user_if_head, LV_ALIGN_OUT_BOTTOM_MID, 20, 18);
//   // lv_obj_add_event_cb(g_chart, chart_event_cb, LV_EVENT_ALL, NULL);  //
//   // 此处应该添加针对按键触发显示的点信息
//   g_cursor = lv_chart_add_cursor(g_chart, lv_palette_main(LV_PALETTE_BLUE),
//                                  LV_DIR_LEFT | LV_DIR_BOTTOM); // 添加光标
//   g_ser1 = lv_chart_add_series(g_chart, lv_palette_main(LV_PALETTE_RED),
//                                LV_CHART_AXIS_PRIMARY_Y); // 添加数据1系列
//   g_ser2 = lv_chart_add_series(g_chart, lv_palette_main(LV_PALETTE_BLUE),
//                                LV_CHART_AXIS_PRIMARY_Y); // 添加数据2系列
//   lv_chart_set_range(g_chart, LV_CHART_AXIS_PRIMARY_Y, CURVE_Y_AXIS_MIN_VAL,
//                      CURVE_Y_AXIS_MAX_VAL); // 设置y轴范围
//   lv_chart_set_range(g_chart, LV_CHART_AXIS_PRIMARY_X, CURVE_X_AXIS_MIN_VAL,
//                      CURVE_X_AXIS_MAX_VAL); // 设置x轴显示范围
//   lv_chart_set_update_mode(g_chart, LV_CHART_UPDATE_MODE_CIRCULAR);
//   lv_chart_set_point_count(g_chart, CURVE_POINT_NUM);
//   lv_chart_set_zoom_x(g_chart, 256); // 不缩放
//   lv_chart_set_axis_tick(g_chart, LV_CHART_AXIS_PRIMARY_Y, 8, 5, 6, 5, true, 40);
//   lv_chart_set_axis_tick(g_chart, LV_CHART_AXIS_PRIMARY_X, 8, 3, 27, 5, true, 30); // -1 <-> 26
//   lv_obj_refresh_ext_draw_size(g_chart);
//   lv_obj_set_style_size(g_chart, 0, LV_PART_INDICATOR);

//   lv_chart_set_all_value(g_chart, g_ser1, -CURVE_Y_AXIS_MAX_VAL);
//   lv_chart_set_ext_y_array(g_chart, g_ser1, g_curue_data1);
//   lv_chart_set_all_value(g_chart, g_ser2, -CURVE_Y_AXIS_MAX_VAL);
//   lv_chart_set_ext_y_array(g_chart, g_ser2, g_curue_data2);

//   static lv_style_t style_line_point;
//   lv_style_init(&style_line_point);
//   lv_style_set_line_width(&style_line_point, 2);
//   lv_style_set_line_color(&style_line_point, lv_palette_main(LV_PALETTE_GREEN));
//   lv_style_set_line_rounded(&style_line_point, true);

//   line1 = lv_line_create(g_chart);
//   lv_obj_add_style(line1, &style_line_point, 0);
//   // 添加图标说明
//   x_axis_tag = lv_obj_create(parent);
//   lv_obj_clear_flag(x_axis_tag, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_set_size(x_axis_tag, 30, 80);
//   lv_obj_align_to(x_axis_tag, g_chart, LV_ALIGN_OUT_LEFT_MID, -30, 0);
//   lv_obj_t *x_axis_tag_lable = lv_label_create(x_axis_tag);
//   lv_label_set_text(x_axis_tag_lable, CURVE_X_TAG_NAME);
//   lv_obj_set_style_text_font(x_axis_tag_lable, &user_interface_font_12, 0);
//   lv_obj_align(x_axis_tag_lable, LV_ALIGN_CENTER, 0, 0);
//   y_axis_tag = lv_obj_create(parent);
//   lv_obj_clear_flag(y_axis_tag, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_set_size(y_axis_tag, 80, 30);
//   lv_obj_align_to(y_axis_tag, g_chart, LV_ALIGN_OUT_BOTTOM_MID, -10, 20);
//   lv_obj_t *y_axis_tag_lable = lv_label_create(y_axis_tag);
//   lv_label_set_text(y_axis_tag_lable, CURVE_Y_TAG_NAME);
//   lv_obj_set_style_text_font(y_axis_tag_lable, &user_interface_font_12, 0);
//   lv_obj_align(y_axis_tag_lable, LV_ALIGN_CENTER, 0, -3);
// #if USE_SDL_SIM
//   g_curve_timer = lv_timer_create(curve_timer_cb, 200, NULL); // 对于曲线界面需要一直启用定时器
// #endif
//   navbar_foreground(true, true);
//   g_curue_view_enable = 1;
// }
// static void curve_delete(lv_obj_t *parent)
// {
// #if !USE_SDL_SIM
//   lv_timer_del(g_sys_status_icon_timer);
// #endif
//   g_curue_view_enable = 0;
// #if USE_SDL_SIM
//   lv_timer_del(g_curve_timer);
// #endif
//   lv_obj_del(user_if_head);
//   lv_obj_del(g_chart);
//   lv_obj_del(x_axis_tag);
//   lv_obj_del(y_axis_tag);
// }
/********************************************************************log记录界面******************************************************************/
/*! 清空表格
 * @param: obj : 表格对象
 */
// static void table_clear(lv_obj_t *obj)
// {
//   if (!lv_obj_is_valid(obj))
//     return;
//   uint16_t i, j;
//   uint16_t row_cnt = lv_table_get_row_cnt(obj);
//   uint16_t col_cnt = lv_table_get_col_cnt(obj);
//   for (i = 0; i < row_cnt; i++)
//     for (j = 0; j < col_cnt; j++)
//       lv_table_set_cell_value(g_table, i, j, ""); // 置空
// }
// static uint32_t g_table_cur_row_num = 0;
// static void table_timer_cb(lv_timer_t *e)
// {
//   /*Refresh the text*/
//   { // TODO: for test
//     srand((unsigned int)time(NULL));
//     uint16_t i;
//     char buf[32] = {0};
//     for (i = 0; i < TABLE_COL_CNT; i++)
//     {
//       sprintf(buf, "%d", (rand() % (250 + i)));
//       lv_table_set_cell_value(g_table, g_table_cur_row_num, i, (const char *)buf);
//     }
//     g_table_cur_row_num++;

//     if (g_table_cur_row_num > TABLE_ROW_CNT)
//     {
//       table_clear(g_table);
//       g_table_cur_row_num = 0; // 重置table
//     }
//   }
// }
// static void draw_part_event_cb(lv_event_t *e)
// {
//   lv_obj_t *obj = lv_event_get_target(e);
//   lv_obj_draw_part_dsc_t *dsc = lv_event_get_param(e);
//   /*If the cells are drawn...*/
//   if (dsc->part == LV_PART_ITEMS)
//   {
//     uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
//     uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);

//     /*Make the texts in the first cell center aligned*/
//     if (row == 0)
//     {
//       dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;
//       dsc->rect_dsc->bg_color =
//           lv_color_mix(lv_palette_main(LV_PALETTE_BLUE), dsc->rect_dsc->bg_color, LV_OPA_20);
//       dsc->rect_dsc->bg_opa = LV_OPA_COVER;
//     }
//     /*In the first column align the texts to the right*/
//     else if (col == 0)
//     {
//       dsc->label_dsc->flag = LV_TEXT_ALIGN_RIGHT;
//     }

//     /*MAke every 2nd row grayish*/
//     if ((row != 0 && row % 2) == 0)
//     {
//       dsc->rect_dsc->bg_color =
//           lv_color_mix(lv_palette_main(LV_PALETTE_GREY), dsc->rect_dsc->bg_color, LV_OPA_10);
//       dsc->rect_dsc->bg_opa = LV_OPA_COVER;
//     }
//   }
// }
/*!
 * @function: operation_create 记录界面
 */
// static void record_create(lv_obj_t *parent)
// {
// #define TABLE_TITLE_HEIGHT 26
// #define TABLE_TITLE_WIDTH 80
// #define TABLE_TO_TABVIEW_J_OFFSET 3
//   uint32_t i;
//   user_if_head = user_interface_title(parent, TABVIEW_ID4);
//   static lv_style_t table_title_style; // 表头字体样式
//   lv_style_init(&table_title_style);
//   lv_style_set_text_font(&table_title_style, &user_interface_font_10);
// #define TABLE_HEAD_HEIGHT 38
//   table_list_head = lv_list_create(parent); // 创建固定表头
//   lv_obj_add_style(table_list_head, &table_title_style, 0);
//   lv_obj_set_size(table_list_head, MONITOR_WIDTH - (TABLE_TO_TABVIEW_J_OFFSET * 2),
//                   TABLE_HEAD_HEIGHT + (TABLE_TO_TABVIEW_J_OFFSET * 2));
//   lv_obj_align_to(table_list_head, user_if_head, LV_ALIGN_OUT_BOTTOM_MID, 0, 0); // 设置表头位置
//   lv_obj_clear_flag(table_list_head, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_t *item_head = lv_list_add_btn(table_list_head, NULL, NULL);
//   for (i = 0; i < TABLE_COL_CNT; i++)
//   {
//     lv_obj_t *lab = lv_label_create(item_head);
//     lv_label_set_text_fmt(lab, "%s", g_table_cell_dsc[i]);
//     lv_obj_set_flex_align(item_head, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_END,
//                           LV_FLEX_ALIGN_SPACE_BETWEEN);
//   }
//   g_table = lv_table_create(parent);
//   lv_table_set_row_cnt(g_table, TABLE_ROW_CNT);
//   lv_table_set_col_cnt(g_table, TABLE_COL_CNT);
//   for (i = 0; i < TABLE_COL_CNT; i++)
//   { // 添加表头
//     lv_table_set_col_width(g_table, i,
//                            (MONITOR_WIDTH - (TABLE_TO_TABVIEW_J_OFFSET * 2)) / TABLE_COL_CNT);
//   }
//   lv_obj_set_size(g_table, MONITOR_WIDTH - (TABLE_TO_TABVIEW_J_OFFSET * 2),
//                   MONITOR_HEIGHT - TABVIEW_TAB_H - TABLE_HEAD_HEIGHT - 18);
//   lv_obj_align_to(g_table, table_list_head, LV_ALIGN_OUT_BOTTOM_MID, 0, TABLE_TO_TABVIEW_J_OFFSET);
//   lv_obj_add_event_cb(g_table, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
//   g_table_timer = lv_timer_create(table_timer_cb, 5 * 1000, NULL); // 动态添加表信息 5s定时

//   navbar_foreground(true, true);
// }

// static void record_delete(lv_obj_t *parent)
// {
// #if !USE_SDL_SIM
//   lv_timer_del(g_sys_status_icon_timer);
// #endif
//   lv_timer_del(g_table_timer);
//   lv_obj_del(user_if_head);
//   lv_obj_del(table_list_head);
//   lv_obj_del(g_table);
// }
/*********************************************************************************************************************************************/
// #pragma pack(4)
// typedef struct
// {
//   uint16_t depth_gauge;               // 深度计系数
//   uint16_t depth_reporting_threshold; // 深度计上报阈值
//   uint8_t depth_dir : 1;              // 深度计方向
//   uint8_t depth_mode : 1;             // 深度计模式
//   uint8_t flow_mode : 1;              // 流量计模式
//   uint8_t flow_pulse_pin : 1;         // 深度计脉冲模式引脚选择 TODO: 当前只有两个
//   /**
//    * @brief 倾角传感器
//    *
//    */
//   uint32_t inc_x_val; // x度数
//   uint32_t inc_y_val; // y度数
//   /**
//    * @brief 称重传感器
//    *
//    */
//   uint32_t weigh_pj_low_val;   // 工程低值
//   uint32_t weigh_pj_high_val;  // 工程高值
//   uint32_t weigh_ori_low_val;  // 原始低值
//   uint32_t weigh_ori_high_val; // 原始高值
//   /**
//    * @brief 流量计1
//    *
//    */
//   uint32_t flow1_pj_low_val;   // 工程低值
//   uint32_t flow1_pj_high_val;  // 工程高值
//   uint32_t flow1_ori_low_val;  // 原始低值
//   uint32_t flow1_ori_high_val; // 原始高值
//   /**
//    * @brief 流量计2
//    *
//    */
//   uint32_t flow2_pj_low_val;   // 工程低值
//   uint32_t flow2_pj_high_val;  // 工程高值
//   uint32_t flow2_ori_low_val;  // 原始低值
//   uint32_t flow2_ori_high_val; // 原始高值
// } app_cfg_t;                   // 应用配置参数
// #pragma pack()
// static void event_handler_depth_meter(lv_event_t *e);
// static void event_handler_flow_meter(lv_event_t *e);
// static void event_handler_calibration(lv_event_t *e);
// static void event_handler_system(lv_event_t *e);
// static void event_handler_user1(lv_event_t *e);
// static void event_handler_user2(lv_event_t *e);
// static void setting_interface_title_create(lv_obj_t *parent);
// static void setting_interface_menu_head_delete(void);
// static void setting_interface_menu_head_create(lv_obj_t *parent, const char *dsc);
// static void setting_interface_depth_create(lv_obj_t *parent);
// static void setting_interface_depth_delete(void);
// static void setting_interface_menu_delete(void);
// static void setting_interface_menu_create(lv_obj_t *parent);
// static void setting_interface_flow_create(lv_obj_t *parent);
// static void setting_interface_flow_delete(void);
// static void setting_interface_calibration_create(lv_obj_t *parent);
// static void setting_interface_calibration_delete(void);

// static void setting_interface_system_create(lv_obj_t *parent);
// static void setting_interface_system_delete(void);
// static void setting_interface_user1_create(lv_obj_t *parent);
// static void setting_interface_user1_delete(void);
// static void setting_interface_user2_create(lv_obj_t *parent);
// static void setting_interface_user2_delete(void);
// static lv_event_cb_t g_setting_btn_event[] = {event_handler_depth_meter, event_handler_flow_meter,
//                                               event_handler_calibration, event_handler_system,
//                                               event_handler_user1, event_handler_user2};
// static lv_obj_t *g_setting_if_menu_head;
// static lv_obj_t *g_setting_ctl_wid_area;
// static lv_obj_t *g_setting_if_depth_area;
// static lv_obj_t *g_setting_if_flow_area;
// static lv_obj_t *g_setting_if_cal_area;

// static lv_obj_t *g_setting_if_system_area;
// // static lv_obj_t *g_setting_if_user1_area;
// static lv_obj_t *g_setting_if_user2_area;

// static lv_obj_t *cal_inc_x_label;
// static lv_obj_t *cal_inc_y_label;
// static lv_timer_t *g_set_cal_inc_timer;
// enum
// {
//   SETTING_DEF_ID = 0, // menu界面
//   SETTING_DEPTH_METER_ID = 1,
//   SETTING_FLOW_METER_ID = 2,
//   SETTING_CALIBRATION_ID = 3,
//   SETTING_SYSTEM_ID = 4,
//   SETTING_USER1_ID = 5,
//   SETTING_USER2_ID = 6,
//   SETTING_MENU_ID_MAX
// };
// static uint16_t g_setting_btn_cur_id = SETTING_DEF_ID; // 初始化默认
// // typedef void (*setting_if_cb_t)(lv_obj_t *d);  // setting menu创建函数指针
// typedef void (*setting_if_cb_del_t)(void); // setting menu删除函数指针
// // static setting_if_cb_t g_set_if_menu_create[] = {setting_interface_menu_create};
// static setting_if_cb_del_t g_set_if_menu_del[] = {
//     setting_interface_menu_delete, setting_interface_depth_delete,
//     setting_interface_flow_delete, setting_interface_calibration_delete,
//     setting_interface_system_delete, setting_interface_user1_delete,
//     setting_interface_user2_delete};
// static lv_style_t pps_style_radio;
// static lv_style_t pps_style_radio_chk;
// static uint32_t active_index_1 = 0;
// static uint32_t active_index_2 = 0;

// enum
// {
//   SETTING_CAL_INC_ID = 0,        // 倾角传感器校准界面
//   SETTING_CAL_WEIGH_ID = 1,      // 称重传感器校准界面
//   SETTING_CAL_FLOW1_FREQ_ID = 2, // 流量计1频率模式校准界面
//   SETTING_CAL_FLOW2_FREQ_ID = 3, // 流量计2频率模式校准界面
//   SETTING_CAL_FLOW1_ELEC_ID = 4, // 流量计1电流模式校准界面
//   SETTING_CAL_FLOW2_ELEC_ID = 5, // 流量计2电流模式校准界面
//   SETTING_CAL_MENU_ID_MAX
// };
// static uint16_t g_setting_cal_cur_id = SETTING_CAL_INC_ID; // 初始化默认-倾角传感器校准界面
// static void setting_if_inc_cal_create(lv_obj_t *parent);
// static void setting_if_inc_cal_delete(void);
// static void event_handler_cal_inclination(lv_event_t *e);
// static void setting_if_weigh_cal_create(lv_obj_t *parent);
// static void setting_if_weigh_cal_delete(void);
// static void event_handler_cal_weigh(lv_event_t *e);
// static lv_obj_t *setting_if_flow_cal_create_def(lv_obj_t *parent, const char *btn_dsc,
//                                                 lv_event_cb_t event_cb, lv_obj_t *pj_low,
//                                                 lv_obj_t *pj_high, lv_obj_t *ori_low,
//                                                 lv_obj_t *ori_high, bool lbtn_able, bool rbtn_able);
// static void setting_if_flow1_freq_cal_create(lv_obj_t *parent);
// static void setting_if_flow1_freq_cal_delete(void);
// static void event_handler_cal_flow1_freq(lv_event_t *e);
// static void setting_if_flow2_freq_cal_create(lv_obj_t *parent);
// static void setting_if_flow2_freq_cal_delete(void);
// static void event_handler_cal_flow2_freq(lv_event_t *e);
// static void setting_if_flow1_elec_cal_create(lv_obj_t *parent);
// static void setting_if_flow1_elec_cal_delete(void);
// static void event_handler_cal_flow1_elec(lv_event_t *e);
// static void setting_if_flow2_elec_cal_create(lv_obj_t *parent);
// static void setting_if_flow2_elec_cal_delete(void);
// static void event_handler_cal_flow2_elec(lv_event_t *e);

// typedef void (*setting_cal_cb_t)(lv_obj_t *d); // setting校准界面创建函数指针
// typedef void (*setting_cal_cb_del_t)(void);    // setting校准界面删除函数指针
// static setting_cal_cb_t g_set_cal_menu_create[] = {
//     setting_if_inc_cal_create, setting_if_weigh_cal_create,
//     setting_if_flow1_freq_cal_create, setting_if_flow2_freq_cal_create,
//     setting_if_flow1_elec_cal_create, setting_if_flow2_elec_cal_create};
// static setting_cal_cb_del_t g_set_cal_menu_del[] = {
//     setting_if_inc_cal_delete, setting_if_weigh_cal_delete,
//     setting_if_flow1_freq_cal_delete, setting_if_flow2_freq_cal_delete,
//     setting_if_flow1_elec_cal_delete, setting_if_flow2_elec_cal_delete};

// static lv_obj_t *g_setting_if_inc_cal_area;   // inc
// static lv_obj_t *g_setting_if_weigh_cal_area; // weigh
// static lv_obj_t *g_setting_if_flow1_freq_cal_area;
// static lv_obj_t *g_setting_if_flow2_freq_cal_area;
// static lv_obj_t *g_setting_if_flow1_elec_cal_area;
// static lv_obj_t *g_setting_if_flow2_elec_cal_area;

// 校准菜单右按钮回调
// static void cal_menu_right_event_handler(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     LV_LOG_USER("cal menu right btn clicked\n");
//     if (g_set_cal_menu_del[g_setting_cal_cur_id] != NULL)
//       g_set_cal_menu_del[g_setting_cal_cur_id]();
//     if (g_setting_cal_cur_id <= SETTING_CAL_MENU_ID_MAX - 1)
//     {
//       g_setting_cal_cur_id++;
//     }
//     else
//     {
//       g_setting_cal_cur_id = 0;
//     }
//     lv_obj_t *parent = lv_obj_get_parent(g_setting_if_menu_head);
//     if (g_set_cal_menu_create[g_setting_cal_cur_id] != NULL)
//       g_set_cal_menu_create[g_setting_cal_cur_id](parent);
//   }
// }

// 校准菜单左动按钮回调
// static void cal_menu_left_event_handler(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     LV_LOG_USER("cal menu left btn clicked\n");
//     if (g_set_cal_menu_del[g_setting_cal_cur_id] != NULL)
//       g_set_cal_menu_del[g_setting_cal_cur_id]();
//     if (g_setting_cal_cur_id > 0)
//     {
//       g_setting_cal_cur_id--;
//     }
//     else
//     {
//       g_setting_cal_cur_id = SETTING_CAL_MENU_ID_MAX - 1;
//     }
//     lv_obj_t *parent = lv_obj_get_parent(g_setting_if_menu_head);
//     if (g_set_cal_menu_create[g_setting_cal_cur_id] != NULL)
//       g_set_cal_menu_create[g_setting_cal_cur_id](parent);
//   }
// }

// static void event_handler_cal_flow1_elec(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     // 流量计2 频率模式 校准触发
//     LV_LOG_USER("cal menu weigh btn clicked\n");
//   }
// }

// static void setting_if_flow1_elec_cal_create(lv_obj_t *parent)
// {
//   lv_obj_t *pj_low;
//   lv_obj_t *pj_high;
//   lv_obj_t *ori_low;
//   lv_obj_t *ori_high;
//   g_setting_if_flow1_elec_cal_area =
//       setting_if_flow_cal_create_def(parent, "流量计1 电流模式", event_handler_cal_flow1_elec,
//                                      pj_low, pj_high, ori_low, ori_high, true, true);
//   g_setting_cal_cur_id = SETTING_CAL_FLOW1_ELEC_ID;
//   navbar_foreground(true, true);
// }

// static void setting_if_flow1_elec_cal_delete(void)
// {
//   if (g_setting_if_flow1_elec_cal_area != NULL)
//   {
//     lv_obj_del(g_setting_if_flow1_elec_cal_area);
//     g_setting_if_flow1_elec_cal_area = NULL;
//   }
// }

// static void event_handler_cal_flow2_elec(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     // 流量计2 频率模式 校准触发
//     LV_LOG_USER("cal menu weigh btn clicked\n");
//   }
// }

// static void setting_if_flow2_elec_cal_create(lv_obj_t *parent)
// {
//   lv_obj_t *pj_low;
//   lv_obj_t *pj_high;
//   lv_obj_t *ori_low;
//   lv_obj_t *ori_high;
//   g_setting_if_flow2_elec_cal_area =
//       setting_if_flow_cal_create_def(parent, "流量计2 电流模式", event_handler_cal_flow2_elec,
//                                      pj_low, pj_high, ori_low, ori_high, true, false);
//   g_setting_cal_cur_id = SETTING_CAL_FLOW2_ELEC_ID;
//   navbar_foreground(true, true);
// }

// static void setting_if_flow2_elec_cal_delete(void)
// {
//   if (g_setting_if_flow2_elec_cal_area != NULL)
//   {
//     lv_obj_del(g_setting_if_flow2_elec_cal_area);
//     g_setting_if_flow2_elec_cal_area = NULL;
//   }
// }

// static void event_handler_cal_flow2_freq(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     // 流量计2 频率模式 校准触发
//     LV_LOG_USER("cal menu weigh btn clicked\n");
//   }
// }

// static void setting_if_flow2_freq_cal_create(lv_obj_t *parent)
// {
//   lv_obj_t *pj_low;
//   lv_obj_t *pj_high;
//   lv_obj_t *ori_low;
//   lv_obj_t *ori_high;
//   g_setting_if_flow2_freq_cal_area =
//       setting_if_flow_cal_create_def(parent, "流量计2 频率模式", event_handler_cal_flow2_freq,
//                                      pj_low, pj_high, ori_low, ori_high, true, true);
//   g_setting_cal_cur_id = SETTING_CAL_FLOW2_FREQ_ID;
//   navbar_foreground(true, true);
// }

// static void setting_if_flow2_freq_cal_delete(void)
// {
//   if (g_setting_if_flow2_freq_cal_area != NULL)
//   {
//     lv_obj_del(g_setting_if_flow2_freq_cal_area);
//     g_setting_if_flow2_freq_cal_area = NULL;
//   }
// }

// static void event_handler_cal_flow1_freq(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     // 流量计1 频率模式 校准触发
//     LV_LOG_USER("cal menu weigh btn clicked\n");
//   }
// }

// static void setting_if_flow1_freq_cal_create(lv_obj_t *parent)
// {
//   lv_obj_t *pj_low;
//   lv_obj_t *pj_high;
//   lv_obj_t *ori_low;
//   lv_obj_t *ori_high;
//   g_setting_if_flow1_freq_cal_area =
//       setting_if_flow_cal_create_def(parent, "流量计1 频率模式", event_handler_cal_flow1_freq,
//                                      pj_low, pj_high, ori_low, ori_high, true, true);
//   g_setting_cal_cur_id = SETTING_CAL_FLOW1_FREQ_ID;
//   navbar_foreground(true, true);
// }

// static void setting_if_flow1_freq_cal_delete(void)
// {
//   if (g_setting_if_flow1_freq_cal_area != NULL)
//   {
//     lv_obj_del(g_setting_if_flow1_freq_cal_area);
//     g_setting_if_flow1_freq_cal_area = NULL;
//   }
// }

// 流量计默认校准界面
// static lv_obj_t *setting_if_flow_cal_create_def(lv_obj_t *parent, const char *btn_dsc,
//                                                 lv_event_cb_t event_cb, lv_obj_t *pj_low,
//                                                 lv_obj_t *pj_high, lv_obj_t *ori_low,
//                                                 lv_obj_t *ori_high, bool lbtn_able,
//                                                 bool rbtn_able)
// {
//   lv_obj_t *setting_if_cal_area = lv_obj_create(parent); // 创建区域
//   lv_obj_set_size(setting_if_cal_area, SETTING_CTLWID_AREA_WIDTH, SETTING_CTLWID_AREA_HEIGHT);
//   lv_obj_set_style_radius(setting_if_cal_area, 0, 0);
//   lv_obj_align_to(setting_if_cal_area, g_setting_if_menu_head, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
//   lv_obj_clear_flag(setting_if_cal_area, LV_OBJ_FLAG_SCROLLABLE);
// #define WEIGH_BTN_WIDTH 100
// #define WEIGH_BTN_HEIGHT 100
//   static lv_style_t setting_cal_btn_style; // 按钮样式
//   lv_style_init(&setting_cal_btn_style);
//   lv_style_set_radius(&setting_cal_btn_style, WEIGH_BTN_WIDTH / 2);
//   lv_style_set_text_font(&setting_cal_btn_style, &user_interface_font_20);

//   lv_obj_t *cal_weigh_btn = lv_btn_create(setting_if_cal_area); // 创建称重传感器校准按钮
//   lv_obj_set_size(cal_weigh_btn, WEIGH_BTN_WIDTH, WEIGH_BTN_HEIGHT);
//   lv_obj_align_to(cal_weigh_btn, setting_if_cal_area, LV_ALIGN_CENTER,
//                   -SETTING_CTLWID_AREA_WIDTH / 4, 0);
//   lv_obj_add_style(cal_weigh_btn, &setting_cal_btn_style, 0);
//   lv_obj_add_event_cb(cal_weigh_btn, event_cb, LV_EVENT_ALL, NULL); // 添加触发事件
//   lv_obj_t *cal_weigh_btn_label = lv_label_create(cal_weigh_btn);   // 图标描述
//   lv_label_set_text(cal_weigh_btn_label, "校准");
//   lv_obj_set_style_text_font(cal_weigh_btn_label, &user_interface_font_42, 0);
//   lv_obj_align(cal_weigh_btn_label, LV_ALIGN_CENTER, 0, 0);
//   lv_obj_t *cal_weigh_btn_src_label = lv_label_create(setting_if_cal_area); // 图标描述
//   lv_label_set_text(cal_weigh_btn_src_label, btn_dsc);
//   lv_obj_set_style_text_font(cal_weigh_btn_src_label, &user_interface_font_16, 0);
//   lv_obj_align_to(cal_weigh_btn_src_label, cal_weigh_btn, LV_ALIGN_OUT_TOP_MID, 0, -5);

//   lv_obj_t *A = lv_obj_create(setting_if_cal_area); // 创建测量值显示区域
// #define INC_A_AREA_WIDTH (SETTING_CTLWID_AREA_WIDTH / 2 - 50)
// #define INC_A_AREA_HEIGHT ((SETTING_CTLWID_AREA_HEIGHT / 6) * 4)
//   lv_obj_set_size(A, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT);
//   lv_obj_set_style_radius(A, 0, 0);
//   lv_obj_clear_flag(A, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(A, setting_if_cal_area, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 2, 0);

//   lv_obj_t *Aa = lv_obj_create(A);
//   lv_obj_set_size(Aa, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Aa, 0, 0);
//   lv_obj_clear_flag(Aa, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Aa, A, LV_ALIGN_CENTER, 0,
//                   -(INC_A_AREA_HEIGHT / 4 + (INC_A_AREA_HEIGHT / 4) / 2));
//   lv_obj_t *Aa1 = lv_obj_create(Aa);
//   lv_obj_set_size(Aa1, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Aa1, 0, 0);
//   lv_obj_clear_flag(Aa1, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Aa1, Aa, LV_ALIGN_CENTER, -INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *Aa2 = lv_obj_create(Aa);
//   lv_obj_set_size(Aa2, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Aa2, 0, 0);
//   lv_obj_clear_flag(Aa2, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Aa2, Aa, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *cal_pj_low_dsc_label = lv_label_create(Aa1);
//   lv_label_set_text(cal_pj_low_dsc_label, "工程低值:");
//   lv_obj_set_style_text_font(cal_pj_low_dsc_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_pj_low_dsc_label);
//   pj_low = lv_label_create(Aa2);
//   lv_label_set_text(pj_low, "00.00");
//   lv_obj_set_style_text_font(pj_low, &user_interface_font_16, 0);
//   lv_obj_center(pj_low);

//   lv_obj_t *Ab = lv_obj_create(A);
//   lv_obj_set_size(Ab, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ab, 0, 0);
//   lv_obj_clear_flag(Ab, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ab, A, LV_ALIGN_CENTER, 0, -((INC_A_AREA_HEIGHT / 4) / 2));
//   lv_obj_t *Ab1 = lv_obj_create(A);
//   lv_obj_set_size(Ab1, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ab1, 0, 0);
//   lv_obj_clear_flag(Ab1, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ab1, Ab, LV_ALIGN_CENTER, -INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *Ab2 = lv_obj_create(A);
//   lv_obj_set_size(Ab2, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ab2, 0, 0);
//   lv_obj_clear_flag(Ab2, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ab2, Ab, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *cal_pj_high_dsc_label = lv_label_create(Ab1);
//   lv_label_set_text(cal_pj_high_dsc_label, "工程高值:");
//   lv_obj_set_style_text_font(cal_pj_high_dsc_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_pj_high_dsc_label);
//   pj_high = lv_label_create(Ab2);
//   lv_label_set_text(pj_high, "00.00");
//   lv_obj_set_style_text_font(pj_high, &user_interface_font_16, 0);
//   lv_obj_center(pj_high);

//   lv_obj_t *Ac = lv_obj_create(A);
//   lv_obj_set_size(Ac, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ac, 0, 0);
//   lv_obj_clear_flag(Ac, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ac, A, LV_ALIGN_CENTER, 0, ((INC_A_AREA_HEIGHT / 4) / 2));
//   lv_obj_t *Ac1 = lv_obj_create(Ac);
//   lv_obj_set_size(Ac1, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ac1, 0, 0);
//   lv_obj_clear_flag(Ac1, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ac1, Ac, LV_ALIGN_CENTER, -INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *Ac2 = lv_obj_create(Ac);
//   lv_obj_set_size(Ac2, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ac2, 0, 0);
//   lv_obj_clear_flag(Ac2, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ac2, Ac, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *cal_ori_low_dsc_label = lv_label_create(Ac1);
//   lv_label_set_text(cal_ori_low_dsc_label, "原始低值:");
//   lv_obj_set_style_text_font(cal_ori_low_dsc_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_ori_low_dsc_label);
//   ori_low = lv_label_create(Ac2);
//   lv_label_set_text(ori_low, "00.00");
//   lv_obj_set_style_text_font(ori_low, &user_interface_font_16, 0);
//   lv_obj_center(ori_low);

//   lv_obj_t *Ad = lv_obj_create(A);
//   lv_obj_set_size(Ad, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ad, 0, 0);
//   lv_obj_clear_flag(Ad, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ad, A, LV_ALIGN_CENTER, 0, (INC_A_AREA_HEIGHT / 4 + (INC_A_AREA_HEIGHT / 4) / 2));
//   lv_obj_t *Ad1 = lv_obj_create(A);
//   lv_obj_set_size(Ad1, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ad1, 0, 0);
//   lv_obj_clear_flag(Ad1, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ad1, Ad, LV_ALIGN_CENTER, -INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *Ad2 = lv_obj_create(A);
//   lv_obj_set_size(Ad2, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ad2, 0, 0);
//   lv_obj_clear_flag(Ad2, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ad2, Ad, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *cal_ori_high_dsc_label = lv_label_create(Ad1);
//   lv_label_set_text(cal_ori_high_dsc_label, "原始高值:");
//   lv_obj_set_style_text_font(cal_ori_high_dsc_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_ori_high_dsc_label);
//   ori_high = lv_label_create(Ad2);
//   lv_label_set_text(ori_high, "00.00");
//   lv_obj_set_style_text_font(ori_high, &user_interface_font_16, 0);
//   lv_obj_center(ori_high);

//   static lv_style_t menu_h_head_icon_style;
//   lv_style_init(&menu_h_head_icon_style);
//   lv_style_set_text_font(&menu_h_head_icon_style, &lv_font_montserrat_16);
//   if (lbtn_able == true)
//   {
//     lv_obj_t *left_btn = lv_btn_create(setting_if_cal_area);
//     static lv_style_t left_btn_style;
//     lv_style_init(&left_btn_style);
//     lv_style_set_radius(&left_btn_style, 0);
//     lv_style_set_text_font(&left_btn_style, &lv_font_montserrat_16);
//     lv_style_set_bg_color(&left_btn_style, lv_palette_lighten(LV_PALETTE_GREY, 3));
//     lv_style_set_text_color(&left_btn_style, lv_color_black());
//     lv_obj_add_style(left_btn, &left_btn_style, 0);
//     lv_obj_set_size(left_btn, TABVIEW_TAB_H, SETTING_CTLWID_AREA_HEIGHT);
//     lv_obj_align_to(left_btn, setting_if_cal_area, LV_ALIGN_LEFT_MID, -20, 0);
//     lv_obj_add_event_cb(left_btn, cal_menu_left_event_handler, LV_EVENT_ALL, NULL);
//     lv_obj_t *left_btn_lable = lv_label_create(left_btn);
//     lv_obj_add_style(left_btn_lable, &menu_h_head_icon_style, 0);
//     lv_label_set_text(left_btn_lable, LV_SYMBOL_LEFT);
//     lv_obj_center(left_btn_lable);
//   }
//   if (rbtn_able == true)
//   {
//     lv_obj_t *right_btn = lv_btn_create(setting_if_cal_area);
//     static lv_style_t right_btn_style;
//     lv_style_init(&right_btn_style);
//     lv_style_set_radius(&right_btn_style, 0);
//     lv_style_set_text_font(&right_btn_style, &lv_font_montserrat_16);
//     lv_style_set_bg_color(&right_btn_style, lv_palette_lighten(LV_PALETTE_GREY, 3));
//     lv_style_set_text_color(&right_btn_style, lv_color_black());
//     lv_obj_add_style(right_btn, &right_btn_style, 0);
//     lv_obj_set_size(right_btn, TABVIEW_TAB_H, SETTING_CTLWID_AREA_HEIGHT);
//     lv_obj_align_to(right_btn, setting_if_cal_area, LV_ALIGN_RIGHT_MID, 20, 0);
//     lv_obj_add_event_cb(right_btn, cal_menu_right_event_handler, LV_EVENT_ALL, NULL);
//     lv_obj_t *right_icon_lable = lv_label_create(right_btn);
//     lv_obj_add_style(right_icon_lable, &menu_h_head_icon_style, 0);
//     lv_label_set_text(right_icon_lable, LV_SYMBOL_RIGHT);
//     lv_obj_center(right_icon_lable);
//   }
//   navbar_foreground(true, true);
//   return setting_if_cal_area;
// }
// static void event_handler_cal_weigh(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     // 称重状态传感器 校准触发
//     LV_LOG_USER("cal menu weigh btn clicked\n");
//   }
// }
// 称重传感器校准界面
// static void setting_if_weigh_cal_create(lv_obj_t *parent)
// {
//   lv_obj_t *setting_if_cal_area = lv_obj_create(parent); // 创建区域
//   lv_obj_set_size(setting_if_cal_area, SETTING_CTLWID_AREA_WIDTH, SETTING_CTLWID_AREA_HEIGHT);
//   lv_obj_set_style_radius(setting_if_cal_area, 0, 0);
//   lv_obj_align_to(setting_if_cal_area, g_setting_if_menu_head, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
//   lv_obj_clear_flag(setting_if_cal_area, LV_OBJ_FLAG_SCROLLABLE);
// #define WEIGH_BTN_WIDTH 100
// #define WEIGH_BTN_HEIGHT 100
//   static lv_style_t setting_cal_btn_style; // 按钮样式
//   lv_style_init(&setting_cal_btn_style);
//   lv_style_set_radius(&setting_cal_btn_style, WEIGH_BTN_WIDTH / 2);
//   lv_style_set_text_font(&setting_cal_btn_style, &user_interface_font_20);

//   lv_obj_t *cal_weigh_btn = lv_btn_create(setting_if_cal_area); // 创建称重传感器校准按钮
//   lv_obj_set_size(cal_weigh_btn, WEIGH_BTN_WIDTH, WEIGH_BTN_HEIGHT);
//   lv_obj_align_to(cal_weigh_btn, setting_if_cal_area, LV_ALIGN_CENTER,
//                   -SETTING_CTLWID_AREA_WIDTH / 4, 0);
//   lv_obj_add_style(cal_weigh_btn, &setting_cal_btn_style, 0);
//   lv_obj_add_event_cb(cal_weigh_btn, event_handler_cal_weigh, LV_EVENT_ALL, NULL); // 添加触发事件
//   lv_obj_t *cal_weigh_btn_label = lv_label_create(cal_weigh_btn);                  // 图标描述
//   lv_label_set_text(cal_weigh_btn_label, "校准");
//   lv_obj_set_style_text_font(cal_weigh_btn_label, &user_interface_font_42, 0);
//   lv_obj_align(cal_weigh_btn_label, LV_ALIGN_CENTER, 0, 0);
//   lv_obj_t *cal_weigh_btn_src_label = lv_label_create(setting_if_cal_area); // 图标描述
//   lv_label_set_text(cal_weigh_btn_src_label, "称重传感器");
//   lv_obj_set_style_text_font(cal_weigh_btn_src_label, &user_interface_font_16, 0);
//   lv_obj_align_to(cal_weigh_btn_src_label, cal_weigh_btn, LV_ALIGN_OUT_TOP_MID, 0, -5);

//   lv_obj_t *A = lv_obj_create(setting_if_cal_area); // 创建测量值显示区域
// #define INC_A_AREA_WIDTH (SETTING_CTLWID_AREA_WIDTH / 2 - 50)
// #define INC_A_AREA_HEIGHT ((SETTING_CTLWID_AREA_HEIGHT / 6) * 4)
//   lv_obj_set_size(A, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT);
//   lv_obj_set_style_radius(A, 0, 0);
//   lv_obj_clear_flag(A, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(A, setting_if_cal_area, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 2, 0);

//   lv_obj_t *Aa = lv_obj_create(A);
//   lv_obj_set_size(Aa, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Aa, 0, 0);
//   lv_obj_clear_flag(Aa, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Aa, A, LV_ALIGN_CENTER, 0,
//                   -(INC_A_AREA_HEIGHT / 4 + (INC_A_AREA_HEIGHT / 4) / 2));
//   lv_obj_t *Aa1 = lv_obj_create(Aa);
//   lv_obj_set_size(Aa1, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Aa1, 0, 0);
//   lv_obj_clear_flag(Aa1, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Aa1, Aa, LV_ALIGN_CENTER, -INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *Aa2 = lv_obj_create(Aa);
//   lv_obj_set_size(Aa2, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Aa2, 0, 0);
//   lv_obj_clear_flag(Aa2, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Aa2, Aa, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *cal_pj_low_dsc_label = lv_label_create(Aa1);
//   lv_label_set_text(cal_pj_low_dsc_label, "工程低值:");
//   lv_obj_set_style_text_font(cal_pj_low_dsc_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_pj_low_dsc_label);
//   lv_obj_t *cal_pj_low_label = lv_label_create(Aa2);
//   lv_label_set_text(cal_pj_low_label, "00.00");
//   lv_obj_set_style_text_font(cal_pj_low_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_pj_low_label);

//   lv_obj_t *Ab = lv_obj_create(A);
//   lv_obj_set_size(Ab, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ab, 0, 0);
//   lv_obj_clear_flag(Ab, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ab, A, LV_ALIGN_CENTER, 0, -((INC_A_AREA_HEIGHT / 4) / 2));
//   lv_obj_t *Ab1 = lv_obj_create(A);
//   lv_obj_set_size(Ab1, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ab1, 0, 0);
//   lv_obj_clear_flag(Ab1, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ab1, Ab, LV_ALIGN_CENTER, -INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *Ab2 = lv_obj_create(A);
//   lv_obj_set_size(Ab2, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ab2, 0, 0);
//   lv_obj_clear_flag(Ab2, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ab2, Ab, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *cal_pj_high_dsc_label = lv_label_create(Ab1);
//   lv_label_set_text(cal_pj_high_dsc_label, "工程高值:");
//   lv_obj_set_style_text_font(cal_pj_high_dsc_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_pj_high_dsc_label);
//   lv_obj_t *cal_pj_high_label = lv_label_create(Ab2);
//   lv_label_set_text(cal_pj_high_label, "00.00");
//   lv_obj_set_style_text_font(cal_pj_high_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_pj_high_label);

//   lv_obj_t *Ac = lv_obj_create(A);
//   lv_obj_set_size(Ac, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ac, 0, 0);
//   lv_obj_clear_flag(Ac, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ac, A, LV_ALIGN_CENTER, 0, ((INC_A_AREA_HEIGHT / 4) / 2));
//   lv_obj_t *Ac1 = lv_obj_create(Ac);
//   lv_obj_set_size(Ac1, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ac1, 0, 0);
//   lv_obj_clear_flag(Ac1, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ac1, Ac, LV_ALIGN_CENTER, -INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *Ac2 = lv_obj_create(Ac);
//   lv_obj_set_size(Ac2, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ac2, 0, 0);
//   lv_obj_clear_flag(Ac2, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ac2, Ac, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *cal_ori_low_dsc_label = lv_label_create(Ac1);
//   lv_label_set_text(cal_ori_low_dsc_label, "原始低值:");
//   lv_obj_set_style_text_font(cal_ori_low_dsc_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_ori_low_dsc_label);
//   lv_obj_t *cal_ori_low_label = lv_label_create(Ac2);
//   lv_label_set_text(cal_ori_low_label, "000.0");
//   lv_obj_set_style_text_font(cal_ori_low_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_ori_low_label);

//   lv_obj_t *Ad = lv_obj_create(A);
//   lv_obj_set_size(Ad, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ad, 0, 0);
//   lv_obj_clear_flag(Ad, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ad, A, LV_ALIGN_CENTER, 0, (INC_A_AREA_HEIGHT / 4 + (INC_A_AREA_HEIGHT / 4) / 2));
//   lv_obj_t *Ad1 = lv_obj_create(A);
//   lv_obj_set_size(Ad1, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ad1, 0, 0);
//   lv_obj_clear_flag(Ad1, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ad1, Ad, LV_ALIGN_CENTER, -INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *Ad2 = lv_obj_create(A);
//   lv_obj_set_size(Ad2, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 4);
//   lv_obj_set_style_radius(Ad2, 0, 0);
//   lv_obj_clear_flag(Ad2, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ad2, Ad, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *cal_ori_high_dsc_label = lv_label_create(Ad1);
//   lv_label_set_text(cal_ori_high_dsc_label, "原始高值:");
//   lv_obj_set_style_text_font(cal_ori_high_dsc_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_ori_high_dsc_label);
//   lv_obj_t *cal_ori_high_label = lv_label_create(Ad2);
//   lv_label_set_text(cal_ori_high_label, "000.0");
//   lv_obj_set_style_text_font(cal_ori_high_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_ori_high_label);

//   lv_obj_t *left_btn = lv_btn_create(setting_if_cal_area);
//   static lv_style_t left_btn_style;
//   lv_style_init(&left_btn_style);
//   lv_style_set_radius(&left_btn_style, 0);
//   lv_style_set_text_font(&left_btn_style, &lv_font_montserrat_16);
//   lv_style_set_bg_color(&left_btn_style, lv_palette_lighten(LV_PALETTE_GREY, 3));
//   lv_style_set_text_color(&left_btn_style, lv_color_black());
//   lv_obj_add_style(left_btn, &left_btn_style, 0);
//   lv_obj_set_size(left_btn, TABVIEW_TAB_H, SETTING_CTLWID_AREA_HEIGHT);
//   lv_obj_align_to(left_btn, setting_if_cal_area, LV_ALIGN_LEFT_MID, -20, 0);
//   lv_obj_add_event_cb(left_btn, cal_menu_left_event_handler, LV_EVENT_ALL, NULL);
//   lv_obj_t *left_btn_lable = lv_label_create(left_btn);
//   lv_obj_set_style_text_font(left_btn_lable, &lv_font_montserrat_16, 0);
//   lv_label_set_text(left_btn_lable, LV_SYMBOL_LEFT);
//   lv_obj_center(left_btn_lable);

//   lv_obj_t *right_btn = lv_btn_create(setting_if_cal_area);
//   static lv_style_t right_btn_style;
//   lv_style_init(&right_btn_style);
//   lv_style_set_radius(&right_btn_style, 0);
//   lv_style_set_text_font(&right_btn_style, &lv_font_montserrat_16);
//   lv_style_set_bg_color(&right_btn_style, lv_palette_lighten(LV_PALETTE_GREY, 3));
//   lv_style_set_text_color(&right_btn_style, lv_color_black());
//   lv_obj_add_style(right_btn, &right_btn_style, 0);
//   lv_obj_set_size(right_btn, TABVIEW_TAB_H, SETTING_CTLWID_AREA_HEIGHT);
//   lv_obj_align_to(right_btn, setting_if_cal_area, LV_ALIGN_RIGHT_MID, 20, 0);
//   lv_obj_add_event_cb(right_btn, cal_menu_right_event_handler, LV_EVENT_ALL, NULL);
//   lv_obj_t *right_icon_lable = lv_label_create(right_btn);
//   lv_obj_set_style_text_font(right_icon_lable, &lv_font_montserrat_16, 0);
//   lv_label_set_text(right_icon_lable, LV_SYMBOL_RIGHT);
//   lv_obj_center(right_icon_lable);

//   g_setting_if_weigh_cal_area = setting_if_cal_area;
//   g_setting_cal_cur_id = SETTING_CAL_WEIGH_ID;
//   navbar_foreground(true, true);
// }
// static void setting_if_weigh_cal_delete(void)
// {
//   if (g_setting_if_weigh_cal_area != NULL)
//   {
//     lv_obj_del(g_setting_if_weigh_cal_area);
//     g_setting_if_weigh_cal_area = NULL;
//   }
// }
// #if !USE_SDL_SIM
// static void set_cal_inc_timer_cb(lv_timer_t *timer)
// {
//   // if (g_ui_cfg_data == NULL)
//   //   return;
//   if (cal_inc_x_label != NULL)
//     lv_label_set_text_fmt(cal_inc_x_label, "%d", g_ui_user_data.angle_x / 100);
//   if (cal_inc_y_label != NULL)
//     lv_label_set_text_fmt(cal_inc_y_label, "%d", g_ui_user_data.angle_y / 100);
// }
// #else
// static void set_cal_inc_timer_cb(lv_timer_t *timer)
// {
//   // if (g_ui_cfg_data == NULL)
//   //   return;
//   if (cal_inc_x_label != NULL)
//     lv_label_set_text_fmt(cal_inc_x_label, "%d", lv_rand(0, 10));
//   if (cal_inc_y_label != NULL)
//     lv_label_set_text_fmt(cal_inc_y_label, "%d", lv_rand(2, 20));
// }
// #endif // 倾角传感器校准界面
// static void setting_if_inc_cal_create(lv_obj_t *parent)
// {
//   lv_obj_t *setting_if_cal_area = lv_obj_create(parent); // 创建区域
//   lv_obj_set_size(setting_if_cal_area, SETTING_CTLWID_AREA_WIDTH, SETTING_CTLWID_AREA_HEIGHT);
//   lv_obj_set_style_radius(setting_if_cal_area, 0, 0);
//   lv_obj_align_to(setting_if_cal_area, g_setting_if_menu_head, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
//   lv_obj_clear_flag(setting_if_cal_area, LV_OBJ_FLAG_SCROLLABLE);
// #define CAL_BTN_WIDTH 100
// #define CAL_BTN_HEIGHT 100
//   static lv_style_t setting_cal_btn_style; // 按钮样式
//   lv_style_init(&setting_cal_btn_style);
//   lv_style_set_radius(&setting_cal_btn_style, CAL_BTN_WIDTH / 2);
//   lv_style_set_text_font(&setting_cal_btn_style, &user_interface_font_20);
//   // lv_style_set_bg_color(&setting_ctl_btn_style, lv_palette_lighten(LV_PALETTE_GREY, 3));
//   // lv_style_set_text_color(&setting_ctl_btn_style, lv_color_black());

//   lv_obj_t *cal_inc_btn = lv_btn_create(setting_if_cal_area); // 创建倾角传感器校准按钮
//   lv_obj_set_size(cal_inc_btn, CAL_BTN_WIDTH, CAL_BTN_HEIGHT);
//   lv_obj_align_to(cal_inc_btn, setting_if_cal_area, LV_ALIGN_CENTER, -SETTING_CTLWID_AREA_WIDTH / 4,
//                   0);
//   lv_obj_add_style(cal_inc_btn, &setting_cal_btn_style, 0);
//   lv_obj_add_event_cb(cal_inc_btn, event_handler_cal_inclination, LV_EVENT_ALL,
//                       NULL);                                  // 添加触发事件
//   lv_obj_t *cal_inc_btn_label = lv_label_create(cal_inc_btn); // 图标描述
//   lv_label_set_text(cal_inc_btn_label, "校准");
//   // lv_obj_add_style(cal_inc_btn_label, &setting_cal_dsc_style, 0);
//   lv_obj_set_style_text_font(cal_inc_btn_label, &user_interface_font_42, 0);
//   lv_obj_align(cal_inc_btn_label, LV_ALIGN_CENTER, 0, 0);
//   lv_obj_t *cal_inc_btn_src_label = lv_label_create(setting_if_cal_area); // 图标描述
//   lv_label_set_text(cal_inc_btn_src_label, "倾角传感器");
//   // lv_obj_add_style(cal_inc_btn_src_label, &setting_cal_btn_dsc_style, 0);
//   lv_obj_set_style_text_font(cal_inc_btn_src_label, &user_interface_font_16, 0);
//   lv_obj_align_to(cal_inc_btn_src_label, cal_inc_btn, LV_ALIGN_OUT_TOP_MID, 0, -5);

//   lv_obj_t *A = lv_obj_create(setting_if_cal_area); // 创建测量值显示区域
// #define INC_A_AREA_WIDTH (SETTING_CTLWID_AREA_WIDTH / 2 - 50)
// #define INC_A_AREA_HEIGHT ((SETTING_CTLWID_AREA_HEIGHT / 6) * 2)
//   lv_obj_set_size(A, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT);
//   lv_obj_set_style_radius(A, 0, 0);
//   lv_obj_clear_flag(A, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(A, setting_if_cal_area, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 2, 0);

//   lv_obj_t *Aa = lv_obj_create(A);
//   lv_obj_set_size(Aa, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT / 2);
//   lv_obj_set_style_radius(Aa, 0, 0);
//   lv_obj_clear_flag(Aa, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Aa, A, LV_ALIGN_CENTER, 0, -INC_A_AREA_HEIGHT / 4);
//   lv_obj_t *Aa1 = lv_obj_create(Aa);
//   lv_obj_set_size(Aa1, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 2);
//   lv_obj_set_style_radius(Aa1, 0, 0);
//   lv_obj_clear_flag(Aa1, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Aa1, Aa, LV_ALIGN_CENTER, -INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *Aa2 = lv_obj_create(Aa);
//   lv_obj_set_size(Aa2, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 2);
//   lv_obj_set_style_radius(Aa2, 0, 0);
//   lv_obj_clear_flag(Aa2, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Aa2, Aa, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *cal_inc_x_dsc_label = lv_label_create(Aa1); // 图标描述
//   lv_label_set_text(cal_inc_x_dsc_label, "x(度数):");
//   lv_obj_set_style_text_font(cal_inc_x_dsc_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_inc_x_dsc_label);
//   cal_inc_x_label = lv_label_create(Aa2); // 图标描述
//   lv_label_set_text(cal_inc_x_label, "00.00");
//   lv_obj_set_style_text_font(cal_inc_x_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_inc_x_label);

//   lv_obj_t *Ab = lv_obj_create(A);
//   lv_obj_set_size(Ab, INC_A_AREA_WIDTH, INC_A_AREA_HEIGHT / 2);
//   lv_obj_set_style_radius(Ab, 0, 0);
//   lv_obj_clear_flag(Ab, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ab, A, LV_ALIGN_CENTER, 0, INC_A_AREA_HEIGHT / 4);
//   lv_obj_t *Ab1 = lv_obj_create(A);
//   lv_obj_set_size(Ab1, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 2);
//   lv_obj_set_style_radius(Ab1, 0, 0);
//   lv_obj_clear_flag(Ab1, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ab1, Ab, LV_ALIGN_CENTER, -INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *Ab2 = lv_obj_create(A);
//   lv_obj_set_size(Ab2, INC_A_AREA_WIDTH / 2, INC_A_AREA_HEIGHT / 2);
//   lv_obj_set_style_radius(Ab2, 0, 0);
//   lv_obj_clear_flag(Ab2, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_align_to(Ab2, Ab, LV_ALIGN_CENTER, INC_A_AREA_WIDTH / 4, 0);
//   lv_obj_t *cal_inc_y_dsc_label = lv_label_create(Ab1); // 图标描述
//   lv_label_set_text(cal_inc_y_dsc_label, "y(度数):");
//   lv_obj_set_style_text_font(cal_inc_y_dsc_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_inc_y_dsc_label);
//   cal_inc_y_label = lv_label_create(Ab2); // 图标描述
//   lv_label_set_text(cal_inc_y_label, "00.00");
//   lv_obj_set_style_text_font(cal_inc_y_label, &user_interface_font_16, 0);
//   lv_obj_center(cal_inc_y_label);

//   lv_obj_t *right_btn = lv_btn_create(setting_if_cal_area);
//   static lv_style_t right_btn_style;
//   lv_style_init(&right_btn_style);
//   lv_style_set_radius(&right_btn_style, 0);
//   lv_style_set_text_font(&right_btn_style, &lv_font_montserrat_16);
//   lv_style_set_bg_color(&right_btn_style, lv_palette_lighten(LV_PALETTE_GREY, 3));
//   lv_style_set_text_color(&right_btn_style, lv_color_black());
//   lv_obj_add_style(right_btn, &right_btn_style, 0);
//   lv_obj_set_size(right_btn, TABVIEW_TAB_H, SETTING_CTLWID_AREA_HEIGHT);
//   lv_obj_align_to(right_btn, setting_if_cal_area, LV_ALIGN_RIGHT_MID, 20, 0);
//   lv_obj_add_event_cb(right_btn, cal_menu_right_event_handler, LV_EVENT_ALL, NULL);
//   lv_obj_t *right_icon = lv_label_create(right_btn);
//   lv_obj_set_style_text_font(right_icon, &lv_font_montserrat_16, 0);
//   lv_label_set_text(right_icon, LV_SYMBOL_RIGHT);
//   lv_obj_center(right_icon);

//   g_setting_if_inc_cal_area = setting_if_cal_area;
//   g_setting_cal_cur_id = SETTING_CAL_INC_ID;
// #if !USE_SDL_SIM
//   g_set_cal_inc_timer = lv_timer_create(set_cal_inc_timer_cb, 500, NULL); // 前后台配置数据同步
// #else
//   g_set_cal_inc_timer = lv_timer_create(set_cal_inc_timer_cb, 500, NULL); // 前后台配置数据同步
// #endif
//   navbar_foreground(true, true);
// }

// static void setting_if_inc_cal_delete(void)
// {
//   // #if !USE_SDL_SIM
//   if (g_set_cal_inc_timer != NULL)
//   {
//     lv_timer_del(g_set_cal_inc_timer);
//     g_set_cal_inc_timer = NULL;
//   }
//   // #endif
//   if (g_setting_if_inc_cal_area != NULL)
//   {
//     lv_obj_del(g_setting_if_inc_cal_area);
//     g_setting_if_inc_cal_area = NULL;
//   }
// }

// static void event_handler_cal_inclination(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     // 倾角状态传感器 校准触发
//     LV_LOG_USER("cal menu inc btn clicked\n");
// #if !USE_SDL_SIM
//     if (threadid_data != NULL)
//     {
//       LSAPI_OSI_Event_t send_event;
//       send_event.id = UI_EVENT_CAL_INC_BTN_ID;
//       send_event.param1 = (uint32_t)0;
//       LSAPI_OSI_EvnetSend(threadid_data, &send_event);
//     }
// #endif
//   }
// }
// // 传感器校准界面
// static void setting_interface_calibration_create(lv_obj_t *parent)
// {
//   setting_interface_menu_head_create(parent, "传感器校准"); // 创建头
//   setting_if_inc_cal_create(parent);                        // 默认倾角传感器校准界面
//   g_setting_if_cal_area = g_setting_if_inc_cal_area;
//   g_setting_btn_cur_id = SETTING_CALIBRATION_ID;
//   navbar_foreground(true, true);
// }
// static void setting_interface_calibration_delete(void)
// {
//   // 判断当前界面id
//   /*
//   SETTING_CAL_INC_ID = 0,  // 倾角传感器校准界面
//   SETTING_CAL_WEIGH_ID = 1, // 称重传感器校准界面
//   SETTING_CAL_FLOW1_FREQ_ID = 2, // 流量计1频率模式校准界面
//   SETTING_CAL_FLOW2_FREQ_ID = 3, // 流量计2频率模式校准界面
//   SETTING_CAL_FLOW1_ELEC_ID = 4, // 流量计1电流模式校准界面
//   SETTING_CAL_FLOW2_ELEC_ID = 5, // 流量计2电流模式校准界面
//   */
//   switch (g_setting_cal_cur_id)
//   {
//   case SETTING_CAL_INC_ID:
//     g_setting_if_cal_area = g_setting_if_inc_cal_area;
//     if (g_set_cal_inc_timer != NULL)
//     {
//       lv_timer_del(g_set_cal_inc_timer);
//       g_set_cal_inc_timer = NULL;
//     }
//     break;
//   case SETTING_CAL_WEIGH_ID:
//     g_setting_if_cal_area = g_setting_if_weigh_cal_area;
//     break;
//   case SETTING_CAL_FLOW1_FREQ_ID:
//     g_setting_if_cal_area = g_setting_if_flow1_freq_cal_area;
//     break;
//   case SETTING_CAL_FLOW2_FREQ_ID:
//     g_setting_if_cal_area = g_setting_if_flow2_freq_cal_area;
//     break;
//   case SETTING_CAL_FLOW1_ELEC_ID:
//     g_setting_if_cal_area = g_setting_if_flow1_elec_cal_area;
//     break;
//   case SETTING_CAL_FLOW2_ELEC_ID:
//     g_setting_if_cal_area = g_setting_if_flow2_elec_cal_area;
//     break;
//   }
//   if (g_setting_if_cal_area != NULL)
//   {
//     lv_obj_del(g_setting_if_cal_area);
//     g_setting_if_cal_area = NULL;
//   }
//   setting_interface_menu_head_delete();
// }

// static void radio_event_handler(lv_event_t *e)
// {
//   uint32_t *active_id = lv_event_get_user_data(e);
//   lv_obj_t *cont = lv_event_get_current_target(e);
//   lv_obj_t *act_cb = lv_event_get_target(e);
//   lv_obj_t *old_cb = lv_obj_get_child(cont, *active_id);
//   if (act_cb == cont)
//     return;
//   lv_obj_clear_state(old_cb, LV_STATE_CHECKED);
//   lv_obj_add_state(act_cb, LV_STATE_CHECKED);
//   *active_id = lv_obj_get_index(act_cb);
//   LV_LOG_USER("Selected radio buttons: %d, %d, -->> *active_id:%d", (int)active_index_1,
//               (int)active_index_2, *active_id);
//   g_sys_cfg.setting_flow_pin = *active_id;
// }
// // static uint8_t g_setting_flow_mode_state = 0;  // 默认正: 0,反：1
// static lv_obj_t *flow_mode_btn_label;
// static void event_handler_flow_dir(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     g_sys_cfg.setting_flow_mode = !g_sys_cfg.setting_flow_mode;
//     if (g_sys_cfg.setting_flow_mode)
//     {
//       lv_label_set_text(flow_mode_btn_label, "电流");
//     }
//     else
//     {
//       lv_label_set_text(flow_mode_btn_label, "频率");
//     }
//   }
// }
// static lv_obj_t *radiobutton_create(lv_obj_t *parent, const char *txt)
// {
//   lv_obj_t *obj = lv_checkbox_create(parent);
//   lv_checkbox_set_text(obj, txt);
//   lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
//   lv_obj_add_style(obj, &pps_style_radio, LV_PART_INDICATOR);
//   lv_obj_add_style(obj, &pps_style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
//   lv_obj_center(obj);
//   return obj;
// }
// @note: 创建流量计参数配置界面
// static void setting_interface_flow_create(lv_obj_t *parent)
// {
//   setting_interface_menu_head_create(parent, "流量计");
//   lv_obj_t *setting_if_flow_area = lv_obj_create(parent);
//   lv_obj_set_size(setting_if_flow_area, SETTING_CTLWID_AREA_WIDTH, SETTING_CTLWID_AREA_HEIGHT);
//   lv_obj_set_style_radius(setting_if_flow_area, 0, 0);
//   lv_obj_align_to(setting_if_flow_area, g_setting_if_menu_head, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
//   lv_obj_clear_flag(setting_if_flow_area, LV_OBJ_FLAG_SCROLLABLE);
// #define FLOW_BTN_WIDTH 100
// #define FLOW_BTN_HEIGHT 100
//   static lv_style_t setting_flow_btn_style; // 按钮样式
//   lv_style_init(&setting_flow_btn_style);
//   lv_style_set_radius(&setting_flow_btn_style, FLOW_BTN_WIDTH / 2);
//   lv_style_set_text_font(&setting_flow_btn_style, &user_interface_font_20);
//   // lv_style_set_bg_color(&setting_ctl_btn_style, lv_palette_lighten(LV_PALETTE_GREY, 3));
//   // lv_style_set_text_color(&setting_ctl_btn_style, lv_color_black());

//   // 1
//   lv_obj_t *A = lv_obj_create(setting_if_flow_area); // 创建A区
//   lv_obj_set_style_radius(A, 0, 0);
// #define A_AREA_WIDTH (SETTING_CTLWID_AREA_WIDTH)
// #define A_AREA_HEIGHT (SETTING_CTLWID_AREA_HEIGHT / 2 + 30)
//   lv_obj_set_size(A, A_AREA_WIDTH, A_AREA_HEIGHT);
//   lv_obj_align_to(A, setting_if_flow_area, LV_ALIGN_CENTER, 0,
//                   -(SETTING_CTLWID_AREA_HEIGHT / 2 - A_AREA_HEIGHT / 2));
//   lv_obj_clear_flag(A, LV_OBJ_FLAG_SCROLLABLE);

//   lv_obj_t *dir_btn = lv_btn_create(A);
//   lv_obj_set_size(dir_btn, FLOW_BTN_WIDTH, FLOW_BTN_HEIGHT);
//   lv_obj_align_to(dir_btn, A, LV_ALIGN_CENTER, 0, 15);
//   lv_obj_add_style(dir_btn, &setting_flow_btn_style, 0);
//   lv_obj_add_event_cb(dir_btn, event_handler_flow_dir, LV_EVENT_ALL, NULL); // 添加触发事件
//   flow_mode_btn_label = lv_label_create(dir_btn);                           // 图标描述
//   if (g_sys_cfg.setting_flow_mode == 0)
//   {
//     lv_label_set_text(flow_mode_btn_label, "频率");
//   }
//   else if (g_sys_cfg.setting_flow_mode == 1)
//   {
//     lv_label_set_text(flow_mode_btn_label, "电流");
//   }
//   else
//   {
//     lv_label_set_text(flow_mode_btn_label, "频率");
//   }
//   lv_obj_set_style_text_font(flow_mode_btn_label, &user_interface_font_42, 0);
//   lv_obj_align(flow_mode_btn_label, LV_ALIGN_CENTER, 0, 0);
//   lv_obj_t *dir_btn_src_label = lv_label_create(setting_if_flow_area); // 图标描述
//   lv_label_set_text(dir_btn_src_label, "流量计模式");
//   lv_obj_set_style_text_font(dir_btn_src_label, &user_interface_font_16, 0);
//   lv_obj_align(dir_btn_src_label, LV_ALIGN_CENTER, 0, -SETTING_CTLWID_AREA_HEIGHT / 4 - 33);

//   // 4
//   lv_obj_t *B = lv_obj_create(setting_if_flow_area); // 创建A区
//   lv_obj_set_style_radius(B, 0, 0);
// #define B_AREA_WIDTH (SETTING_CTLWID_AREA_WIDTH)
// #define B_AREA_HEIGHT (SETTING_CTLWID_AREA_HEIGHT - (A_AREA_HEIGHT))
//   lv_obj_set_size(B, B_AREA_WIDTH, B_AREA_HEIGHT);
//   lv_obj_align_to(B, A, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
//   lv_obj_clear_flag(B, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_t *pulse_pin_sl_lable = lv_label_create(B);
//   lv_obj_set_style_text_font(pulse_pin_sl_lable, &user_interface_font_16, 0);
//   lv_label_set_text(pulse_pin_sl_lable, "脉冲引脚选择:");
//   lv_obj_align_to(pulse_pin_sl_lable, B, LV_ALIGN_CENTER, -SETTING_CTLWID_AREA_WIDTH / 4, 0);

//   lv_style_init(&pps_style_radio);
//   lv_style_set_radius(&pps_style_radio, LV_RADIUS_CIRCLE);
//   lv_style_init(&pps_style_radio_chk);
//   lv_style_set_bg_img_src(&pps_style_radio_chk, NULL);

//   lv_obj_t *cont2 = lv_obj_create(B);               // radio区域
//   lv_obj_clear_flag(cont2, LV_OBJ_FLAG_SCROLLABLE); // 禁止滚动条
//   lv_obj_set_flex_flow(cont2, LV_FLEX_FLOW_ROW);
//   lv_obj_set_size(cont2, 150, 50);
//   lv_obj_add_event_cb(cont2, radio_event_handler, LV_EVENT_CLICKED, &active_index_2);
//   uint32_t i;
//   char buf[32];
//   for (i = 0; i < 2; i++)
//   {
//     lv_snprintf(buf, sizeof(buf), ":CH%d", (int)i + 1);
//     radiobutton_create(cont2, buf);
//   }
//   if (g_sys_cfg.setting_flow_pin == 0)
//   {
//     lv_obj_add_state(lv_obj_get_child(cont2, 0), LV_STATE_CHECKED);
//   }
//   else if (g_sys_cfg.setting_flow_pin == 1)
//   {
//     lv_obj_add_state(lv_obj_get_child(cont2, 1), LV_STATE_CHECKED);
//   }
//   else
//   {
//     lv_obj_add_state(lv_obj_get_child(cont2, 0), LV_STATE_CHECKED);
//   }
//   lv_obj_align_to(cont2, pulse_pin_sl_lable, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

//   g_setting_btn_cur_id = SETTING_FLOW_METER_ID;
//   g_setting_if_flow_area = setting_if_flow_area;
//   navbar_foreground(true, true);
// }

// static void setting_interface_flow_delete(void)
// {
//   if (g_setting_if_flow_area != NULL)
//   {
//     lv_obj_del(g_setting_if_flow_area);
//     g_setting_if_flow_area = NULL;
//   }
//   setting_interface_menu_head_delete();
// }

// static void lv_spinbox_increment_event_cb(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
//   {
//     lv_spinbox_increment(g_slist_config_spinbox);
//     g_sys_cfg.setting_depth_coefficient = lv_spinbox_get_value(g_slist_config_spinbox);
//     LV_LOG_USER("depth gauge spibox increment num:%d", g_sys_cfg.setting_depth_coefficient);
//   }
// }

// static void drs_spinbox_increment_event_cb(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
//   {
//     lv_spinbox_increment(g_depth_reporting_spinbox);
//     g_sys_cfg.setting_depth_threshold = lv_spinbox_get_value(g_depth_reporting_spinbox);
//     LV_LOG_USER("depth reporting threshold spibox increment num:%d",
//                 g_sys_cfg.setting_depth_threshold);
//   }
// }

// static void lv_spinbox_decrement_event_cb(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
//   {
//     lv_spinbox_decrement(g_slist_config_spinbox);
//     g_sys_cfg.setting_depth_coefficient = lv_spinbox_get_value(g_slist_config_spinbox);
//     LV_LOG_USER("depth gauge spibox decrement num:%d", g_sys_cfg.setting_depth_coefficient);
//   }
// }

// static void drs_spinbox_decrement_event_cb(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
//   {
//     lv_spinbox_decrement(g_depth_reporting_spinbox);
//     g_sys_cfg.setting_depth_threshold = lv_spinbox_get_value(g_depth_reporting_spinbox);
//     LV_LOG_USER("depth reporting threshold spibox decrement num:%d",
//                 g_sys_cfg.setting_depth_threshold);
//   }
// }

// static lv_obj_t *dir_btn_label;
// static void event_handler_depth_dir(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     g_sys_cfg.setting_depth_dir = !g_sys_cfg.setting_depth_dir;
//     if (g_sys_cfg.setting_depth_dir == 1)
//     {
//       lv_label_set_text(dir_btn_label, "反");
//     }
//     else
//     {
//       lv_label_set_text(dir_btn_label, "正");
//     }
//   }
// }

// static lv_obj_t *mode_btn_label;
// static void event_handler_depth_mode(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     g_sys_cfg.setting_depth_mode = !g_sys_cfg.setting_depth_mode;
//     if (g_sys_cfg.setting_depth_mode == 1)
//     {
//       lv_label_set_text(mode_btn_label, "脉冲");
//     }
//     else
//     {
//       lv_label_set_text(mode_btn_label, "增量");
//     }
//   }
// }

// // @note: 创建深度计参数配置界面
// static void setting_interface_depth_create(lv_obj_t *parent)
// {
//   setting_interface_menu_head_create(parent, "深度计");
//   lv_obj_t *setting_if_depth_area = lv_obj_create(parent);
//   lv_obj_set_size(setting_if_depth_area, SETTING_CTLWID_AREA_WIDTH, SETTING_CTLWID_AREA_HEIGHT);
//   lv_obj_set_style_radius(setting_if_depth_area, 0, 0);
//   lv_obj_align_to(setting_if_depth_area, g_setting_if_menu_head, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
//   lv_obj_clear_flag(setting_if_depth_area, LV_OBJ_FLAG_SCROLLABLE);
// #define DEPTH_BTN_WIDTH 100
// #define DEPTH_BTN_HEIGHT 100
//   static lv_style_t setting_depth_btn_style; // 按钮样式
//   lv_style_init(&setting_depth_btn_style);
//   lv_style_set_radius(&setting_depth_btn_style, DEPTH_BTN_WIDTH / 2);
//   lv_style_set_text_font(&setting_depth_btn_style, &user_interface_font_20);
//   // lv_style_set_bg_color(&setting_depth_btn_style, lv_palette_lighten(LV_PALETTE_GREY, 3));
//   // lv_style_set_text_color(&setting_depth_btn_style, lv_color_black());

//   // 1
//   lv_obj_t *A = lv_obj_create(setting_if_depth_area); // 创建A区
//   lv_obj_set_style_radius(A, 0, 0);
// #define A_AREA_WIDTH (SETTING_CTLWID_AREA_WIDTH / 2)
// #define A_AREA_HEIGHT (SETTING_CTLWID_AREA_HEIGHT / 2 + 30)
//   lv_obj_set_size(A, A_AREA_WIDTH, A_AREA_HEIGHT);
//   lv_obj_align_to(A, setting_if_depth_area, LV_ALIGN_CENTER, -(SETTING_CTLWID_AREA_WIDTH / 4),
//                   -(SETTING_CTLWID_AREA_HEIGHT / 2 - A_AREA_HEIGHT / 2));
//   lv_obj_clear_flag(A, LV_OBJ_FLAG_SCROLLABLE);

//   lv_obj_t *dir_btn = lv_btn_create(setting_if_depth_area);
//   lv_obj_set_size(dir_btn, DEPTH_BTN_WIDTH, DEPTH_BTN_HEIGHT);
//   lv_obj_align_to(dir_btn, setting_if_depth_area, LV_ALIGN_CENTER, -SETTING_CTLWID_AREA_WIDTH / 4,
//                   -SETTING_CTLWID_AREA_HEIGHT / 4 + 30);
//   lv_obj_add_style(dir_btn, &setting_depth_btn_style, 0);
//   lv_obj_add_event_cb(dir_btn, event_handler_depth_dir, LV_EVENT_ALL, NULL); // 添加触发事件
//   dir_btn_label = lv_label_create(dir_btn);                                  // 图标描述
//   if (g_sys_cfg.setting_depth_dir == 0)
//   {
//     lv_label_set_text(dir_btn_label, "正");
//   }
//   else if (g_sys_cfg.setting_depth_dir == 1)
//   {
//     lv_label_set_text(dir_btn_label, "反");
//   }
//   else
//   {
//     lv_label_set_text(dir_btn_label, "正");
//   }
//   lv_obj_set_style_text_font(dir_btn_label, &user_interface_font_42, 0);
//   lv_obj_align(dir_btn_label, LV_ALIGN_CENTER, 0, 0);
//   lv_obj_t *dir_btn_src_label = lv_label_create(setting_if_depth_area); // 图标描述
//   lv_obj_set_style_text_font(dir_btn_src_label, &user_interface_font_16, 0);
//   lv_label_set_text(dir_btn_src_label, "深度计方向");
//   lv_obj_align(dir_btn_src_label, LV_ALIGN_CENTER, -SETTING_CTLWID_AREA_WIDTH / 4,
//                -SETTING_CTLWID_AREA_HEIGHT / 4 - 33);

//   // 2
//   lv_obj_t *B = lv_obj_create(setting_if_depth_area); // 创建B区
//   lv_obj_set_style_radius(B, 0, 0);
// #define B_AREA_WIDTH (A_AREA_WIDTH)
// #define B_AREA_HEIGHT (A_AREA_HEIGHT)
//   lv_obj_set_size(B, B_AREA_WIDTH, B_AREA_HEIGHT);
//   lv_obj_align_to(B, setting_if_depth_area, LV_ALIGN_CENTER, +(SETTING_CTLWID_AREA_WIDTH / 4),
//                   -(SETTING_CTLWID_AREA_HEIGHT / 2 - B_AREA_HEIGHT / 2));
//   lv_obj_clear_flag(B, LV_OBJ_FLAG_SCROLLABLE);

//   lv_obj_t *mode_btn = lv_btn_create(setting_if_depth_area);
//   lv_obj_set_size(mode_btn, DEPTH_BTN_WIDTH, DEPTH_BTN_HEIGHT);
//   lv_obj_align_to(mode_btn, setting_if_depth_area, LV_ALIGN_CENTER, +SETTING_CTLWID_AREA_WIDTH / 4,
//                   -SETTING_CTLWID_AREA_HEIGHT / 4 + 30);
//   lv_obj_add_style(mode_btn, &setting_depth_btn_style, 0);
//   lv_obj_add_event_cb(mode_btn, event_handler_depth_mode, LV_EVENT_ALL, NULL); // 添加触发事件
//   mode_btn_label = lv_label_create(mode_btn);                                  // 图标描述
//   lv_obj_set_style_text_font(mode_btn_label, &user_interface_font_42, 0);
//   if (g_sys_cfg.setting_depth_mode == 0)
//   {
//     lv_label_set_text(mode_btn_label, "增量");
//   }
//   else if (g_sys_cfg.setting_depth_mode == 1)
//   {
//     lv_label_set_text(mode_btn_label, "脉冲");
//   }
//   else
//   {
//     lv_label_set_text(mode_btn_label, "增量");
//   }
//   lv_obj_align(mode_btn_label, LV_ALIGN_CENTER, 0, 0);
//   lv_obj_t *mode_btn_src_label = lv_label_create(setting_if_depth_area); // 图标描述
//   lv_obj_set_style_text_font(mode_btn_src_label, &user_interface_font_16, 0);
//   lv_label_set_text(mode_btn_src_label, "深度计模式");
//   lv_obj_align(mode_btn_src_label, LV_ALIGN_CENTER, SETTING_CTLWID_AREA_WIDTH / 4,
//                -SETTING_CTLWID_AREA_HEIGHT / 4 - 33);

//   // 3
//   lv_obj_t *C = lv_obj_create(setting_if_depth_area);
//   lv_obj_set_style_radius(C, 0, 0);
// #define C_AREA_WIDTH (SETTING_CTLWID_AREA_WIDTH)
// #define C_AREA_HEIGHT ((SETTING_CTLWID_AREA_HEIGHT - (SETTING_CTLWID_AREA_HEIGHT / 2 + 30)) / 2)
//   lv_obj_set_size(C, C_AREA_WIDTH, C_AREA_HEIGHT);
//   lv_obj_align_to(C, setting_if_depth_area, LV_ALIGN_CENTER, 0,
//                   (A_AREA_HEIGHT - (SETTING_CTLWID_AREA_HEIGHT / 2)) + 25);
//   lv_obj_clear_flag(C, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_t *depth_reporting_threshold_s = lv_label_create(C);
//   lv_obj_set_style_text_font(depth_reporting_threshold_s, &user_interface_font_16, 0);
//   lv_label_set_text(depth_reporting_threshold_s, "深度测量值:");
//   lv_obj_align_to(depth_reporting_threshold_s, C, LV_ALIGN_CENTER, -SETTING_CTLWID_AREA_WIDTH / 4,
//                   0);
//   g_depth_reporting_spinbox = lv_spinbox_create(setting_if_depth_area);
//   lv_spinbox_set_range(g_depth_reporting_spinbox, 0, 1000); // 范围0-1000
//   lv_spinbox_set_digit_format(g_depth_reporting_spinbox, 3, 3);
//   lv_spinbox_step_prev(g_depth_reporting_spinbox);
//   lv_obj_set_width(g_depth_reporting_spinbox, 50);
//   lv_obj_set_height(g_depth_reporting_spinbox, lv_obj_get_height(depth_reporting_threshold_s) + 20);
//   lv_obj_align_to(g_depth_reporting_spinbox, depth_reporting_threshold_s, LV_ALIGN_OUT_RIGHT_MID,
//                   15, 0);
//   lv_spinbox_set_value(g_depth_reporting_spinbox, g_sys_cfg.setting_depth_threshold);
//   lv_coord_t h = lv_obj_get_height(g_depth_reporting_spinbox);
//   lv_obj_t *lbtn = lv_btn_create(setting_if_depth_area);
//   lv_obj_set_size(lbtn, h, h);
//   lv_obj_align_to(lbtn, g_depth_reporting_spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
//   lv_obj_set_style_bg_img_src(lbtn, LV_SYMBOL_PLUS, 0);
//   lv_obj_add_event_cb(lbtn, drs_spinbox_increment_event_cb, LV_EVENT_ALL, NULL);
//   lv_obj_t *rbtn = lv_btn_create(setting_if_depth_area);
//   lv_obj_set_size(rbtn, h, h);
//   lv_obj_align_to(rbtn, lbtn, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
//   lv_obj_set_style_bg_img_src(rbtn, LV_SYMBOL_MINUS, 0);
//   lv_obj_add_event_cb(rbtn, drs_spinbox_decrement_event_cb, LV_EVENT_ALL, NULL);

//   // 4
//   lv_obj_t *D = lv_obj_create(setting_if_depth_area);
//   lv_obj_set_style_radius(D, 0, 0);
// #define D_AREA_WIDTH (C_AREA_WIDTH)
// #define D_AREA_HEIGHT (C_AREA_HEIGHT)
//   lv_obj_set_size(D, D_AREA_WIDTH, D_AREA_HEIGHT);
//   lv_obj_align_to(D, setting_if_depth_area, LV_ALIGN_CENTER, 0,
//                   (A_AREA_HEIGHT - (SETTING_CTLWID_AREA_HEIGHT / 2)) + 25 + D_AREA_HEIGHT);
//   lv_obj_clear_flag(D, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_t *depth_gauge_s = lv_label_create(setting_if_depth_area);
//   lv_obj_set_style_text_font(depth_gauge_s, &user_interface_font_16, 0);
//   lv_label_set_text(depth_gauge_s, "深度计系数:");
//   lv_obj_align_to(depth_gauge_s, D, LV_ALIGN_CENTER, -SETTING_CTLWID_AREA_WIDTH / 4, 0);
//   g_slist_config_spinbox = lv_spinbox_create(setting_if_depth_area);
//   lv_spinbox_set_range(g_slist_config_spinbox, 0, 1000); // 范围0-1000
//   lv_spinbox_set_digit_format(g_slist_config_spinbox, 3, 3);
//   lv_spinbox_step_prev(g_slist_config_spinbox);
//   lv_obj_set_width(g_slist_config_spinbox, 50);
//   lv_obj_set_height(g_slist_config_spinbox, lv_obj_get_height(depth_gauge_s) + 20);
//   lv_obj_align_to(g_slist_config_spinbox, depth_gauge_s, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
//   lv_spinbox_set_value(g_slist_config_spinbox, g_sys_cfg.setting_depth_coefficient);
//   h = lv_obj_get_height(g_slist_config_spinbox);
//   lbtn = lv_btn_create(setting_if_depth_area);
//   lv_obj_set_size(lbtn, h, h);
//   lv_obj_align_to(lbtn, g_slist_config_spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
//   lv_obj_set_style_bg_img_src(lbtn, LV_SYMBOL_PLUS, 0);
//   lv_obj_add_event_cb(lbtn, lv_spinbox_increment_event_cb, LV_EVENT_ALL, NULL);
//   rbtn = lv_btn_create(setting_if_depth_area);
//   lv_obj_set_size(rbtn, h, h);
//   lv_obj_align_to(rbtn, lbtn, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
//   lv_obj_set_style_bg_img_src(rbtn, LV_SYMBOL_MINUS, 0);
//   lv_obj_add_event_cb(rbtn, lv_spinbox_decrement_event_cb, LV_EVENT_ALL, NULL);

//   g_setting_btn_cur_id = SETTING_DEPTH_METER_ID;
//   g_setting_if_depth_area = setting_if_depth_area;
//   navbar_foreground(true, true);
// }

// static void setting_interface_depth_delete(void)
// {
//   if (g_setting_if_depth_area != NULL)
//     lv_obj_del(g_setting_if_depth_area);
//   setting_interface_menu_head_delete(); // 删除界面头
// }

// static void setting_night_event_handler(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_VALUE_CHANGED)
//   {
//     LV_LOG_USER("setting -> system -> night btn = state: %s\n",
//                 lv_obj_has_state(obj, LV_STATE_CHECKED) ? "on" : "off");
//     lv_obj_t *night_lable = (lv_obj_t *)lv_event_get_user_data(e);
//     if (lv_obj_has_state(obj, LV_STATE_CHECKED))
//     {
//       lv_label_set_text(night_lable, "深色");
//       g_sys_cfg.setting_theme_mode = 1;
// #if LV_USE_THEME_DEFAULT
//       lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
//                             g_sys_cfg.setting_theme_mode, &lv_font_montserrat_12);
// #endif
//     }
//     else
//     {
//       lv_label_set_text(night_lable, "浅色");
//       g_sys_cfg.setting_theme_mode = 0;
// #if LV_USE_THEME_DEFAULT
//       lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
//                             g_sys_cfg.setting_theme_mode, &lv_font_montserrat_12);
// #endif
//     }
//   }
// }

// static void setting_interface_system_create(lv_obj_t *parent)
// {
//   setting_interface_menu_head_create(parent, "系统");
//   lv_obj_t *setting_if_system_area = lv_obj_create(parent);
//   lv_obj_set_size(setting_if_system_area, SETTING_CTLWID_AREA_WIDTH, SETTING_CTLWID_AREA_HEIGHT);
//   lv_obj_align_to(setting_if_system_area, g_setting_if_menu_head, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

//   lv_obj_t *theme_mode_lable = lv_label_create(setting_if_system_area);
//   lv_obj_set_style_text_font(theme_mode_lable, &user_interface_font_16, 0);
//   lv_label_set_text(theme_mode_lable, "主题模式:");
//   lv_obj_align_to(theme_mode_lable, setting_if_system_area, LV_ALIGN_TOP_LEFT, 0, 0);

//   lv_obj_t *set_sys_night_sw = lv_switch_create(setting_if_system_area);
//   lv_obj_align_to(set_sys_night_sw, theme_mode_lable, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

//   if (g_sys_cfg.setting_theme_mode == 0)
//   { // 默认浅色模式
//     ;
//   }
//   else if (g_sys_cfg.setting_theme_mode == 1)
//   {
//     lv_obj_add_state(set_sys_night_sw, LV_STATE_CHECKED);
//   }

//   lv_obj_t *night_lable = lv_label_create(setting_if_system_area);
//   lv_obj_set_style_text_font(night_lable, &user_interface_font_16, 0);
//   lv_label_set_text(night_lable, "浅色"); // 默认浅色主题
//   lv_obj_align_to(night_lable, set_sys_night_sw, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
//   lv_obj_add_event_cb(set_sys_night_sw, setting_night_event_handler, LV_EVENT_ALL, night_lable);

//   g_setting_if_system_area = setting_if_system_area;
//   g_setting_btn_cur_id = SETTING_SYSTEM_ID;
// }
// static void setting_interface_system_delete(void)
// {
//   if (g_setting_if_system_area != NULL)
//     lv_obj_del(g_setting_if_system_area);
//   setting_interface_menu_head_delete();
// }
// static void setting_interface_user1_create(lv_obj_t *parent)
// {
//   setting_interface_menu_head_create(parent, "用户1");
//   g_setting_btn_cur_id = SETTING_USER1_ID;
// }
// static void setting_interface_user1_delete(void)
// {
//   setting_interface_menu_head_delete();
// }
// static void setting_interface_user2_create(lv_obj_t *parent)
// {
//   setting_interface_menu_head_create(parent, "关于");
//   lv_obj_t *setting_if_about_area = lv_obj_create(parent);
//   lv_obj_set_size(setting_if_about_area, SETTING_CTLWID_AREA_WIDTH, SETTING_CTLWID_AREA_HEIGHT);
//   lv_obj_align_to(setting_if_about_area, g_setting_if_menu_head, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

//   lv_obj_t *logo_lable = lv_label_create(setting_if_about_area);
//   lv_obj_set_style_text_font(logo_lable, &lv_font_montserrat_24, 0);
//   lv_label_set_text(logo_lable, (const char *)"MCXA");
//   lv_obj_align_to(logo_lable, setting_if_about_area, LV_ALIGN_CENTER, 0, -60);

//   lv_obj_t *system_version_lable = lv_label_create(setting_if_about_area);
//   lv_obj_set_style_text_font(system_version_lable, &user_interface_font_16, 0);
//   char info[64] = {0};
//   memset(info, 0x00, sizeof(info));
//   sprintf(info, "系统版本:%s", "0.0.1");
//   lv_label_set_text(system_version_lable, (const char *)info);
//   lv_obj_align_to(system_version_lable, setting_if_about_area, LV_ALIGN_CENTER, 0, -20);

//   lv_obj_t *interface_version_lable = lv_label_create(setting_if_about_area);
//   lv_obj_set_style_text_font(interface_version_lable, &user_interface_font_16, 0);
//   memset(info, 0x00, sizeof(info));
//   sprintf(info, "界面版本:%d.%d.%d-%s", UI_VERSION_MAJOR, UI_VERSION_MINOR, UI_VERSION_PATCH,
//           UI_VERSION_INFO);
//   lv_label_set_text(interface_version_lable, (const char *)info);
//   lv_obj_align_to(interface_version_lable, setting_if_about_area, LV_ALIGN_CENTER, 0, 0);

//   lv_obj_t *data_version_lable = lv_label_create(setting_if_about_area);
//   lv_obj_set_style_text_font(data_version_lable, &user_interface_font_16, 0);
//   memset(info, 0x00, sizeof(info));
//   sprintf(info, "后台版本:%s", "0.0.1");
//   lv_label_set_text(data_version_lable, info);
//   lv_obj_align_to(data_version_lable, setting_if_about_area, LV_ALIGN_CENTER, 0, 20);
//   g_setting_if_user2_area = setting_if_about_area;
//   g_setting_btn_cur_id = SETTING_USER2_ID;
// }
// static void setting_interface_user2_delete(void)
// {
//   if (g_setting_if_user2_area != NULL)
//     lv_obj_del(g_setting_if_user2_area);
//   setting_interface_menu_head_delete(); // 删除界面头
// }

// static void event_handler_system(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     // 创建系统配置界面
//     lv_obj_t *parent = lv_obj_get_parent(g_setting_title_head);
//     setting_interface_menu_delete();
//     setting_interface_system_create(parent);
//     g_setting_btn_cur_id = SETTING_SYSTEM_ID;
//   }
// }
// static void event_handler_user1(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     lv_obj_t *parent = lv_obj_get_parent(g_setting_title_head);
//     setting_interface_menu_delete();
//     setting_interface_user1_create(parent);
//     g_setting_btn_cur_id = SETTING_USER1_ID;
//   }
// }
// static void event_handler_user2(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     lv_obj_t *parent = lv_obj_get_parent(g_setting_title_head);
//     setting_interface_menu_delete();
//     setting_interface_user2_create(parent);
//     g_setting_btn_cur_id = SETTING_USER2_ID;
//   }
// }

// static void event_handler_depth_meter(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     // 创建深度计参数配置界面
//     lv_obj_t *parent = lv_obj_get_parent(g_setting_title_head);
//     setting_interface_menu_delete();
//     setting_interface_depth_create(parent);
//     g_setting_btn_cur_id = SETTING_DEPTH_METER_ID;
//   }
// }

// static void event_handler_flow_meter(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     lv_obj_t *parent = lv_obj_get_parent(g_setting_title_head);
//     setting_interface_menu_delete();
//     setting_interface_flow_create(parent); // 创建流量计参数配置界面
//     g_setting_btn_cur_id = SETTING_FLOW_METER_ID;
//   }
// }

// static void event_handler_calibration(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     lv_obj_t *parent = lv_obj_get_parent(g_setting_title_head);
//     setting_interface_menu_delete();
//     setting_interface_calibration_create(parent); // 创建校准界面
//     g_setting_btn_cur_id = SETTING_CALIBRATION_ID;
//   }
// }

// // 创建menu界面
// static void setting_interface_menu_create(lv_obj_t *parent)
// {
//   lv_obj_t *setting_ctl_wid_area = lv_obj_create(parent);
//   lv_obj_set_size(setting_ctl_wid_area,
//                   SETTING_CTLWID_AREA_WIDTH, // setting按钮区域宽度
//                   SETTING_CTLWID_AREA_HEIGHT // setting按钮区域高度
//   );
//   lv_obj_align_to(setting_ctl_wid_area, g_setting_title_head, LV_ALIGN_OUT_BOTTOM_MID, 0,
//                   SETTING_Y_OFFSET * 6);
//   lv_obj_set_flex_align(setting_ctl_wid_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_EVENLY,
//                         LV_FLEX_ALIGN_SPACE_BETWEEN);
//   static lv_style_t setting_ctl_btn_style; // menu按钮样式
//   lv_style_init(&setting_ctl_btn_style);
//   lv_style_set_radius(&setting_ctl_btn_style, 0);
//   lv_style_set_text_font(&setting_ctl_btn_style, &lv_font_montserrat_12);
//   static lv_coord_t setting_col_dsc[] = {SETTING_BTN_WIDTH, SETTING_BTN_WIDTH, SETTING_BTN_WIDTH,
//                                          LV_GRID_TEMPLATE_LAST};
//   static lv_coord_t setting_row_dsc[] = {SETTING_BTN_HEIGHT, SETTING_BTN_HEIGHT, SETTING_BTN_HEIGHT,
//                                          LV_GRID_TEMPLATE_LAST};
//   lv_obj_set_style_grid_column_dsc_array(setting_ctl_wid_area, setting_col_dsc, 0);
//   lv_obj_set_style_grid_row_dsc_array(setting_ctl_wid_area, setting_row_dsc, 0);
//   lv_obj_set_layout(setting_ctl_wid_area, LV_LAYOUT_GRID);

//   uint16_t i;
//   for (i = 0; i < SETTING_MENU_ID_MAX - 1; i++)
//   {
//     uint8_t col = i % 3;
//     uint8_t row = i / 3;
//     lv_obj_t *obj;
//     lv_obj_t *icon_lable;
//     lv_obj_t *label;
//     obj = lv_btn_create(setting_ctl_wid_area); // menu按钮
//     lv_obj_add_style(obj, &setting_ctl_btn_style, 0);
//     lv_obj_add_event_cb(obj, g_setting_btn_event[i], LV_EVENT_ALL, NULL); // 添加触发事件
//     icon_lable = lv_label_create(obj);                                    // 图标
//     lv_label_set_text(icon_lable, g_setting_btn_icon[i]);
//     lv_obj_center(icon_lable);
//     lv_obj_set_style_text_font(icon_lable, &setting_interface_icon_24, 0);
//     label = lv_label_create(obj); // 图标描述
//     lv_label_set_text(label, g_setting_btn_dsc[i]);
//     lv_obj_align(label, LV_ALIGN_CENTER, 0, 30);
//     lv_obj_set_style_text_font(label, &user_interface_font_16, 0);
//     lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
//   }
//   g_setting_btn_cur_id = SETTING_DEF_ID;
//   g_setting_ctl_wid_area = setting_ctl_wid_area;
//   navbar_foreground(true, true);
// }

// static void setting_interface_menu_delete(void)
// {
//   if (g_setting_ctl_wid_area != NULL)
//   {
//     lv_obj_del(g_setting_ctl_wid_area);
//     g_setting_ctl_wid_area = NULL;
//   }
// }

// // 菜单头返回按钮回调
// static void setting_menu_back_event_handler(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     LV_LOG_USER("setting menu back btn clicked\n");
//     lv_obj_t *parent = lv_obj_get_parent(g_setting_if_menu_head);
//     if (g_set_if_menu_del[g_setting_btn_cur_id] != NULL)
//     {
//       g_set_if_menu_del[g_setting_btn_cur_id](); // 删除当前界面和界面头
//     }
//     setting_interface_menu_create(parent); // 创建默认菜单界面
//     g_setting_btn_cur_id = SETTING_DEF_ID;
//   }
// }

// // 创建菜单头
// static void setting_interface_menu_head_create(lv_obj_t *parent, const char *dsc)
// {
//   lv_obj_t *menu_h = lv_obj_create(parent);
//   lv_obj_clear_flag(menu_h, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_set_size(menu_h, MONITOR_WIDTH, TABVIEW_TAB_H);
//   lv_obj_align_to(menu_h, g_setting_title_head, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
//   lv_obj_set_style_radius(menu_h, 0, 0);
//   lv_obj_t *menu_h_head_label = lv_label_create(menu_h);
//   lv_obj_set_style_text_font(menu_h_head_label, &user_interface_font_16, 0);
//   lv_label_set_text(menu_h_head_label, dsc);
//   lv_obj_align_to(menu_h_head_label, menu_h, LV_ALIGN_RIGHT_MID, 0, 0);
//   lv_obj_t *back_btn = lv_btn_create(menu_h);
// #define SETTING_MENU_HD_HEIGHT (TABVIEW_TAB_H - 10)
// #define SETTING_MENU_HD_WIDTH (SETTING_MENU_HD_HEIGHT)
//   lv_obj_set_style_radius(back_btn, (SETTING_MENU_HD_HEIGHT) / 2, 0);
//   lv_obj_set_style_text_font(back_btn, &lv_font_montserrat_12, 0);
//   lv_obj_set_size(back_btn, SETTING_MENU_HD_WIDTH, SETTING_MENU_HD_HEIGHT);
//   lv_obj_align_to(back_btn, menu_h, LV_ALIGN_LEFT_MID, -5, -1);
//   lv_obj_add_event_cb(back_btn, setting_menu_back_event_handler, LV_EVENT_ALL, NULL);
//   lv_obj_t *back_icon = lv_label_create(back_btn);
//   lv_obj_set_style_text_font(back_icon, &lv_font_montserrat_16, 0);
//   lv_label_set_text(back_icon, LV_SYMBOL_LEFT);
//   lv_obj_center(back_icon);
//   g_setting_if_menu_head = menu_h;
// }

// static void setting_interface_menu_head_delete(void)
// {
//   if (g_setting_if_menu_head != NULL)
//   {
//     lv_obj_del(g_setting_if_menu_head);
//     g_setting_if_menu_head = NULL;
//   }
// }

// static void setting_close_event_handler(lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);
//   // lv_obj_t *obj = lv_event_get_target(e);
//   if (code == LV_EVENT_CLICKED)
//   {
//     if (g_set_if_menu_del[g_setting_btn_cur_id] != NULL)
//     {                                            // 先检查子页面是否删除
//       g_set_if_menu_del[g_setting_btn_cur_id](); // 删除当前界面和界面头
//     }
//     setting_delete(lv_scr_act());
//     g_setting_if_state = 0;
//     LV_LOG_USER("setting close btn clicked\n");
//     if (g_user_interface[g_interface_id] != NULL)
//     {
//       g_user_interface[g_interface_id](lv_scr_act());
//       LV_LOG_USER("and then create %d interface\n", g_interface_id);
//     }
//   }
// }

// 根据界面id创建界面标识头
// static void setting_interface_title_create(lv_obj_t *parent)
// {
//   lv_obj_t *uih = lv_obj_create(parent);
//   lv_obj_clear_flag(uih, LV_OBJ_FLAG_SCROLLABLE); // 禁止滚动条
//   lv_obj_set_size(uih, MONITOR_WIDTH, TABVIEW_TAB_H);
//   lv_obj_set_style_radius(uih, 0, 0);
//   lv_obj_t *user_if_head_label = lv_label_create(uih);
//   lv_obj_set_style_text_font(user_if_head_label, &user_interface_font_16, 0);
//   lv_label_set_text(user_if_head_label, "设置界面");
//   lv_obj_center(user_if_head_label);
//   lv_obj_t *icon = lv_label_create(uih);
//   lv_obj_set_style_text_font(icon, &lv_font_montserrat_16, 0);
//   lv_label_set_text(icon, LV_SYMBOL_SETTINGS);
//   lv_obj_align_to(icon, uih, LV_ALIGN_LEFT_MID, 0, 0);
// #define SETTING_LB_HEIGHT (TABVIEW_TAB_H - 10)
// #define SETTING_LB_WIDTH (SETTING_LB_HEIGHT)
//   lv_obj_t *apply_right_btn = lv_btn_create(uih); // TODO: fix
//   lv_obj_set_style_radius(apply_right_btn, SETTING_LB_HEIGHT / 2, 0);
//   lv_obj_set_style_text_font(apply_right_btn, &lv_font_montserrat_12, 0);
//   lv_obj_set_size(apply_right_btn, SETTING_LB_WIDTH, SETTING_LB_HEIGHT);
//   lv_obj_align_to(apply_right_btn, uih, LV_ALIGN_RIGHT_MID, 0, 0);
//   lv_obj_t *apply_right_btn_dsc_label = lv_label_create(apply_right_btn);
//   lv_obj_set_style_text_font(apply_right_btn_dsc_label, &lv_font_montserrat_12, 0);
//   lv_label_set_text(apply_right_btn_dsc_label, LV_SYMBOL_CLOSE);
//   lv_obj_center(apply_right_btn_dsc_label);
//   lv_obj_add_event_cb(apply_right_btn, setting_close_event_handler, LV_EVENT_ALL, NULL);
//   g_setting_title_head = uih;
//   g_setting_if_state = 1;
// }
// /*!
//  * @function: setting_create 配置界面
//  */
// static void setting_create(lv_obj_t *parent)
// {
//   setting_interface_title_create(parent); // 创建标题
//   setting_interface_menu_create(parent);  // 创建菜单栏
//   navbar_foreground(true, true);
// }
// static void setting_delete(lv_obj_t *parent)
// {
//   if (g_setting_title_head != NULL)
//   {
//     lv_obj_del(g_setting_title_head);
//     g_setting_title_head = NULL;
//   }
//   setting_interface_menu_delete();
// }

// #if !USE_SDL_SIM
// #include "lsapi_os.h"
// #include "lsapi_sys.h"
// #include "osi_api.h"
// /*!
//  * @function: ui事件处理函数
//  */
// void ui_event_process(const uint32_t event_id, const char *param)
// {
//   if (event_id < 9)
//   {
//     LV_LOG_ERROR("[demo mcxa]: recv event_id is error, please check. event_id:%d\n", event_id);
//     return;
//   }
//   user_data_t *u_data = (user_data_t *)param;
//   t2n_report_t *c_data = (t2n_report_t *)param;
//   config_data_t *cf_data = (config_data_t *)param;
//   char *imei_data = (char *)param;
//   sys_data_t *sd_data = (sys_data_t *)param;
//   switch (event_id)
//   {
//   case BK_EVENT_UI_USER_DATA_ID: // 界面显示应用数据 - 通用
//     // LV_LOG_USER("wait event id 9\n");
//     memcpy(&g_ui_user_data, u_data, sizeof(g_ui_user_data));
//     break;
//   case BK_EVENT_T2N_REPORT_ID: // cure界面应用数据
//     // LV_LOG_USER("wait event id 10\n");
//     memcpy(&g_ui_cure_data, c_data, sizeof(g_ui_cure_data));
//     break;
//   case BK_EVENT_DATA_SYNC_ID: // 后台数据同步事件
//     // LV_LOG_USER("wait event id 11\n");
//     g_ui_cfg_data = cf_data;
//     break;
//   case BK_EVENT_IMEI_DATA_SYNC_ID:
//     // LV_LOG_USER("wait event id 12\n");
//     g_sys_imei_data = imei_data;
//     break;
//   case BK_EVENT_SYS_DATA_SYNC_ID:
//     // LV_LOG_USER("wait event id 13\n");
//     g_sys_data = sd_data;
//     break;
//   default:
//     LV_LOG_USER("******default*******\n");
//     break;
//   }
// }
// #endif
