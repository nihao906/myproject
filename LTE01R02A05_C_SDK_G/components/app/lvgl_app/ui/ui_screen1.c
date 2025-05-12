#include "../inc/ui/ui_screen1.h"
#include "stdio.h"
#include "stdlib.h"
#include "osi_api.h"
#include "ModbusM.h"
#include "ql_log.h"
#include "time.h"
#include "../inc/ui/ui.h"

#define     LOG_INFO(msg, ...)          QL_LOG(QL_LOG_LEVEL_INFO, "ui_screen1", msg, ##__VA_ARGS__)

#define DEPTH_REG_ADDR (12)
#define gWORD_SIZE (8192)

lv_obj_t * ui_Screen1;//用于切屏
lv_obj_t * current_bar; //显示电流的bar
lv_obj_t * current_value;
lv_obj_t * SIM_card_label; //sim卡状态
lv_obj_t * gnet_dsc_label; //4G描述label
lv_obj_t * g_gnet_label; //4G状态label
lv_obj_t * gps_level_label;//gps状态
lv_obj_t * net_connect_label;//网络连接状态
lv_obj_t * server_connect_label;//服务器连接状态
lv_obj_t * screen1_led;//用于更改led颜色
lv_obj_t * screen1_play_btn_label;//用于更改按钮的图标

lv_obj_t * screen1_measurements_label1;//速度
lv_obj_t * screen1_measurements_label2;//时间（未知）
lv_obj_t * screen1_measurements_label3;//深度
lv_obj_t * screen1_measurements_label4;//瞬时1
lv_obj_t * screen1_measurements_label5;//瞬时2
lv_obj_t * screen1_measurements_label6;//10cm1（未知）
lv_obj_t * screen1_measurements_label7;//10cm2（未知）
lv_obj_t * screen1_measurements_label8;//累计1
lv_obj_t * screen1_measurements_label9;//累计2

extern lv_obj_t * ui_Screen2;
extern lv_obj_t * ui_Screen4;
#if !TEST
extern uint16_t gWordVar[];
user_data_t g_ui_user_data;
#endif
MACHINE_WORK_STATE current_wort_state = STOP;
/*********************************************************************************************/

lv_timer_t * ui_screen1_data_timer;
lv_timer_t * ui_screen1_led_timer;

int8_t led_state = 1;//1:on  -1:off
static void ui_screen1_led_timer_cb(lv_timer_t *e)
{
    if(current_wort_state == WORK){
        lv_led_toggle(screen1_led);
        led_state = led_state * (-1);
    }
    else{
        if(led_state == -1){
            lv_led_on(screen1_led);
        }
    }
}

uint8_t test_data = 0;
static void ui_screen1_data_timer_cb(lv_timer_t *e)
{
    
#if TEST
    test_data ++;
    lv_label_set_text_fmt(screen1_measurements_label1, "%d", test_data);
    lv_label_set_text_fmt(screen1_measurements_label2, "%d", test_data);
    lv_label_set_text_fmt(screen1_measurements_label3, "%d", test_data);
    lv_label_set_text_fmt(screen1_measurements_label4, "%d", test_data);
    lv_label_set_text_fmt(screen1_measurements_label5, "%d", test_data);
    lv_label_set_text_fmt(screen1_measurements_label6, "%d", test_data);
    lv_label_set_text_fmt(screen1_measurements_label7, "%d", test_data);
    lv_label_set_text_fmt(screen1_measurements_label8, "%d", test_data);
    lv_label_set_text_fmt(screen1_measurements_label9, "%d", test_data);
#else
    char buf[12] = {0};

    memset(buf, 0, 12);
    sprintf(buf, "%.1f", g_ui_user_data.speed / 10.0);
    lv_label_set_text_fmt(screen1_measurements_label1, "%s", buf); 

    /*时间*/
    memset(buf, 0, 12);
    sprintf(buf, "%d", g_ui_user_data.one_pile_work_time);
    lv_label_set_text_fmt(screen1_measurements_label2, "%s", buf); 

    memset(buf, 0, 12);
    sprintf(buf, "%.3f", g_ui_user_data.depth / 1000.0);
    lv_label_set_text_fmt(screen1_measurements_label3, "%s", buf);  // mm转换成cm

    memset(buf, 0, 12);
    sprintf(buf, "%.2f", g_ui_user_data.ss_1 / 100.0);  // 1通道瞬时流量
    lv_label_set_text_fmt(screen1_measurements_label4, "%s", buf);

    memset(buf, 0, 12);
    sprintf(buf, "%.2f", g_ui_user_data.ss_2 / 100.0);  // 2通道瞬时流量
    lv_label_set_text_fmt(screen1_measurements_label5, "%s", buf);

    /*10cm流量1&2*/
    memset(buf, 0, 12);
    sprintf(buf, "%.2f", g_ui_user_data.accumulate1 / 100.0);  // 10cm累计流量1
    lv_label_set_text_fmt(screen1_measurements_label6, "%s", buf);

    memset(buf, 0, 12);
    sprintf(buf, "%.2f", g_ui_user_data.accumulate2 / 100.0);  
    lv_label_set_text_fmt(screen1_measurements_label7, "%s", buf);

    memset(buf, 0, 12);
    sprintf(buf, "%.1f", g_ui_user_data.ll_1 / 100.0); //1通道累计流量
    lv_label_set_text_fmt(screen1_measurements_label8, "%s", buf);

    memset(buf, 0, 12);
    sprintf(buf, "%.1f", g_ui_user_data.ll_2 / 100.0);
    lv_label_set_text_fmt(screen1_measurements_label9, "%s", buf);

    lv_bar_set_value(current_bar,g_ui_user_data.Ia,LV_ANIM_OFF);

    memset(buf, 0, 12);
    sprintf(buf, "%.2f", g_ui_user_data.Ia / 10.0);
    lv_label_set_text_fmt(current_value,"%s",buf);
#endif
}

