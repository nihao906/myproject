#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/ui/ui.h"
#include "../inc/ui/ui_screen2.h"
#include "ModbusM.h"

#include "ql_log.h"
#define LOG_INFO(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "ui_screen2", msg, ##__VA_ARGS__)
/****************************************************************************************************************************/
lv_obj_t *ui_Screen2;            // 用来切屏
lv_obj_t *screen2_keyboard;      // 用来创建btnm
lv_obj_t *screen2_btnm_textarea; // 用来创建textarea

lv_obj_t *screen2_cal_textareas[4] = {NULL, NULL, NULL, NULL}; // 1通道4ma，1通道20ma，2通道4ma，2通道20ma

lv_obj_t *screen2_flow_input_type_dropdown; // 输入模式
// 最小切除流量1,最小切除流量2,电流1最小AD转换流量,电流1最大AD转换流量,电流2最小AD转换流量,电流2最大AD转换流量,脉冲系数1,脉冲系数2
lv_obj_t *screen2_flow_textareas[8];

// 编码器端口,编码器系数分子,编码器系数分母,最小深度mm,最大深度mm,采样深度mm,默认深度偏移,最小有效深度,允许换桩深度,行走电机开启电流,行走电机关闭电流,持续时间,持续时间,行走电流通道
lv_obj_t *screen2_depth_textareas[14];
lv_obj_t *screen2_depth_input_type_dropdown; // 0：正交 1:正交反向 2:方向脉冲 3:方向脉冲反向
lv_obj_t *screen2_dropdown_list;

cal_4_20ma_t cal_data_struct;
flow_config_t flow_data_struct;
depth_config_t depth_data_struct;
extern lv_obj_t *ui_Screen3;
#if !TEST
extern uint16_t gWordVar[];
#endif

check_t screen2_cal_check_infos[4] = {
    {0, CAL1_AD_4MA_ADDR,  TYPE_UINT16, 0, 65535, 0},
    {1, CAL1_AD_20MA_ADDR, TYPE_UINT16, 0, 65535, 0},
    {2, CAL2_AD_4MA_ADDR,  TYPE_UINT16, 0, 65535, 0},
    {3, CAL2_AD_20MA_ADDR, TYPE_UINT16, 0, 65535, 0},
};

check_t screen2_flow_check_infos[8] = {
    {4,  FLOW_MIN_FLOW1_ADDR,        TYPE_INT16,  0, 32767, -32768},
    {5,  FLOW_MIN_FLOW2_ADDR,        TYPE_INT16,  0, 32767, -32768},
    {6,  FLOW_AD_CAL1_FLOW_MIN_ADDR, TYPE_INT16,  0, 32767, -32768},
    {7,  FLOW_AD_CAL1_FLOW_MAX_ADDR, TYPE_INT16,  0, 32767, -32768},
    {8,  FLOW_AD_CAL2_FLOW_MIN_ADDR, TYPE_INT16,  0, 32767, -32768},
    {9,  FLOW_AD_CAL2_FLOW_MAX_ADDR, TYPE_INT16,  0, 32767, -32768},
    {10, FLOW_PAUSE_COEF1_ADDR,      TYPE_UINT16, 0, 65535, 0},
    {11, FLOW_PAUSE_COEF2_ADDR,      TYPE_UINT16, 0, 65535, 0},
};

