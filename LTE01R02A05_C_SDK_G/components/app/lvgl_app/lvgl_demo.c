#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"

#include "ql_lcd.h"
#include "ql_i2c.h"
#include "ql_gpio.h"
#include "ql_keypad.h"
#include "ql_log.h"
// #include "ql_pin_cfg.h"
#include "ql_fs.h"
#include "ql_sdmmc.h"
#include "lvgl.h"
#include "lvgl_demo.h"
#include "./inc/ui/ui.h"
#include "./inc/ui/ui_screen1.h"
#include "./inc/ui/fs_function.h"
#include "../../kernel/include/osi_api.h"

#define     LOG_INFO(msg, ...)          QL_LOG(QL_LOG_LEVEL_INFO, "lvgldemo", msg, ##__VA_ARGS__)

#define     MY_DISP_HOR_RES     480
#define     MY_DISP_VER_RES     320

#define     KEY1                QL_KEY_MAP_18 //切换到上一个屏幕
#define     KEY2                QL_KEY_MAP_19 //切换到下一个屏幕
#define     KEY3                QL_KEY_MAP_16 //开始/暂停
#define     KEY4                QL_KEY_MAP_17 //停止

ql_task_t lvgl_test_task;
osiThread_t *lvgl_thread = NULL;// 后台ui事件处理线程
static lv_color_t * buf_1 = NULL;
static lv_disp_t * disp = NULL;
lv_indev_t * indev_touchpad = NULL;
lv_indev_t * indev_button = NULL;
uint8_t act_screen_id = 1;//记录当前屏幕id
extern user_data_t g_ui_user_data;

uint8_t gt911_array[184] = {
	0x81, 0x00, 0x04, 0x58, 0x02, 0x0A, 0x0C, 0x20, 0x01, 0x08, 0x28, 0x05, 0x50, // 0x8047 - 0x8053
	0x3C, 0x0F, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8054 - 0x8060
	0x00, 0x89, 0x2A, 0x0B, 0x2D, 0x2B, 0x0F, 0x0A, 0x00, 0x00, 0x01, 0xA9, 0x03, // 0x8061 - 0x806D
	0x2D, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, // 0x806E - 0x807A
	0x59, 0x94, 0xC5, 0x02, 0x07, 0x00, 0x00, 0x04, 0x93, 0x24, 0x00, 0x7D, 0x2C, // 0x807B - 0x8087
	0x00, 0x6B, 0x36, 0x00, 0x5D, 0x42, 0x00, 0x53, 0x50, 0x00, 0x53, 0x00, 0x00, // 0x8088	- 0x8094
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8095 - 0x80A1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80A2 - 0x80AD
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, // 0x80AE - 0x80BA
	0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // 0x80BB - 0x80C7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80C8 - 0x80D4
	0x02, 0x04, 0x06, 0x08, 0x0A, 0x0F, 0x10, 0x12, 0x16, 0x18, 0x1C, 0x1D, 0x1E, // 0x80D5 - 0x80E1
	0x1F, 0x20, 0x21, 0x22, 0x24, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, // 0x80E2 - 0x80EE
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80EF - 0x80FB
	0x00, 0x00,                                                                   // 0x80FC - 0x80FD
}; 