/*********************************************************************************************/
bool start_pause_flag = 0;
void play_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        LOG_INFO("play btn clicked");
        /*stop -> work*/
        if((current_wort_state == STOP) && (start_pause_flag == 0)){

            LOG_INFO("(current_wort_state == STOP) && (start_pause_flag == 0)");

            /*将数据全部置0*/
            lv_obj_t * obj[9] = {
                screen1_measurements_label1,
                screen1_measurements_label2,
                screen1_measurements_label3,
                screen1_measurements_label4,
                screen1_measurements_label5,
                screen1_measurements_label6,
                screen1_measurements_label7,
                screen1_measurements_label8,
                screen1_measurements_label9,
            };
            for(uint8_t i = 0; i < 9; i++){
                lv_label_set_text(obj[i],"0");
            }
#if !TEST
            /*发送启动命令*/
            gWordVar[DEPTH_REG_ADDR] = ((gWordVar[DEPTH_REG_ADDR] & 0x00ff) | WORK);
            LOG_INFO("zb_ModBusWordWriteHook(DEPTH_REG_ADDR,1) res = %d",zb_ModBusWordWriteHook(DEPTH_REG_ADDR,1));
#else
            test_data = 0;
#endif
            /*启动定时器，开始显示监测数据*/
            lv_timer_resume(ui_screen1_data_timer);

            /*更新当前工作状态*/
            current_wort_state = WORK;

            /*开始按键变成暂停按键，并更新状态*/
            start_pause_flag = 1;
            lv_led_set_color(screen1_led,lv_color_hex(THEME_COLOR_SPRING_GREEN_DEF));
            lv_label_set_text(screen1_play_btn_label,LV_SYMBOL_PAUSE);
        }
        /*stop -> pause*/
        else if((current_wort_state == STOP) && (start_pause_flag == 1)){
            /*不做反应*/
        } 
        /*pause -> work*/
        else if((current_wort_state == PAUSE) && (start_pause_flag == 0)){
            LOG_INFO("(current_wort_state == PAUSE) && (start_pause_flag == 0)");
#if !TEST
            /*发送启动命令*/
            gWordVar[DEPTH_REG_ADDR] = ((gWordVar[DEPTH_REG_ADDR] & 0x00ff) | WORK);
            LOG_INFO("zb_ModBusWordWriteHook(DEPTH_REG_ADDR,1) res = %d",zb_ModBusWordWriteHook(DEPTH_REG_ADDR,1));
#endif
            /*启动定时器，继续监测数据*/
            lv_timer_resume(ui_screen1_data_timer);

            /*更新当前工作状态*/
            current_wort_state = WORK;

            /*开始按键变成暂停按键，并更新状态标志*/
            lv_led_set_color(screen1_led,lv_color_hex(THEME_COLOR_SPRING_GREEN_DEF));
            lv_label_set_text(screen1_play_btn_label,LV_SYMBOL_PAUSE);

            start_pause_flag = 1;
        } 
        /*pause -> pause*/
        else if((current_wort_state == PAUSE) && (start_pause_flag == 1)){
            /*不做反应*/
        }
        /*work -> pause*/
        else if((current_wort_state == WORK) && (start_pause_flag == 1)){
            LOG_INFO("(current_wort_state == WORK) && (start_pause_flag == 1)");
#if !TEST
            /*发送暂停命令*/
            gWordVar[DEPTH_REG_ADDR] = ((gWordVar[DEPTH_REG_ADDR] & 0x00ff) | PAUSE);
            LOG_INFO("zb_ModBusWordWriteHook(DEPTH_REG_ADDR,1) res = %d",zb_ModBusWordWriteHook(DEPTH_REG_ADDR,1));
#endif
            /*暂停定时器*/
            lv_timer_pause(ui_screen1_data_timer);

            /*更新当前工作状态*/
            current_wort_state = PAUSE;

            /*暂停按键变成开始按键，并更新状态标志*/
            lv_led_set_color(screen1_led,lv_palette_main(LV_PALETTE_YELLOW));
            lv_label_set_text(screen1_play_btn_label,LV_SYMBOL_PLAY);

            start_pause_flag = 0;
        }
        /*work -> work*/
        else if((current_wort_state == WORK) && (start_pause_flag == 0)){
            /*不做反应*/
        }
    }
}