check_t screen2_depth_check_infos[14] = {
    {12, DEPTH_PORT_ADDR,         TYPE_UINT8,  0, 1,     0},//深度端口只有两个
    {13, DEPTH_ENC_N_ADDR,        TYPE_UINT16, 0, 65535, 0},
    {14, DEPTH_ENC_M_ADDR,        TYPE_UINT16, 0, 65535, 0},
    {15, DEPTH_MIN_DEPTH_ADDR,    TYPE_INT16,  0, 32767, -32768},
    {16, DEPTH_MAX_DEPTH_ADDR,    TYPE_INT16,  0, 32767, -32768},
    {17, DEPTH_SAMPLE_DEPTH_ADDR, TYPE_INT16,  0, 32767, -32768},
    {18, DEPTH_OFFSET_ADDR,       TYPE_INT16,  0, 32767, -32768},
    {19, DEPTH_MIN_VALID_ADDR,    TYPE_INT16,  0, 32767, -32768},
    {20, DEPTH_INC_PILE_ADDR,     TYPE_INT16,  0, 32767, -32768},
    {21, DEPTH_CUR_ON_ADDR,       TYPE_UINT16, 0, 65535, 0},
    {22, DEPTH_CUR_OFF_ADDR,      TYPE_UINT16, 0, 65535, 0},
    {23, DEPTH_MOVE_ON_DUR_ADDR,  TYPE_UINT16, 0, 65535, 0},
    {24, DEPTH_MOVE_OFF_DUR_ADDR, TYPE_UINT16, 0, 65535, 0},
    {25, DEPTH_MOVE_CUR_CH_ADDR,  TYPE_UINT16, 0, 65535, 0},
};

/****************************************************************************************************************************/
void screen2_cal_SAVE_btn_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        LOG_INFO("screen2_cal_SAVE_btn_event clicked");
#if !TEST
        /*通过modbus发送保存cal结构体的命令*/
        gWordVar[CAL_4_20MA_ADDR] = SAVE_CMD;
        zb_ModBusWordWriteHook(CAL_4_20MA_ADDR,1);
#endif     
    }
}

void screen2_cal_DEFAULT_btn_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        LOG_INFO("screen2_cal_DEFAULT_btn_event clicked");

        lv_textarea_set_text(screen2_cal_textareas[0], "12740");
        lv_textarea_set_text(screen2_cal_textareas[1], "63845");
        lv_textarea_set_text(screen2_cal_textareas[2], "12760");
        lv_textarea_set_text(screen2_cal_textareas[3], "63953");

        /*发送默认配置信息*/
#if !TEST
        gWordVar[CAL_4_20MA_ADDR] = 0;
        gWordVar[CAL1_AD_4MA_ADDR] = 12740;
        gWordVar[CAL1_AD_20MA_ADDR] = 63845;
        gWordVar[CAL2_AD_4MA_ADDR] = 12760;
        gWordVar[CAL2_AD_20MA_ADDR] = 63953;
        zb_ModBusWordWriteHook(CAL_4_20MA_ADDR,5);
#endif
    }
}

/****************************************************************************************************************************/
void screen2_flow_SAVE_btn_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    // lv_obj_t * obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        LOG_INFO("screen2_flow_SAVE_btn_event clicked");
#if !TEST
        /*通过modbus发送保存flow结构体的命令*/
        gWordVar[FLOW_CONFIG_ADDR] = SAVE_CMD;
        zb_ModBusWordWriteHook(FLOW_CONFIG_ADDR,1);
#endif 
    }
}

void screen2_flow_DEFAULT_btn_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        LOG_INFO("screen2_flow_DEFAULT_btn_event clicked");
        char default_value[][6] = {"0", "0", "0", "10000", "0", "10000", "6944", "6944"};
        for (int i = 0; i < 8; i++){
            lv_textarea_set_text(screen2_flow_textareas[i], default_value[i]);
        }
        lv_dropdown_set_selected(screen2_flow_input_type_dropdown, 0);
#if !TEST
        gWordVar[FLOW_CONFIG_ADDR] = 0;
        gWordVar[FLOW_INPUT_TYPE_ADDR] = FLOW_INPUT_TYPE_MODE1;
        gWordVar[FLOW_MIN_FLOW1_ADDR] = 0;
        gWordVar[FLOW_MIN_FLOW2_ADDR] = 0;
        gWordVar[FLOW_AD_CAL1_FLOW_MIN_ADDR] = 0;
        gWordVar[FLOW_AD_CAL1_FLOW_MAX_ADDR] = 10000;
        gWordVar[FLOW_AD_CAL2_FLOW_MIN_ADDR] = 0;
        gWordVar[FLOW_AD_CAL2_FLOW_MAX_ADDR] = 10000;
        gWordVar[FLOW_PAUSE_COEF1_ADDR] = 6944;
        gWordVar[FLOW_PAUSE_COEF2_ADDR] = 6944;
        zb_ModBusWordWriteHook(FLOW_CONFIG_ADDR,10);