/******************************************************回调函数**********************************************************/
static void dispflush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    LOG_INFO("dispflush area x1 = %d, x2 = %d, y1 = %d, y2 = %d",area->x1,area->x2,area->y1,area->y2);
    ql_lcd_write((uint16_t*)color_p, area->x1 , area->y1, area->x2 , area->y2);
    lv_disp_flush_ready(disp);
}
struct touchpad_xy{
    int16_t y;
    int16_t x;
} ;
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    //LOG_INFO("enter touchpad_read");
    //static lv_coord_t last_x = 0;
    //static lv_coord_t last_y = 0;
    static struct touchpad_xy last_point = {0,0};

    /*检测是否被触摸*/
    uint8_t state = 255;
    ql_I2cRead_16bit_addr(i2c_2,0x5D,GT911_STATE_REG,&state,1);

    uint8_t STATE = state;
    uint8_t touch_number = (state & 0x0f);
    uint8_t touch_state = (state & 0x80);

    /*代表被触摸了*/
    if((touch_state == 0x80) && (touch_number >= 1) && (STATE != 255)){
        
        /*获取触摸坐标,lvgl只支持单点触摸，因此只读取第一个坐标数据*/
        //LOG_INFO("touch_number = %d, touch_state = %d",touch_number,touch_state);

        // uint8_t low8 = 0;
        // uint8_t high8 = 0;

        // ql_I2cRead_16bit_addr(i2c_2,0x5D,(GT911_TP1_REG),&low8,1);
        // ql_I2cRead_16bit_addr(i2c_2,0x5D,(GT911_TP1_REG + 1),&high8,1);
        // last_y = (low8 | (high8 << 8));

        // ql_I2cRead_16bit_addr(i2c_2,0x5D,(GT911_TP1_REG + 2),&low8,1);
        // ql_I2cRead_16bit_addr(i2c_2,0x5D,(GT911_TP1_REG + 3),&high8,1);
        // last_x = (low8 | (high8 << 8));
        ql_I2cRead_16bit_addr(i2c_2,0x5D,GT911_TP1_REG,(uint8_t*)&last_point,4);
        last_point.y = 320 - last_point.y;

        // LOG_INFO("x = %d, y = %d",last_point.x,last_point.y);
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_point.x;
    data->point.y = last_point.y;

    //清除数据标志位
    uint8_t flag = 0;
    ql_I2cWrite_16bit_addr(i2c_2,0x5D,GT911_STATE_REG,&flag,1);
}

void ql_keypad_callback(ql_keymatrix_t keymatrix)
{   
    LOG_INFO("keymap:%d keyout:%d keyin:%d pressd:%d", keymatrix.keymap, keymatrix.keyout, keymatrix.keyin, keymatrix.keystate);
}

static int8_t button_get_pressed_id(void)
{
    uint8_t i;
    uint8_t KEYS[4] = {KEY1,KEY2,KEY3,KEY4};

    for(i = 0; i < 4; i++) {
        uint32_t press_state = 0;
        ql_keypad_state(&press_state,KEYS[i]);
        if(press_state == 1){
            return i;
        }
    }
    return -1;
}

static void button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    //LOG_INFO("enter button_read");
    static uint8_t last_btn = 0;
    int8_t btn_act = button_get_pressed_id();

    if((btn_act == 2) || (btn_act == 3)){//当点击按键为后两个时
        if(act_screen_id == 3){//如果当前活动屏幕id为第三个屏幕，则返回控制spinbox按键的id
            btn_act += 2;
        }
        else if((act_screen_id == 2) || (act_screen_id == 4)){//当前活动屏幕为第二个或者第四个的时候不对后两个按键做出反应
            return;
        }
    }

    if(btn_act >= 0) {
        data->state = LV_INDEV_STATE_PR;
        last_btn = btn_act;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
    data->btn_id = last_btn;
}

