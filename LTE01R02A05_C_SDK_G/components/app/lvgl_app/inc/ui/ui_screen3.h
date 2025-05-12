#include "lvgl.h"

#define THEME_COLOR_WHITE_DEF 0xFFFFFF
#define THEME_COLOR_BTN_BLUE_DEF 0x2196F3

extern const lv_font_t lv_font_chinese_12_name;
extern const lv_font_t lv_font_montserrat_24;

extern lv_obj_t * ui_Screen3;//用来切屏
extern lv_obj_t * screen3_calender_dsc_label;
extern lv_obj_t * screen3_calender;
extern lv_obj_t * screen3_spinbox_dsc_label;
extern lv_obj_t * btn_page_plus;
extern lv_obj_t * btn_page_reduce;
extern lv_obj_t * screen3_spinbox;
extern lv_obj_t * scrren3_spinbox_btn;

extern lv_obj_t * data_table1;
extern lv_obj_t * data_table2;
extern lv_obj_t * data_table3;
extern lv_obj_t * data_table4;
extern lv_obj_t * data_table5;
extern lv_obj_t * data_table6;
extern lv_obj_t * data_table7;
extern lv_obj_t * data_table8;
void ui_screen3_init(void);