#endif
    }
}

/****************************************************************************************************************************/
void screen2_depth_SAVE_btn_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        LOG_INFO("screen2_depth_SAVE_btn_event clicked");
#if !TEST
        /*通过modbus发送保存depth结构体的命令*/
        gWordVar[DEPTH_CONFIG_ADDR] = SAVE_CMD;
        zb_ModBusWordWriteHook(DEPTH_CONFIG_ADDR,1);
#endif 
    }
}

void screen2_depth_DEFAULT_btn_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        LOG_INFO("screen2_depth_DEFAULT_btn_event clicked");
        
        /*将当前栏目的所有文本框都设为默认值*/
        char depth_default_data[][6] = {"1", "1000", "640", "-100", "12000", "100", "-100", "1000", "-5000", "500", "100", "150", "150", "2"};
        for (int i = 0; i < 14; i++)
        {
            lv_textarea_set_text(screen2_depth_textareas[i], depth_default_data[i]);
        }
        lv_dropdown_set_selected(screen2_depth_input_type_dropdown, 0);

        /*发送默认配置信息*/
#if !TEST
        gWordVar[DEPTH_CONFIG_ADDR] = 0;
        gWordVar[DEPTH_INPUT_TYPE_ADDR] = ((gWordVar[DEPTH_INPUT_TYPE_ADDR] & 0x00ff) | DEPTH_INPUT_TYPE_MODE1);
        gWordVar[DEPTH_PORT_ADDR] = ((gWordVar[DEPTH_INPUT_TYPE_ADDR] & 0xff00) | 1);
        gWordVar[DEPTH_ENC_N_ADDR] = 1000;
        gWordVar[DEPTH_ENC_M_ADDR] = 640;
        gWordVar[DEPTH_MIN_DEPTH_ADDR] = -100;
        gWordVar[DEPTH_MAX_DEPTH_ADDR] = 12000;
        gWordVar[DEPTH_SAMPLE_DEPTH_ADDR] = 100;
        gWordVar[DEPTH_OFFSET_ADDR] = -100;
        gWordVar[DEPTH_MIN_VALID_ADDR] = 1000;
        gWordVar[DEPTH_INC_PILE_ADDR] = -5000;
        gWordVar[DEPTH_CUR_ON_ADDR] = 500;
        gWordVar[DEPTH_CUR_OFF_ADDR] = 100;
        gWordVar[DEPTH_MOVE_ON_DUR_ADDR] = 150;
        gWordVar[DEPTH_MOVE_OFF_DUR_ADDR] = 150;
        gWordVar[DEPTH_MOVE_CUR_CH_ADDR] = 2;
        zb_ModBusWordWriteHook(DEPTH_CONFIG_ADDR,15);
#endif
    }
}

/****************************************************************************************************************************/
/*选择流量输入模式，选择后modbus发送对应值*/
void screen2_flow_input_type_dropdown_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED){
        uint16_t id = lv_dropdown_get_selected(screen2_flow_input_type_dropdown);
        LOG_INFO("id = %d",id);
#if !TEST
        //4-20ma
        if(id == 0){
            gWordVar[FLOW_INPUT_TYPE_ADDR] = FLOW_INPUT_TYPE_MODE1;
        }
        //0-3.6k
        else if(id == 1){
            gWordVar[FLOW_INPUT_TYPE_ADDR] = FLOW_INPUT_TYPE_MODE2;
        }
        zb_ModBusWordWriteHook(FLOW_INPUT_TYPE_ADDR,1);
#endif
    }
}