/*************************************************************************************************************************/
void gt911_init(void)
{
    ql_I2cInit(i2c_2,FAST_MODE);
    ql_pin_set_gpio(TP_nRST_pin);
    ql_pin_set_gpio(nTP_INT_pin);

    ql_gpio_init(TP_nRST_GPIO_num,GPIO_OUTPUT,PULL_NONE,LVL_LOW);
    ql_gpio_init(nTP_INT_GPIO_num,GPIO_OUTPUT,PULL_NONE,LVL_LOW);

    //设置从机地址0xBA
    ql_rtos_task_sleep_ms(10);
    ql_gpio_set_level(TP_nRST_GPIO_num,LVL_HIGH);
    ql_rtos_task_sleep_ms(10);

    //int转为悬浮输入
    ql_gpio_init(nTP_INT_GPIO_num,GPIO_INPUT,PULL_NONE,LVL_LOW);
    ql_rtos_task_sleep_ms(10);  
    
    //复位
    uint8_t temp = 2;
    ql_I2cWrite_16bit_addr(i2c_2,GT911_ADDRESS,GT911_CTRL_ADDRESS,&temp,1);

    //更新配置数组
    gt911_t config = {
        .X_Resolution = X_RESOLUTION,
        .Y_Resolution = Y_RESOLUTION,
        .Number_Of_Touch_Support = MAX_TOUCH_NUMBER,
        .ReverseX = false,
        .ReverseY = false,
        .SoftwareNoiseReduction = true,
        .SwithX2Y = false,
    };
    gt911_array[1] = config.X_Resolution & 0x00FF;
	gt911_array[2] = (config.X_Resolution >> 8) & 0x00FF;
	gt911_array[3] = config.Y_Resolution & 0x00FF;
	gt911_array[4] = (config.Y_Resolution >> 8) & 0x00FF;
	gt911_array[5] = config.Number_Of_Touch_Support;
	gt911_array[6] = 0;
	gt911_array[6] |= config.ReverseY << 7;
	gt911_array[6] |= config.ReverseX << 6;
	gt911_array[6] |= config.SwithX2Y << 3;
	gt911_array[6] |= config.SoftwareNoiseReduction << 2;
    
    //计算校验和
    uint8_t buf[2] = {0,0};//第一个下标是校验位，第二个是模式：0代表掉电不保存
    for(uint8_t i = 0; i < 184; i++){
        buf[0] += gt911_array[i];
    }
    buf[0] = (~buf[0])+1;

    //往0x8047写入配置参数
    ql_I2cWrite_16bit_addr(i2c_2,GT911_ADDRESS,GT911_CFG_ADDRESS,(uint8_t*)gt911_array,184);

    //往0x80FF写入校验和
    ql_I2cWrite_16bit_addr(i2c_2,GT911_ADDRESS,GT911_CHECK_REG,(uint8_t*)buf,2);

    //结束复位
    temp = 0;
    ql_I2cWrite_16bit_addr(i2c_2,GT911_ADDRESS,GT911_CTRL_ADDRESS,&temp,1);
}

void button_init(void)
{
    ql_keypad_out_e row[QL_KEYPAD_ROW_LENGTH] = {QL_KP_OUT0, QL_KP_OUT_NO_VALID, QL_KP_OUT_NO_VALID, QL_KP_OUT_NO_VALID, QL_KP_OUT_NO_VALID, QL_KP_OUT_NO_VALID};
    ql_keypad_in_e  col[QL_KEYPAD_COL_LENGTH] = {QL_KP_IN_NO_VALID, QL_KP_IN1, QL_KP_IN2, QL_KP_IN3, QL_KP_IN4, QL_KP_IN_NO_VALID};
    ql_keypad_init(ql_keypad_callback, row, col);
}

/*************************************************************************************************************************/
//接收到数据时对相关结构体进行处理
void ui_event_process(const uint32_t event_id, const char *param) 
{
    /*判断是否为有效id*/
    if(event_id < 9){
        LOG_INFO("receive wrong event id, %d",event_id);
        return;
    }

    /*根据id进行处理*/
    switch (event_id)
    {
    case 9: //屏1
    {
#if !TEST
        user_data_t * u_data = (user_data_t *)param;
        memcpy(&g_ui_user_data, u_data, sizeof(g_ui_user_data));
#endif
        break;
    }
    default:
        break;
    }
}

