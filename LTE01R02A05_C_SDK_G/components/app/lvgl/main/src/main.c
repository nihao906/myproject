
/*
problem:
1.drvLcdOpenV2打开失败
*/

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>

#if USE_SDL_SIM
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
//#include <SDL2/SDL.h>
#endif

#include "../../lv_drivers/display/monitor.h"
#include "../../lv_drivers/indev/keyboard.h"
#include "../../lv_drivers/indev/mouse.h"
#include "../../lv_drivers/indev/mousewheel.h"
#include "../../lv_examples/lv_demo.h"
#include "../../lvgl/examples/lv_examples.h"
#include "../../lvgl/lvgl.h"


#if !USE_SDL_SIM
#include "drv_lcd.h"
#include "drv_backlight.h"

#include "osi_api.h"
#include "hal_chip.h"
#include "drv_names.h"
#include "drv_lcd_v2.h"

#include "ql_i2c.h"
#include "MyI2C.h"
#include "ql_lcd.h"
#include "ql_api_osi.h"
#include "ql_log.h"
#endif

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "lvgl", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "lvgl", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "lvgl", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "lvgl", msg, ##__VA_ARGS__)

#define GT911_TEST 0
#if GT911_TEST
#include "GT911.h"
GT911_Config_t GT911_Config = {
    .X_Resolution = 480,
    .Y_Resolution = 320,
    .Number_Of_Touch_Support = 5,
    .ReverseX = false,
    .ReverseY = false,
    .SoftwareNoiseReduction = true,
    .SwithX2Y = false
};
#endif

static void hal_init(void);
static int tick_thread(void *data);


#if !USE_SDL_SIM
bool is_os_sleep = false;
uint8_t open_backlight_flag = 1;
// lv_group_t *g;
static lv_indev_t *indev_button;

//extern void ui_event_process(const uint32_t event_id, const char *param);
extern void keyboard_read_test(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
extern void keyboard_init(void);
extern void lv_demo_mcxa(void);
extern void lv_port_indev_init_test(void);

void lv_log_print_g_cb(const char *buf)
{
    QL_LOG_PRINTF_TAG(QL_LOG_LEVEL_DEBUG, OSI_MAKE_LOG_TAG('L', 'V', 'G', 'L'), buf);
}
#endif

#if USE_SDL_SIM
int main(int argc, char **argv)
#else
void lvgl_thread(void * param)
#endif
{
    osiThreadSleep(5000);
    LOGI("enter lvgl_thread after start 20s\n");

    #if GT911_TEST
        ql_errcode_gpio err = 0;       
        ql_pin_set_func(62,1);
        ql_pin_set_func(137,1); 
        ql_gpio_init(GPIO_4,GPIO_OUTPUT,PULL_UP,0);
        ql_gpio_init(GPIO_5,GPIO_OUTPUT,PULL_UP,0);

        /*通过int引脚唤醒gt911*/
        int level = 2;
        ql_gpio_set_level(GPIO_5,1);
        ql_gpio_get_level(GPIO_5,&level);
        LOGI("INT level = %d\n",level);
        osiThreadSleep(5);
        ql_gpio_set_level(GPIO_5,0);
        ql_gpio_get_level(GPIO_5,&level);
        LOGI("INT level = %d\n",level);
        osiThreadSleep(5);

        /*设置从机地址为0xBA*/
        err = ql_gpio_set_level(GPIO_4,0); //RST QL_GPIO_INVALID_PARAM_ERR
        //if(err!=0) LOGI("ql_gpio_set_level GPIO_4 fail,err = %d\n",err);
        osiThreadSleep(10);
        err = ql_gpio_set_level(GPIO_5,0); //INT
        //if(err!=0) LOGI("ql_gpio_set_level GPIO_5 fail,err = %d\n",err);
        osiThreadSleep(20);
        err = ql_gpio_set_level(GPIO_4,1);
        //if(err!=0) LOGI("ql_gpio_set_level GPIO_4 fail,err = %d\n",err);
        osiThreadSleep(20);

        err = ql_gpio_init(GPIO_5,GPIO_INPUT,PULL_NONE,LVL_LOW);
        //if(err!=0) LOGI("ql_gpio_init GPIO_5 fail\n");
        osiThreadSleep(100);

        #if 0
        if (i2c_init() == 0){
            LOGI("i2c_init failed\n");
        }
        #else
        ql_errcode_i2c_e e = ql_I2cInit(i2c_2,0);
        if(e!=0) LOGI("ql_I2cInit failed,err = %d\n",e);
        #endif
    #else

    #if !USE_SDL_SIM
    lv_log_register_print_cb(lv_log_print_g_cb);
    LOGI("lv_log_register_print_cb end\n");
    #endif
    /*Initialize LVGL*/
    lv_init();
    LOGI("lv_init end\n");
    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init();
    LOGI("hal_init end\n");

    lv_demo_mcxa();
    LOGI("lv_demo_mcxa end\n");
    #endif
#if USE_SDL_SIM
    while (1)
    {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_timer_handler();
        usleep(5 * 1000);
    }
#else
    
    
    while (1)
    {
        LOGI("enter while recycle\n");

        #if GT911_TEST
            #if 0
            /*
            1.地址
            2.gpio初始化模式
            */
            if(i2c_start() == 0){
                LOGI("i2c_start failed\n");
            }
            if(i2c_send_byte(0xBA) == 0){ //0xBA, 0x5D   
                LOGI("i2c_send_byte failed\n");
            }
            
            ql_LvlMode ack = i2c_receive_ack();
            if(i2c_stop() == 0){
                LOGI("i2c_stop failed\n");
            }
            LOGI("ack = %d\n",ack);
            osiThreadSleep(1000);
            
            // err = ql_gpio_set_level(GPIO_4,1);
            // if(err!=0) LOGI("ql_gpio_set_level GPIO_4 1 fail,err = %d\n",err);
            // int level = 2;
            // ql_gpio_get_level(GPIO_4,&level);
            // LOGI("level: %d\n",level);
            // osiThreadSleep(500);

            // err = ql_gpio_set_level(GPIO_4,0); 
            // if(err!=0) LOGI("ql_gpio_set_level GPIO_4 0 fail,err = %d\n",err);
            // ql_gpio_get_level(GPIO_4,&level);
            // LOGI("level: %d\n",level);
            // osiThreadSleep(500);

            #else

            
           
                ql_errcode_i2c_e err = ql_I2cWrite_16bit_addr(i2c_2,0xBA,0x8040,2,1);
                if(err != 0){
                    LOGI("ql_I2cWrite failed\n");
                }else{
                    LOGI("ql_I2cWrite success\n");
                    break;
                }
                
            

            #endif
        osiThreadSleep(500);  // 2s
        #else
        if (is_os_sleep)
        {
            LOGI("is_os_sleep is true\n");
            osiEvent_t waitevent;
            //drvLcdBackLightClose(); // 关背光  drvLcdSetBackLightEnable(p,false);
            drvLcdClose();  //drvLcdCloseV2(p);
            osiEventWait(osiThreadCurrent(), &waitevent);
            drvLcdInit();   //drvLcdInitV2(); drvLcdOpenV2(p);
            open_backlight_flag = 1; // 开背光 drvLcdSetBackLightEnable(p,true);
            lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);
            lv_task_handler();
        }else
        {
            LOGI("is_os_sleep is false\n");
            osiThreadSleep(20);
            lv_task_handler();
            lv_tick_inc(20);

        }
        #endif
    }
    osiThreadExit();
#endif
    return;
}

void prvDispFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    LOGI("disp flush %d/%d/%d/%d", area->x1, area->y1, area->x2, area->y2);

    ql_lcd_write((uint16_t*)color_p, area->x1 , area->y1, area->x2 , area->y2);
    lv_disp_flush_ready(disp);
    // lcdFrameBuffer_t data;
    // lcdDisplay_t display;
    // if (disp_drv)
    // {
    //     memset(&data, 0x00, sizeof(lcdFrameBuffer_t));
    //     memset(&display, 0x00, sizeof(lcdDisplay_t));
    //     data.buffer = (uint16*)color_p;
    //     data.width = area->x2 - area->x1 + 1;
    //     data.height = area->y2 - area->y1 + 1;
    //     data.region_x = 0;
    //     data.region_y = 0;
    //     data.rotation = disp_drv->rotated;
    //     data.colorFormat = LCD_RESOLUTION_RGB565;
    //     display.x = area->x1;
    //     display.y = area->y1;
    //     display.height = area->y2 - area->y1 + 1;
    //     display.width = area->x2 - area->x1 + 1;

    //     drvLcdBlockTransfer(&data, &display);
    //     if (open_backlight_flag)
    //     {
    //         // drvLcdBackLightOpen();
    //         open_backlight_flag = 0;
    //     }
    //     lv_disp_flush_ready(disp_drv);
    // }
}
// extern const lcdSpec_t g_lcd_st7796;
// static lcdSpec_t *lcd_cfg_tab[] = {
//     (lcdSpec_t *)&g_lcd_st7796,
// };

/*Test if `id` button is pressed or not*/
static bool button_is_pressed(uint8_t id)
{
    extern uint32_t g_map_key_cur;
    /*Your code comes here*/
    if (g_map_key_cur == id)
        return true;
    return false;
}