void list_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_DELETE)
    {
        LV_LOG_USER("screen2_dropdown_list is deleted\n");
        lv_obj_add_flag(screen2_depth_input_type_dropdown, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(obj, list_event);
    }
}

void screen2_depth_input_type_dropdown_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        lv_obj_clear_flag(screen2_depth_input_type_dropdown, LV_OBJ_FLAG_CLICKABLE);
        screen2_dropdown_list = lv_dropdown_get_list(obj);
        lv_obj_clear_flag(screen2_dropdown_list,LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_text_font(screen2_dropdown_list, &ui_font_16, 0);
        lv_obj_add_event_cb(screen2_dropdown_list, list_event, LV_EVENT_ALL, NULL);
    }
}

void screen2_depth_input_type_dropdown_event2(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t id = lv_dropdown_get_selected(screen2_depth_input_type_dropdown);
        LOG_INFO("id = %d",id);
#if !TEST
        if(id == 0){//正交
            gWordVar[DEPTH_INPUT_TYPE_ADDR] = ((gWordVar[DEPTH_INPUT_TYPE_ADDR] & 0x00ff) | DEPTH_INPUT_TYPE_MODE1);
        }
        else if(id == 1){//正交反向
            gWordVar[DEPTH_INPUT_TYPE_ADDR] = ((gWordVar[DEPTH_INPUT_TYPE_ADDR] & 0x00ff) | DEPTH_INPUT_TYPE_MODE2);
        }
        else if(id == 2){//方向脉冲
            gWordVar[DEPTH_INPUT_TYPE_ADDR] = ((gWordVar[DEPTH_INPUT_TYPE_ADDR] & 0x00ff) | DEPTH_INPUT_TYPE_MODE3);
        }
        else if(id == 3){//方向脉冲反向
            gWordVar[DEPTH_INPUT_TYPE_ADDR] = ((gWordVar[DEPTH_INPUT_TYPE_ADDR] & 0x00ff) | DEPTH_INPUT_TYPE_MODE4);
        }
        zb_ModBusWordWriteHook(DEPTH_INPUT_TYPE_ADDR,1);
#endif
    }
}

/****************************************************************************************************************************/
/*当点击回车时，获取显示文本框的值，*/
void screen2_btnm_input_data(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *keyboard = lv_event_get_target(e);                // 获取键盘指针
    check_t *check_info = (check_t *)lv_event_get_user_data(e); // 获取userdata
    if (check_info != NULL)
    {
        //LOG_INFO("id = %d, addr = %d, type = %d, decimal_places = %d, max = %d, min = %d", check_info->id, check_info->addr, check_info->type, check_info->decimal_places, check_info->max_num, check_info->min_num);
    }
    else
    {
        LOG_INFO("check_info = NULL");
        return;
    }

    if (code == LV_EVENT_VALUE_CHANGED)
    {

        /*将键盘的值实时显示在textarea上*/
        int id = lv_btnmatrix_get_selected_btn(keyboard);          // 获取点击的按钮的序号
        const char *txt = lv_btnmatrix_get_btn_text(keyboard, id); // 获取点击的按钮的文本内容

        // 当点击回车时
        if (strcmp(txt, LV_SYMBOL_OK) == 0)
        {

            /*获取此时显示文本框的字符串值并转为整型*/
            const char *str = lv_textarea_get_text(screen2_btnm_textarea);
            //LOG_INFO("str = %s", str);

            int num = 0;
            if (check_info->type != TYPE_FLOAT)
            {
                num = atoi(str);
            }

            /*判断该值是否在定义域内*/
            num = (num > check_info->max_num) ? check_info->max_num : ((num < check_info->min_num) ? check_info->min_num : num);
            char correct_str[20] = {""};
            itoa(num, correct_str, 10);
            //LOG_INFO("correct_str = %s", correct_str);

            /*将合适的值赋值给触发事件的文本框上*/
            if ((check_info->id <= 3) && (check_info->id >= 0))
            {
                lv_textarea_set_text(screen2_cal_textareas[check_info->id], correct_str);
            }
            else if ((check_info->id >= 4) && (check_info->id <= 11))
            {
                lv_textarea_set_text(screen2_flow_textareas[check_info->id - 4], correct_str);
            }
            else if((check_info->id >= 12) && (check_info->id <= 25)){
                lv_textarea_set_text(screen2_depth_textareas[check_info->id - 12], correct_str);
            }

            /*删除btnm和文本框的控件，并将指针置NULL*/
            lv_obj_del(screen2_btnm_textarea);
            lv_obj_del(screen2_keyboard);
            screen2_btnm_textarea = NULL;
            screen2_keyboard = NULL;

            /*将num赋值给gWrodVar里对应的寄存器地址，并传输*/
#if !TEST
            if(check_info->id == 12){
                gWordVar[check_info->addr] = ((gWordVar[check_info->addr] & 0xff00) | (uint8_t)num);
            }
            else{
                if(check_info->type == TYPE_INT16){
                    gWordVar[check_info->addr] = (int16_t)num;
                }
                else if(check_info->type == TYPE_UINT16){
                    gWordVar[check_info->addr] = (uint16_t)num;
                }
            }
            zb_ModBusWordWriteHook(check_info->addr, 1);
#endif
        }
    }
}