static void lvgl_demo_thread(void *param)
{
    /*初始化ST7796SLCD显示屏*/
    ql_lcd_init();

    /*初始化GT911触摸屏*/
    gt911_init();

    /*初始化EC600U按键*/
    button_init();

    /*初始化lvgl*/
    lv_init();
    LOG_INFO("lv_init end");

    /*注册disp驱动*/
    static lv_disp_draw_buf_t draw_buf_dsc_1;
    //static lv_color_t buf_1[MY_DISP_HOR_RES * 64];
    buf_1 = (lv_color_t*)malloc(480 * 320 * sizeof(lv_color_t));
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * MY_DISP_VER_RES); 

    static lv_disp_drv_t disp_drv;                    
    lv_disp_drv_init(&disp_drv);   
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;
    disp_drv.flush_cb = dispflush;
    disp_drv.draw_buf = &draw_buf_dsc_1;

    disp = lv_disp_drv_register(&disp_drv);
    LOG_INFO("disp init end");

    /*注册touchpad驱动*/
    static lv_indev_drv_t touchpad_drv;
    lv_indev_drv_init(&touchpad_drv);
    touchpad_drv.type = LV_INDEV_TYPE_POINTER;
    touchpad_drv.read_cb = touchpad_read;
    indev_touchpad = lv_indev_drv_register(&touchpad_drv);
    LOG_INFO("touchpad init end");

    /*注册button驱动*/
    static lv_indev_drv_t button_drv;
    lv_indev_drv_init(&button_drv);
    button_drv.type = LV_INDEV_TYPE_BUTTON;
    button_drv.read_cb = button_read;
    indev_button = lv_indev_drv_register(&button_drv);

    /*Assign buttons to points on the screen*/
    static const lv_point_t btn_points[6] = {
        {0, 0},   //切到上一个界面
        {450, 0}, //切刀下一个界面
        {420, 170},   //screen1 开始/暂停
        {420, 240},   //screen1 停止
        {300, 0},   //screen3 减少页数btn
        {410, 0},   //screen3 增加页数btn
    };
    lv_indev_set_button_points(indev_button, btn_points);
    LOG_INFO("button init end");

    /*demo*/
    ui_init();
    LOG_INFO("ui_init end");


    lvgl_thread = osiThreadCurrent();

    while (1)
    {
        osiEvent_t waitevent;
        lv_timer_handler();
        lv_tick_inc(20);

        // if(osiEventTryWait(lvgl_thread, &waitevent,20)){
        //     LOG_INFO("waitevent id = %d",waitevent.id);
        //     char *context = (char *)waitevent.param1;
        //     ui_event_process(waitevent.id, context);
        // }

        ql_rtos_task_sleep_ms(20);
    }
}

void lvgl_app_init(void)
{
    // ql_pin_set_func(QL_PIN_SDMMC_CMD    , QL_PIN_SDMMC_MODE_FUNC);     
	// ql_pin_set_func(QL_PIN_SDMMC_DATA_0 , QL_PIN_SDMMC_MODE_FUNC);     
	// ql_pin_set_func(QL_PIN_SDMMC_DATA_1 , QL_PIN_SDMMC_MODE_FUNC);     
	// ql_pin_set_func(QL_PIN_SDMMC_DATA_2 , QL_PIN_SDMMC_MODE_FUNC);     
	// ql_pin_set_func(QL_PIN_SDMMC_DATA_3 , QL_PIN_SDMMC_MODE_FUNC);     
	// ql_pin_set_func(QL_PIN_SDMMC_CLK    , QL_PIN_SDMMC_MODE_FUNC);  
   

    QlOSStatus err = ql_rtos_task_create(&lvgl_test_task, 1024*10, 12, "ui", lvgl_demo_thread, NULL, 5);    
    if (err != QL_OSI_SUCCESS){
        LOG_INFO("lvgl demo task created failed");
    }
}

// LOG_INFO("findDataFile err = %d",findDataFile(2024,1,10));
    // LOG_INFO("findDataFile err = %d",findDataFile(2024,1,1));
    // LOG_INFO("findDataFile err = %d",findDataFile(2024,1,12));
    // LOG_INFO("findDataFile err = %d",findDataFile(2023,1,10));

    // int fd = ql_fopen("SD:2024/01/10.txt","rb+");
    // struct Return_Data data = {0};
    // readDataFromFile(fd,&data);
    // LOG_INFO("id count = %d",data.id_count);
    // if (data.id_count > 0) {
    //     for (uint8_t i = 0; i < data.id_count; i++) {
    //         LOG_INFO("id = %d", data.dataArr[i].id);
    //         LOG_INFO("start_time = %02d:%02d:%02d", data.dataArr[i].start_hour, data.dataArr[i].start_min, data.dataArr[i].start_second);
    //         LOG_INFO("end_time = %02d:%02d:%02d", data.dataArr[i].end_hour, data.dataArr[i].end_min, data.dataArr[i].end_second);
    //         LOG_INFO("max_depth = %.2f", data.dataArr[i].max_depth);
    //         LOG_INFO("max_V = %.2f", data.dataArr[i].max_V);
    //         LOG_INFO("average_I = %.2f", data.dataArr[i].average_I);
    //         LOG_INFO("count = %d", data.dataArr[i].count);
    //         LOG_INFO("offset = %d", data.dataArr[i].offset);
    //     } 
    // }
    // ql_fclose(fd);