void stop_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        LOG_INFO("stop btn clicked");

        /*stop -> stop*/
        if(current_wort_state == STOP){
            /*不做处理*/
        }
        else{
            LOG_INFO("current_wort_state != STOP");
            lv_timer_pause(ui_screen1_data_timer);

            lv_led_set_color(screen1_led,lv_palette_main(LV_PALETTE_RED));
            lv_label_set_text(screen1_play_btn_label,LV_SYMBOL_PLAY);
            start_pause_flag = 0;
#if !TEST
            /*发送终止信号*/
            gWordVar[DEPTH_REG_ADDR] = ((gWordVar[DEPTH_REG_ADDR] & 0x00ff) | STOP);
            LOG_INFO("zb_ModBusWordWriteHook(DEPTH_REG_ADDR,1) res = %d",zb_ModBusWordWriteHook(DEPTH_REG_ADDR,1));

            /*清空全局变量*/
            memset(&g_ui_user_data, 0x00, sizeof(g_ui_user_data)); 
#endif
            current_wort_state = STOP;
        }
    }
}

/*********************************************************************************************/
void ui_screen1_init(void)
{
#if !TEST
    memset(&g_ui_user_data, 0x00, sizeof(g_ui_user_data)); 
#endif
    //sim卡标识
    SIM_card_label = lv_label_create(ui_Screen1);
    lv_obj_set_style_text_font(SIM_card_label, &system_status_icon_24, 0);
    lv_label_set_text(SIM_card_label, ICON_SIM_OFF_STATE_24);
    lv_obj_set_pos(SIM_card_label,35,5);

    //4G
    gnet_dsc_label = lv_label_create(ui_Screen1);
    lv_obj_set_style_text_font(gnet_dsc_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(gnet_dsc_label, "4G");
    lv_obj_set_pos(gnet_dsc_label,65,10);

    g_gnet_label = lv_label_create(ui_Screen1);
    lv_obj_set_style_text_font(g_gnet_label, &system_status_icon_24, 0);
    lv_label_set_text(g_gnet_label, ICON_GNET_CSQ_4_24);
    lv_obj_set_pos(g_gnet_label,90,7);

    //网络连接
    net_connect_label = lv_label_create(ui_Screen1);
    lv_label_set_text(net_connect_label, ICON_NET_WARN_STATE_24);
    lv_obj_set_style_text_font(net_connect_label, &system_status_icon_24, 0);
    lv_obj_set_pos(net_connect_label,120,5);

    //服务器连接
    server_connect_label = lv_label_create(ui_Screen1);
    lv_obj_set_style_text_font(server_connect_label, &system_status_icon_24, 0);
    lv_label_set_text(server_connect_label, ICON_SERVER_CON_NO_STATE_24);
    lv_obj_set_pos(server_connect_label,345,5);

    //gps
    gps_level_label = lv_label_create(ui_Screen1);
    lv_obj_set_style_text_font(gps_level_label, &system_status_icon_24, 0);
    lv_label_set_text(gps_level_label, ICON_GPS_CSQ_24);
    lv_obj_set_pos(gps_level_label,415,5);

    lv_obj_t * screen1_label = lv_label_create(ui_Screen1);
    lv_obj_set_style_text_font(screen1_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(screen1_label, "GPS");
    lv_obj_set_pos(screen1_label,380,10);

    //电流bar
    current_bar = lv_bar_create(ui_Screen1);
    lv_obj_set_size(current_bar, 32, 234 - 50);
    lv_bar_set_mode(current_bar, LV_BAR_MODE_RANGE);
    lv_obj_align(current_bar,LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_pad_all(current_bar, 0, 0);
    lv_bar_set_range(current_bar, 0, 2000);
    lv_bar_set_value(current_bar, 150, LV_ANIM_OFF);

    screen1_label = lv_label_create(ui_Screen1);
    lv_label_set_text(screen1_label,"电流");
    lv_obj_set_style_text_font(screen1_label, &ui_font_12, 0);
    lv_obj_align_to(screen1_label, current_bar, LV_ALIGN_OUT_TOP_MID, 0, 0);

    current_value = lv_label_create(ui_Screen1);
    lv_label_set_text(screen1_label, "150.0");
    lv_obj_set_style_text_font(screen1_label, &lv_font_montserrat_16, 0);
    lv_obj_align_to(screen1_label, current_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    //显示实时数据区域
    int size[9][2] = {
        {100, 67},{130,67},{130,67},
        {180,67},{180,67},
        {180,67},{180,67},
        {180,67},{180,67}
    };
    int align[9][2] = {
        {-140,33},{-22,33},{110,33},
        {-100,103},{85,103},
        {-100,173},{85,173},
        {-100,243},{85,243}
    };

    lv_obj_t * obj_i = NULL;
    const char *g_measurements_dsc[9] = {
        "速度(m/min)",   "时间(min:s)" , "深度(m)",
        "瞬时流量1(L/m)","瞬时流量2(L/m)",
        "10CM流量1(L)", "10CM流量2(L)",
        "累计流量1(L)", "累计流量2(L)"
    };

    for(int i = 0; i < 9; i++){

        //创建子区域
        obj_i = lv_obj_create(ui_Screen1);
        lv_obj_clear_flag(obj_i, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(obj_i, LV_ALIGN_TOP_MID, align[i][0], align[i][1]);
        lv_obj_set_size(obj_i,size[i][0],size[i][1]);

        //创建描述label
        screen1_label = lv_label_create(obj_i);
        lv_obj_set_style_text_font(screen1_label, &ui_font_12, 0);
        lv_label_set_text(screen1_label, g_measurements_dsc[i]);
        lv_obj_align(screen1_label, LV_ALIGN_TOP_MID, 0, -12);

        //创建数据label
        if(i == 0){
            screen1_measurements_label1 = lv_label_create(obj_i);
            lv_obj_set_style_text_font(screen1_measurements_label1, &lv_font_montserrat_42, 0);
            lv_obj_align(screen1_measurements_label1, LV_ALIGN_CENTER, 0, 12);
            lv_label_set_text(screen1_measurements_label1, "0");
        }
        else if(i == 1){
            screen1_measurements_label2 = lv_label_create(obj_i);
            lv_obj_set_style_text_font(screen1_measurements_label2, &lv_font_montserrat_42, 0);
            lv_obj_align(screen1_measurements_label2, LV_ALIGN_CENTER, 0, 12);
            lv_label_set_text(screen1_measurements_label2, "0");
        }
        else if(i == 2){
            screen1_measurements_label3 = lv_label_create(obj_i);
            lv_obj_set_style_text_font(screen1_measurements_label3, &lv_font_montserrat_42, 0);
            lv_obj_align(screen1_measurements_label3, LV_ALIGN_CENTER, 0, 12);
            lv_label_set_text(screen1_measurements_label3, "0");
        }
        else if(i == 3){
            screen1_measurements_label4 = lv_label_create(obj_i);
            lv_obj_set_style_text_font(screen1_measurements_label4, &lv_font_montserrat_42, 0);
            lv_obj_align(screen1_measurements_label4, LV_ALIGN_CENTER, 0, 12);
            lv_label_set_text(screen1_measurements_label4, "0");
        }
        else if(i == 4){
            screen1_measurements_label5 = lv_label_create(obj_i);
            lv_obj_set_style_text_font(screen1_measurements_label5, &lv_font_montserrat_42, 0);
            lv_obj_align(screen1_measurements_label5, LV_ALIGN_CENTER, 0, 12);
            lv_label_set_text(screen1_measurements_label5, "0");
        }
        else if(i == 5){
            screen1_measurements_label6 = lv_label_create(obj_i);
            lv_obj_set_style_text_font(screen1_measurements_label6, &lv_font_montserrat_42, 0);
            lv_obj_align(screen1_measurements_label6, LV_ALIGN_CENTER, 0, 12);
            lv_label_set_text(screen1_measurements_label6, "0");
        }
        else if(i == 6){
            screen1_measurements_label7 = lv_label_create(obj_i);
            lv_obj_set_style_text_font(screen1_measurements_label7, &lv_font_montserrat_42, 0);
            lv_obj_align(screen1_measurements_label7, LV_ALIGN_CENTER, 0, 12);
            lv_label_set_text(screen1_measurements_label7, "0");
        }
        else if(i == 7){
            screen1_measurements_label8 = lv_label_create(obj_i);
            lv_obj_set_style_text_font(screen1_measurements_label8, &lv_font_montserrat_42, 0);
            lv_obj_align(screen1_measurements_label8, LV_ALIGN_CENTER, 0, 12);
            lv_label_set_text(screen1_measurements_label8, "0");
        }
        else if(i == 8){
            screen1_measurements_label9 = lv_label_create(obj_i);
            lv_obj_set_style_text_font(screen1_measurements_label9, &lv_font_montserrat_42, 0);
            lv_obj_align(screen1_measurements_label9, LV_ALIGN_CENTER, 0, 12);
            lv_label_set_text(screen1_measurements_label9, "0");
        }     
    }

    //创建右侧按钮与状态灯
    screen1_led = lv_led_create(ui_Screen1);
    lv_led_set_color(screen1_led,lv_palette_main(LV_PALETTE_GREY));
    lv_obj_set_size(screen1_led,45,45);
    lv_obj_align(screen1_led,LV_ALIGN_TOP_RIGHT,0,40);

    // 添加停止按钮
    lv_obj_t * screen1_stop_btn = lv_btn_create(ui_Screen1);
    lv_obj_set_size(screen1_stop_btn,58,58);
    lv_obj_set_style_bg_color(screen1_stop_btn,lv_color_hex(0xffffff),0);
    lv_obj_set_pos(screen1_stop_btn,420,240);

    screen1_label = lv_label_create(ui_Screen1);
    lv_obj_set_style_text_font(screen1_label, &lv_font_montserrat_20, 0);
    lv_label_set_text(screen1_label, LV_SYMBOL_STOP);
    lv_obj_set_pos(screen1_label,440,260);

    // 添加play按钮
    lv_obj_t * screen1_play_btn = lv_obj_create(ui_Screen1);
    lv_obj_set_size(screen1_play_btn,58,58);
    lv_obj_set_style_bg_color(screen1_play_btn,lv_color_hex(0xffffff),0);
    lv_obj_set_pos(screen1_play_btn,420,170);

    screen1_play_btn_label = lv_label_create(ui_Screen1);
    lv_obj_set_style_text_font(screen1_play_btn_label, &lv_font_montserrat_20, 0);
    lv_label_set_text(screen1_play_btn_label, LV_SYMBOL_PLAY);
    lv_obj_set_pos(screen1_play_btn_label,440,190);

    //点击切换状态灯颜色
    lv_obj_add_event_cb(screen1_stop_btn, stop_btn_event_cb, LV_EVENT_ALL, NULL);//停止btn
    lv_obj_add_event_cb(screen1_play_btn, play_btn_event_cb, LV_EVENT_ALL, NULL);//开始btn

    //创建定时器，定时刷新数据
    if (screen1_measurements_label1 && screen1_measurements_label2 &&
        screen1_measurements_label3 && screen1_measurements_label4 &&
        screen1_measurements_label5 && screen1_measurements_label6 &&
        screen1_measurements_label7 && screen1_measurements_label8 &&
        screen1_measurements_label9) 
    {
        ui_screen1_data_timer = lv_timer_create(ui_screen1_data_timer_cb,100,NULL);
        lv_timer_pause(ui_screen1_data_timer);
    } else {
        LOG_INFO(" create data timer cb fail");
    }

    /*led闪烁定时器*/
    ui_screen1_led_timer = lv_timer_create(ui_screen1_led_timer_cb,100,NULL);
}