/*当聚焦textarea时，创建btnm和显示文本框，关联两者，并添加事件函数*/
void screen2_btnm_event(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e); // 获取触发事件的文本框的指针
    check_t *check_info = lv_event_get_user_data(e);
    if (check_info != NULL)
    {
        //LOG_INFO("id = %d, addr = %d, type = %d, decimal_places = %d, max = %d, min = %d", check_info->id, check_info->addr, check_info->type, check_info->decimal_places, check_info->max_num, check_info->min_num);
    }
    else
    {
        LOG_INFO("check_info = NULL");
        return;
    }

    // 当聚焦textarea时
    if (event_code == LV_EVENT_FOCUSED)
    {

        // 创建键盘和textarea
        screen2_keyboard = lv_keyboard_create(ui_Screen2);
        lv_obj_clear_flag(screen2_keyboard, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_set_size(screen2_keyboard, 480, 220);
        lv_obj_align(screen2_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_keyboard_set_mode(screen2_keyboard, LV_KEYBOARD_MODE_NUMBER);

        screen2_btnm_textarea = lv_textarea_create(ui_Screen2);
        lv_obj_set_size(screen2_btnm_textarea, 480, 70);
        lv_obj_align_to(screen2_btnm_textarea, screen2_keyboard, LV_ALIGN_OUT_TOP_MID, 0, 0);
        lv_obj_set_style_text_font(screen2_btnm_textarea, &lv_font_montserrat_42, 0);
        lv_textarea_set_max_length(screen2_btnm_textarea, 5);

        // 关联键盘和文本框
        lv_keyboard_set_textarea(screen2_keyboard, screen2_btnm_textarea);

        // 添加事件函数
        lv_obj_add_event_cb(screen2_keyboard, screen2_btnm_input_data, LV_EVENT_VALUE_CHANGED, (void *)check_info); // 把触发事件的控件指针作为userdata传给输入回调函数

        // 获取触发事件的文本框的值显示到显示文本框上
        const char *textarea_value = lv_textarea_get_text(obj);
        lv_textarea_set_text(screen2_btnm_textarea, textarea_value);

        // 给文本框添加聚焦状态，清除触发obj的聚焦状态
        lv_obj_add_state(screen2_btnm_textarea, LV_STATE_FOCUSED);
        lv_obj_clear_state(obj, LV_STATE_FOCUSED);
    }
}

/****************************************************************************************************************************/

void ui_screen2_init(void)
{
    lv_obj_t *screen2_tabview = lv_tabview_create(ui_Screen2, LV_DIR_TOP, 50);
    lv_obj_align(screen2_tabview, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_bg_color(screen2_tabview, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen2_tabview, 20, LV_PART_MAIN);
    lv_obj_set_style_text_font(screen2_tabview, &ui_font_16, 0);

    uint8_t btns_color[2] = {LV_PALETTE_GREEN, LV_PALETTE_BLUE};
    char btns_text[2][10] = {"保存", "默认"};

    lv_obj_t *screen2_cal_tab = lv_tabview_add_tab(screen2_tabview, "电流");
    lv_obj_t *screen2_flow_tab = lv_tabview_add_tab(screen2_tabview, "流量");
    lv_obj_t *screen2_depth_tab = lv_tabview_add_tab(screen2_tabview, "深度");

    // 在screen2_cal_tab上创建
    uint8_t cal_align[4][2] = {{0, 0}, {0, 45}, {0, 90}, {0, 135}};
    char cal_text[4][30] = {"1通道AD转换4ma", "1通道AD转换20ma", "2通道AD转换4ma", "2通道AD转换20ma"};
    char cal_default_value[4][10] = {"12740", "63845", "12760", "63953"};
    uint8_t textarea_size[4][2] = {{100, 40}, {100, 40}, {100, 40}, {100, 40}};

    for (uint8_t i = 0; i < 4; i++)
    {
        lv_obj_t *label = lv_label_create(screen2_cal_tab);
        lv_obj_set_size(label, 200, 40);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, cal_align[i][0], cal_align[i][1]);
        lv_obj_set_style_text_font(label, &ui_font_16, LV_PART_MAIN);
        lv_label_set_text(label, cal_text[i]);

        screen2_cal_textareas[i] = lv_textarea_create(screen2_cal_tab);
        lv_obj_set_style_text_font(screen2_cal_textareas[i], &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_set_size(screen2_cal_textareas[i], textarea_size[i][0], textarea_size[i][1]);
        lv_obj_align_to(screen2_cal_textareas[i], label, LV_ALIGN_OUT_RIGHT_MID, 0, -8);
        lv_obj_clear_flag(screen2_cal_textareas[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_textarea_set_text(screen2_cal_textareas[i], cal_default_value[i]);
        lv_obj_add_event_cb(screen2_cal_textareas[i], screen2_btnm_event, LV_EVENT_FOCUSED, &screen2_cal_check_infos[i]);
    }

    lv_obj_t *cal_btns[2] = {NULL, NULL};
    int16_t cal_btns_pos[2][2] = {{290, 170}, {50, 170}};

    for (uint8_t i = 0; i < 2; i++)
    {
        cal_btns[i] = lv_btn_create(screen2_cal_tab);
        lv_obj_set_size(cal_btns[i], 140, 50);
        lv_obj_set_pos(cal_btns[i], cal_btns_pos[i][0], cal_btns_pos[i][1]);
        lv_obj_set_style_bg_color(cal_btns[i], lv_palette_main(btns_color[i]), 0);
        lv_obj_set_style_opa(cal_btns[i], 150, 0);

        lv_obj_t *label = lv_label_create(cal_btns[i]);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_font(label, &ui_font_16, 0);
        lv_label_set_text(label, btns_text[i]);
    }
    lv_obj_add_event_cb(cal_btns[0], screen2_cal_SAVE_btn_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(cal_btns[1], screen2_cal_DEFAULT_btn_event, LV_EVENT_CLICKED, NULL);

    // 在screen2_flow_tab创建的
    lv_obj_t *flow_dd_label = lv_label_create(screen2_flow_tab);
    lv_obj_set_size(flow_dd_label, 120, 40);
    lv_obj_align(flow_dd_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(flow_dd_label, &ui_font_16, 0);
    lv_label_set_text(flow_dd_label, "输入模式");

    screen2_flow_input_type_dropdown = lv_dropdown_create(screen2_flow_tab);
    lv_obj_set_size(screen2_flow_input_type_dropdown, 130, 40);
    lv_obj_align_to(screen2_flow_input_type_dropdown, flow_dd_label, LV_ALIGN_OUT_RIGHT_MID, 0, -10);
    lv_obj_set_style_text_font(screen2_flow_input_type_dropdown, &lv_font_montserrat_16, 0);
    lv_dropdown_set_options(screen2_flow_input_type_dropdown, "4 ~ 20ma\n0 ~ 3.6k\n");
    lv_obj_add_event_cb(screen2_flow_input_type_dropdown,screen2_flow_input_type_dropdown_cb,LV_EVENT_VALUE_CHANGED, NULL);

    uint16_t flow_label_align[8][2] = {{0, 45}, {0, 90}, {0, 135}, {0, 180}, {0, 225}, {0, 270}, {0, 315}, {0, 360}};
    char flow_label_text[8][35] = {"最小切除流量1", "最小切除流量2", "电流1最小AD转换流量", "电流1最大AD转换流量", "电流2最小AD转换流量", "电流2最大AD转换流量", "脉冲系数1", "脉冲系数2"};
    uint8_t flow_textarea_size[8][2] = {{80, 40}, {80, 40}, {100, 40}, {100, 40}, {100, 40}, {100, 40}, {100, 40}, {100, 40}};
    char flow_textarea_default_data[8][10] = {"0", "0", "0", "10000", "0", "10000", "6944", "6944"};
    for (uint8_t i = 0; i < 8; i++)
    {
        lv_obj_t *label = lv_label_create(screen2_flow_tab);
        lv_obj_set_size(label, 220, 40);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, flow_label_align[i][0], flow_label_align[i][1]);
        lv_obj_set_style_text_font(label, &ui_font_16, LV_PART_MAIN);
        lv_label_set_text(label, flow_label_text[i]);

        screen2_flow_textareas[i] = lv_textarea_create(screen2_flow_tab);
        lv_obj_set_style_text_font(screen2_flow_textareas[i], &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_set_size(screen2_flow_textareas[i], flow_textarea_size[i][0], flow_textarea_size[i][1]);
        lv_obj_align_to(screen2_flow_textareas[i], label, LV_ALIGN_OUT_RIGHT_MID, 0, -8);
        lv_obj_clear_flag(screen2_flow_textareas[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_textarea_set_text(screen2_flow_textareas[i], flow_textarea_default_data[i]);
        lv_obj_add_event_cb(screen2_flow_textareas[i], screen2_btnm_event, LV_EVENT_FOCUSED, &screen2_flow_check_infos[i]);
    }
    lv_obj_t *label = lv_label_create(screen2_flow_tab);
    lv_label_set_text(label, "");
    lv_obj_set_pos(label, 0, 465);

    lv_obj_t *flow_btns[2] = {NULL, NULL};
    int16_t flow_btns_pos[2][2] = {{290, 405}, {50, 405}};
    for (uint8_t i = 0; i < 2; i++)
    {
        flow_btns[i] = lv_btn_create(screen2_flow_tab);
        lv_obj_set_size(flow_btns[i], 140, 50);
        lv_obj_set_pos(flow_btns[i], flow_btns_pos[i][0], flow_btns_pos[i][1]);
        lv_obj_set_style_bg_color(flow_btns[i], lv_palette_main(btns_color[i]), 0);
        lv_obj_set_style_opa(flow_btns[i], 150, 0);

        lv_obj_t *label = lv_label_create(flow_btns[i]);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_font(label, &ui_font_16, 0);
        lv_label_set_text(label, btns_text[i]);
    }
    lv_obj_add_event_cb(flow_btns[0], screen2_flow_SAVE_btn_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(flow_btns[1], screen2_flow_DEFAULT_btn_event, LV_EVENT_CLICKED, NULL);

    // 在screen2_depth_tab创建的
    lv_obj_t *depth_dd_label = lv_label_create(screen2_depth_tab);
    lv_obj_set_size(depth_dd_label, 120, 40);
    lv_obj_align(depth_dd_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(depth_dd_label, &ui_font_16, 0);
    lv_label_set_text(depth_dd_label, "输入模式");

    screen2_depth_input_type_dropdown = lv_dropdown_create(screen2_depth_tab);
    lv_obj_set_size(screen2_depth_input_type_dropdown, 160, 40);
    lv_obj_align_to(screen2_depth_input_type_dropdown, depth_dd_label, LV_ALIGN_OUT_RIGHT_MID, 0, -10);
    lv_obj_set_style_text_font(screen2_depth_input_type_dropdown, &ui_font_16, 0);
    lv_dropdown_set_options(screen2_depth_input_type_dropdown, "正交\n正交反向\n方向脉冲\n方向脉冲反向\n");
    lv_obj_add_event_cb(screen2_depth_input_type_dropdown, screen2_depth_input_type_dropdown_event, LV_EVENT_ALL, NULL);//让下拉列表可以显示汉字
    lv_obj_add_event_cb(screen2_depth_input_type_dropdown, screen2_depth_input_type_dropdown_event2, LV_EVENT_VALUE_CHANGED, NULL);//选择后进行参数设置

    uint16_t depth_label_align[14][2] = {{0, 45}, {0, 90}, {0, 135}, {0, 180}, {0, 225}, {0, 270}, {0, 315}, {0, 360}, {0, 405}, {0, 450}, {0, 495}, {0, 540}, {0, 595}, {0, 640}};
    char depth_label_text[14][30] = {"编码器端口", "编码器系数分子", "编码器系数分母", "最小深度mm", "最大深度mm", "采样深度mm", "默认深度偏移", "最小有效深度", "允许换桩深度", "行走电机开启电流", "行走电机关闭电流", "moveon持续时间", "moveoff持续时间", "行走电流通道"};
    char depth_default_data[14][10] = {"1", "1000", "640", "-100", "12000", "100", "-100", "1000", "-5000", "500", "100", "150", "150", "2"};
    for (uint8_t i = 0; i < 14; i++)
    {
        lv_obj_t *label = lv_label_create(screen2_depth_tab);
        lv_obj_set_size(label, 200, 40);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, depth_label_align[i][0], depth_label_align[i][1]);
        lv_obj_set_style_text_font(label, &ui_font_16, LV_PART_MAIN);
        lv_label_set_text(label, depth_label_text[i]);

        screen2_depth_textareas[i] = lv_textarea_create(screen2_depth_tab);
        lv_obj_set_style_text_font(screen2_depth_textareas[i], &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_set_size(screen2_depth_textareas[i], 100, 40);
        lv_obj_align_to(screen2_depth_textareas[i], label, LV_ALIGN_OUT_RIGHT_MID, 0, -8);
        lv_obj_clear_flag(screen2_depth_textareas[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_textarea_set_text(screen2_depth_textareas[i], depth_default_data[i]);
        lv_obj_add_event_cb(screen2_depth_textareas[i], screen2_btnm_event, LV_EVENT_FOCUSED, &screen2_depth_check_infos[i]);
    }
    label = lv_label_create(screen2_depth_tab);
    lv_label_set_text(label, "");
    lv_obj_set_pos(label, 0, 745);

    lv_obj_t *depth_btns[2] = {NULL, NULL};
    int16_t depth_btns_pos[2][2] = {{290, 685}, {50, 685}};
    for (uint8_t i = 0; i < 2; i++)
    {
        depth_btns[i] = lv_btn_create(screen2_depth_tab);
        lv_obj_set_size(depth_btns[i], 140, 50);
        lv_obj_set_pos(depth_btns[i], depth_btns_pos[i][0], depth_btns_pos[i][1]);
        lv_obj_set_style_bg_color(depth_btns[i], lv_palette_main(btns_color[i]), 0);
        lv_obj_set_style_opa(depth_btns[i], 150, 0);

        lv_obj_t *label = lv_label_create(depth_btns[i]);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_font(label, &ui_font_16, 0);
        lv_label_set_text(label, btns_text[i]);
    }
    lv_obj_add_event_cb(depth_btns[0], screen2_depth_SAVE_btn_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(depth_btns[1], screen2_depth_DEFAULT_btn_event, LV_EVENT_CLICKED, NULL);
}