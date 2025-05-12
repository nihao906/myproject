#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "time.h"

#include  "osi_api.h"
#include "lvgl.h"
#include "../inc/ui/ui.h"
#include "../inc/ui/ui_screen1.h"
#include "../inc/ui/ui_screen2.h"
#include "../inc/ui/ui_screen3.h"
#include "../inc/ui/ui_screen4.h"
#include "../inc/ui/fs_function.h"

#include "ql_fs.h"
#include "ql_log.h"

#define     LOG_INFO(msg, ...)          QL_LOG(QL_LOG_LEVEL_INFO, "ui", msg, ##__VA_ARGS__)

extern const lv_font_t ui_font_16;
extern struct Return_Data global_file_data_p; //存储table数据
extern int current_id;
extern int end_count;
extern uint8_t act_screen_id;
extern lv_timer_t * ui_screen1_data_timer;

#if !TEST
extern user_data_t g_ui_user_data;
#endif
extern MACHINE_WORK_STATE current_wort_state;

void screen1_left_btn_cb(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if((event_code == LV_EVENT_CLICKED) && (ui_Screen4 != NULL)) {
        lv_scr_load_anim(ui_Screen4, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        if(current_wort_state == WORK){
            lv_timer_pause(ui_screen1_data_timer);
        }
        act_screen_id = 4;
    }
}

void screen1_right_btn_cb(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if((event_code == LV_EVENT_CLICKED) && (ui_Screen2 != NULL)) {
        lv_scr_load_anim(ui_Screen2, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        if(current_wort_state == WORK){
            lv_timer_pause(ui_screen1_data_timer);
        }
        act_screen_id = 2;
    }
}

void screen2_left_btn_cb(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if((event_code == LV_EVENT_CLICKED) && (ui_Screen1 != NULL)) {
#if !TEST
        memset(&g_ui_user_data, 0x00, sizeof(g_ui_user_data)); 
#endif
        lv_scr_load_anim(ui_Screen1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        act_screen_id = 1;
        if(current_wort_state == WORK){
            lv_timer_resume(ui_screen1_data_timer);
        }
    }
}

void screen2_right_btn_cb(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if((event_code == LV_EVENT_CLICKED) && (ui_Screen3 != NULL)) {
        lv_scr_load_anim(ui_Screen3, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        act_screen_id = 3;
    }
}

void screen3_left_btn_cb(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if((event_code == LV_EVENT_CLICKED) && (ui_Screen2 != NULL)) {
        lv_scr_load_anim(ui_Screen2, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        act_screen_id = 2;
    }
}

void screen3_right_btn_cb(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if((event_code == LV_EVENT_CLICKED) && (ui_Screen4 != NULL)) {
        lv_scr_load_anim(ui_Screen4, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        act_screen_id = 4;
    }
}

void screen4_left_btn_cb(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if((event_code == LV_EVENT_CLICKED) && (ui_Screen3 != NULL)) {
        lv_scr_load_anim(ui_Screen3, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        act_screen_id = 3;
    }
}

void screen4_right_btn_cb(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if((event_code == LV_EVENT_CLICKED) && (ui_Screen1 != NULL)) {
#if !TEST
        memset(&g_ui_user_data, 0x00, sizeof(g_ui_user_data)); 
#endif
        lv_scr_load_anim(ui_Screen1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        act_screen_id = 1;
        if(current_wort_state == WORK){
            lv_timer_resume(ui_screen1_data_timer);
        }
    }
}

void ui_screens_create(void)
{
    ui_Screen1 = lv_obj_create(NULL);
    ui_Screen2 = lv_obj_create(NULL);
    ui_Screen3 = lv_obj_create(NULL);
    ui_Screen4 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(ui_Screen2,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(ui_Screen3,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(ui_Screen4,LV_OBJ_FLAG_SCROLLABLE);
    lv_disp_load_scr(ui_Screen1);

    lv_obj_t * screen1_left_btn = lv_btn_create(ui_Screen1);
    lv_obj_t * screen1_right_btn = lv_btn_create(ui_Screen1);
    lv_obj_t * screen2_left_btn = lv_btn_create(ui_Screen2);
    lv_obj_t * screen2_right_btn = lv_btn_create(ui_Screen2);
    lv_obj_t * screen3_left_btn = lv_btn_create(ui_Screen3);
    lv_obj_t * screen3_right_btn = lv_btn_create(ui_Screen3);
    lv_obj_t * screen4_left_btn = lv_btn_create(ui_Screen4);
    lv_obj_t * screen4_right_btn = lv_btn_create(ui_Screen4);

    lv_obj_t * btns[8] = {screen1_left_btn,screen1_right_btn,screen2_left_btn,screen2_right_btn,screen3_left_btn,screen3_right_btn,screen4_left_btn,screen4_right_btn};
    lv_obj_t * label = NULL;
    for(uint8_t i = 0; i < 8; i++){

        label = lv_label_create(btns[i]);
        lv_obj_set_style_text_color(label,lv_color_hex(THEME_COLOR_WHITE_DEF),LV_PART_MAIN);
        lv_obj_center(label);

        //设置btn
        lv_obj_set_size(btns[i],30,30);
        lv_obj_set_style_radius(btns[i], 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btns[i],lv_color_hex(THEME_COLOR_BTN_BLUE_DEF),LV_PART_MAIN);
        if(i%2){
            lv_obj_set_pos(btns[i],450,0);
            lv_label_set_text(label,LV_SYMBOL_RIGHT);
        }
        else{
            lv_obj_set_pos(btns[i],0,0);
            lv_label_set_text(label,LV_SYMBOL_LEFT);
        }      
    }

    lv_obj_add_event_cb(screen1_left_btn,screen1_left_btn_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_add_event_cb(screen1_right_btn,screen1_right_btn_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_add_event_cb(screen2_left_btn,screen2_left_btn_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_add_event_cb(screen2_right_btn,screen2_right_btn_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_add_event_cb(screen3_left_btn,screen3_left_btn_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_add_event_cb(screen3_right_btn,screen3_right_btn_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_add_event_cb(screen4_left_btn,screen4_left_btn_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_add_event_cb(screen4_right_btn,screen4_right_btn_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t * screens[4] = {ui_Screen1,ui_Screen2,ui_Screen3,ui_Screen4};
    const char * text[4] = {"作业界面","参数设置","历史数据","水平偏移"};
    for(uint8_t i = 0; i < 4; i++){
        label = lv_label_create(screens[i]);
        lv_obj_set_style_text_font(label, &ui_font_16, 0);
        lv_label_set_text(label,text[i]);
        lv_obj_align(label,LV_ALIGN_TOP_MID,0,10);
    }
}

void ui_init(void)
{
    ui_screens_create();//创建4个活动屏幕，并添加可以切屏的btn
    ui_screen1_init();
    ui_screen2_init();
    ui_screen3_init();
    ui_screen4_init();
    

    // /////////////////////////////////////////////////////设置数据////////////////////////////////////////////////////////////
#if 0
    /*获取当天日期，并显示到日历label上*/
    time_t t = time(NULL);
    struct tm *local_time = localtime(&t);
    uint16_t year = local_time->tm_year + 1900;   // 年份需要加上1900
    uint8_t month = local_time->tm_mon + 1;      // 月份从0开始，需要加1
    uint8_t day = local_time->tm_mday;

    char calendar_text_str[32];
    sprintf(calendar_text_str,"%d.%02d.%02d",year,month,day);
    lv_label_set_text(screen3_calender_dsc_label,calendar_text_str);

    /*将日历设置为对应日期*/
    lv_calendar_set_showed_date(screen3_calender,year,month);
    lv_calendar_set_today_date(screen3_calender,year,month,day);
    
    int8_t result = findDataFile(year,month,day);
    if(result){
        /*存在该目录下的非空文件*/
        char filename[32];
        sprintf(filename,"%c:/%d/%02d/%02d.txt",FILE_LETTER,year,month,day);

        QFILE file_fd = ql_fopen(filename,"rb");

        readDataFromFile(file_fd, &global_file_data_p);

        //重置全局变量“当前id”，“结束计数”, 关闭文件
        ql_fclose(file_fd);
        current_id =0;
        end_count = 0;

        /*根据数据设置控件*/
        //计算总页数，设置spinbox页数label以及spinbox范围
        uint8_t page_count = global_file_data_p.id_count/8 + 1;
        lv_label_set_text_fmt(screen3_spinbox_dsc_label,"/ %d",page_count);
        lv_spinbox_set_range(screen3_spinbox, 1,page_count);
        //LV_LOG_USER("different id num is: %d, page_count = %d\n", global_file_data_p.id_count,page_count);

        //根据当前页数和总页数，设置label的数据
        uint8_t current_page = lv_spinbox_get_value(screen3_spinbox);
        setDataToTables(&global_file_data_p,current_page,page_count);
    }
    else{
        LOG_INFO(" findDataFile result = %d",result);

        //将显示最大页数的label设为NA
        lv_label_set_text_fmt(screen3_spinbox_dsc_label,"N/A");

        //取消spinbox翻页btn和手动设置页数btn的可点击flag
        lv_obj_clear_flag(btn_page_plus,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(btn_page_reduce,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(scrren3_spinbox_btn,LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t * tables[8] = {data_table1,data_table2,data_table3,data_table4,data_table5,data_table6,data_table7,data_table8};
        for(int i = 0; i < 8; i++){
            lv_obj_clear_flag(tables[i],LV_OBJ_FLAG_CLICKABLE);
        }
    }
#endif /*#if 1*/


}