/*Get ID  (0, 1, 2 ..) of the pressed button*/
static int8_t button_get_pressed_id(void)
{
    uint8_t i;

    /*Check to buttons see which is being pressed (assume there are 2 buttons)*/
    for (i = 0; i < 8; i++)
    {
        /*Return the pressed button's ID*/
        if (button_is_pressed(i))
        {
            return i;
        }
    }

    /*No button pressed*/
    return -1;
}

/*Will be called by the library to read the button*/
static void button_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{

    static uint8_t last_btn = 0;

    /*Get the pressed button's ID*/
    int8_t btn_act = button_get_pressed_id();

    if (btn_act >= 0)
    {
        data->state = LV_INDEV_STATE_PR;
        last_btn = btn_act;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Save the last pressed button's ID*/
    data->btn_id = last_btn;
}


static bool prvLvInitLcd(void)
{
    // lvglContext_t *d = &gLvCtx;
    static lv_disp_draw_buf_t disp_buf;
    unsigned int pixel_cnt = LV_HOR_RES_MAX * LV_VER_RES_MAX;
    lv_color_t *buf = (lv_color_t *)malloc(pixel_cnt * sizeof(lv_color_t));
    if (buf == NULL){
        LOGE("malloc lcd buf failed\n");
        return false;
    }

    lv_disp_draw_buf_init(&disp_buf, buf, NULL, pixel_cnt);
    memset(buf, 0x1f, pixel_cnt * sizeof(lv_color_t));
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = prvDispFlush;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.hor_res = LV_HOR_RES_MAX;    /*Set the horizontal resolution in pixels*/
    disp_drv.ver_res = LV_VER_RES_MAX;    /*Set the vertical resolution in pixels*/
    disp_drv.antialiasing = 1;            // 抗锯齿
    disp_drv.full_refresh = 0;            // 全屏刷新
    lv_disp_t * lv_disp = lv_disp_drv_register(&disp_drv); // pointer copy; 
    lv_disp_trig_activity(lv_disp);
    return true;
}


static void hal_init(void)
{
    // LOGI("enter hal_init\n");
    // bool halPmuSetPowerLevel_flag = halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
    // if(halPmuSetPowerLevel_flag){
    //     LOGI("halPmuSetPowerLevel success\n");
    // }else{
    //     LOGI("halPmuSetPowerLevel failed\n");
    // }

    // bool halPmuSwitchPower_flag = halPmuSwitchPower(HAL_POWER_LCD, true, true);
    // if(halPmuSwitchPower_flag){
    //     LOGI("halPmuSwitchPower success\n");
    // }else{
    //     LOGI("halPmuSwitchPower failed\n");
    // }

    // osiThreadSleep(100);
    // LOGI("osiThreadSleep 100 ms\n");
 
    // ql_lcd_set_level_brightness(1, 0);
    // ql_lcd_set_level_brightness(2, 7);
    // ql_lcd_set_level_brightness(3, 15);
    // ql_lcd_set_level_brightness(4, 31);
    // ql_lcd_set_level_brightness(5, 63);
    // ql_lcd_set_brightness(5);
    // lv_color_t *buf_1 = (lv_color_t *)malloc(480*320*sizeof(lv_color_t));
    // if(buf_1 == NULL){
    //     LOGE("malloc lcd buf failed\n");
    // }
    // else{
    //     LOGI("malloc lcd buf ok\n");
    // }
    // osiThreadSleep(5000);
    // drvLcd_t *lcd = drvLcdGetByname(DRV_NAME_LCD1);
    // if(lcd == NULL){
    //     LOGI("drvLcdGetByname failed\n");
    // }else{
    //     LOGI("drvLcdGetByname OK,instance name is %p\n",(void*)lcd);
    // }
    // bool drvLcdOpenV2_flag = drvLcdOpenV2(lcd);                     //fail!!!!!
    // if(drvLcdOpenV2_flag)
    // {
    //     LOGI("drvLcdOpenV2 sucess\n");
    //     bool drvLcdFill_flag = drvLcdFill(lcd, 0, NULL, true);
    //     if(drvLcdFill_flag){
    //         LOGI("drvLcdFill sucess\n");
    //     }else{
    //         LOGI("drvLcdFill failed\n");
    //     }
    //     drvLcdSetBackLightEnable(lcd, true);
    //     LOGI("drvLcdSetBackLightEnable end\n");
    // //    lvGuiInit(lvglAnimCreate);
    // }else{
    //     LOGI("drvLcdOpenV2 fail\n");
    // }
    // int ret = drvLcdInit();
    // LOGI("drvLcdInit ret=%d\n",ret);
    /*A static or global variable to store the buffers*/
    /*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL instead buf_2 */
    /*Static or global buffer(s). The second buffer is optional*/
    // static lv_color_t buf_1[60000];
    // static lv_color_t buf_1[480*120];

    // static lv_disp_draw_buf_t disp_buf;
    // ={
    //     .buf1 = NULL,
    //     .buf2 = NULL,
    //     .buf_act = NULL,
    //     .size = 0,
    //     .area = {
    //         .x1 = 0,
    //         .y1 = 0,
    //         .x2 = 0,
    //         .y2 = 0,
    //     },
    //     .flushing = 0,
    //     .flushing_last = 0,
    //     .last_area = 0,
    //     .last_part = 0
    // };
    
    // lv_disp_draw_buf_init(&disp_buf, buf_1,NULL,480*320);
    // LOGI("buf1 size = %d\n",sizeof(buf_1));

    /*A variable to hold the drivers. Must be static or global.*/
    // static lv_disp_drv_t disp_drv;
    // = {
    //     .hor_res = 0,
    //     .ver_res = 0,
    //     .physical_hor_res = 0,
    //     .physical_ver_res = 0,
    //     .offset_x = 0,
    //     .offset_y = 0,
    //     .draw_buf = NULL,
    //     .full_refresh = 0,
    //     .sw_rotate = 0,
    //     .antialiasing = 0,// 抗锯齿
    //     .rotated = 0,
    //     .screen_transp = 0,
    //     .dpi = 0,
    //     .flush_cb = NULL,
    //     .rounder_cb = NULL,
    //     .set_px_cb = NULL,
    //     .monitor_cb = NULL,
    //     .wait_cb = NULL,
    //     .clean_dcache_cb = NULL,
    //     .gpu_wait_cb = NULL,
    //     .drv_update_cb = NULL,
    //     .gpu_fill_cb = NULL,
    //     .color_chroma_key = NULL,
    //     .user_data = NULL,
    // };
    /*Basic initialization*/
    // lv_disp_drv_init(&disp_drv);
    // disp_drv.draw_buf = &disp_buf;        /*Set an initialized buffer*/
    // disp_drv.flush_cb = ls_monitor_flush; /*Set a flush callback to draw to the display*/
    // disp_drv.hor_res = 480;    /*Set the horizontal resolution in pixels*/
    // disp_drv.ver_res = 320;    /*Set the vertical resolution in pixels*/
    // disp_drv.antialiasing = 1;            // 抗锯齿
    // disp_drv.full_refresh = 0;            // 全屏刷新

    // lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    // if(disp == NULL){
    //     LOGI("lv_disp_drv_register fail\n");
    // }else{
    //     LOGI("lv_disp_drv_register ok\n");
    // }
    if(ql_lcd_init() != QL_SUCCESS )
    {
        LOGE("LCD init failed");
    }
    ql_lcd_clear_screen(0xf800);
     osiThreadSleep(1000);
    prvLvInitLcd();
    /*初始化输入设备*/
    lv_port_indev_init_test();
    LOGI("lv_disp_drv_register END\n");
#if 0

    lv_group_t *g = lv_group_create(); // 取消键盘映射
    lv_group_set_default(g);

    keyboard_init();
    static lv_indev_drv_t indev_drv_2;
    lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
    indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_2.read_cb = keyboard_read_test;
    lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
    if(kb_indev == NULL){
        LOGI("lv_indev_drv_register fail\n");
    }else{
        LOGI("lv_indev_drv_register ok\n");
    }
    lv_indev_set_group(kb_indev, g);

    /*Initialize your button if you have*/
    // button_init();
    /*Register a button input device*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_BUTTON;
    indev_drv.read_cb = button_read;
    indev_button = lv_indev_drv_register(&indev_drv);

    /*Assign buttons to points on the screen*/
    static const lv_point_t btn_points[8] = {
        {20, 94},      /*k2*/
        {20 - 6, 138}, /*k3*/
        {20 - 6, 182}, /*k4*/
        {20, 226},     /*k5*/
        {460, 94},     /*k9*/
        {460, 138},    /*k8*/
        {460, 182},    /*k7*/
        {460, 226},    /*k6*/
    };
    lv_indev_set_button_points(indev_button, btn_points);
#endif
}


//#endif
#if USE_SDL_SIM
/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data)
{
    (void)data;

    while (1)
    {
        SDL_Delay(5);
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}
#else
/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data)
{
    (void)data;

    while (1)
    {
        // SDL_Delay(5);
        //LSAPI_OSI_ThreadSleep(5);
        osiThreadSleep(5);
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

ql_task_t lvgl_app_task = NULL;
void lvgl_app_init(void *parm)
{
    QlOSStatus err;
    err = ql_rtos_task_create(&lvgl_app_task, 1024*16, APP_PRIORITY_NORMAL, "lvgl", lvgl_thread, NULL, 1);
    if(err != QL_OSI_SUCCESS)
    {
        LOGE("create task failed");
    }
}

#endif
