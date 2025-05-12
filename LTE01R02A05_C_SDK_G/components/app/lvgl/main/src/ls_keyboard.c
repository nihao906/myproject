/**
 * @file sdl_kb.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "../../lv_conf.h"
//#if !USE_SDL_SIM
#include "osi_api.h"
#include "drv_keypad.h"

#include "../../lvgl/lvgl.h"
#include "ql_log.h"
#if 1

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint32_t keycode_to_ascii(uint32_t sdl_key);

/**********************
 *  STATIC VARIABLES
 **********************/
static uint32_t last_key;      // 此时按键值
static lv_indev_state_t state; // 状态

//static bool is_key_handle_finish = true;
osiThread_t *curent_thread = NULL;

uint32_t g_map_key_cur = -1;
/*!
 * @note: 键值映射
 */
enum
{
    KB_K2 = 0x09,
    KB_K3 = 0x0f,
    KB_K4 = 0x13,
    KB_K5 = 0x0a,
    KB_K9 = 0x01,
    KB_K8 = 0x10,
    KB_K7 = 0x1a,
    KB_K6 = 0x0c
};
/*!
 * @param:
 *  key | id
 *   K2 - 0x09
 *   K3 - 0x0f
 *   K4 - 0x13
 *   K5 - 0x0a
 *   K9 - 0x01
 *   K8 - 0x10
 *   K7 - 0x1a
 *   K6 - 0x0c
 */
static void _lsKeyCB(keyMap_t id, keyState_t evt,
                     void *p) // 键盘回调函数，这里不要阻塞的接口，可以做简单的赋值
{
    uint8_t status;
    if (evt & KEY_STATE_PRESS)
    {
        status = 1;
    }
    if (evt & KEY_STATE_RELEASE)
    {
        status = 0;
    }
    //LSAPI_MENU_Printf("_lsKeyCB id=(0x%02x), evt=(%d)\n", id, evt);
    QL_LOG(QL_LOG_LEVEL_INFO,"ls_keyboard","_lsKeyCB id=(0x%02x), evt=(%d)\n", id, evt);
    switch (id)
    {
    case KB_K2:
        if (status)
        {
            last_key = LV_KEY_LEFT;         /*Save the pressed key*/
            state = LV_INDEV_STATE_PRESSED; /*Save the key is pressed now*/
            g_map_key_cur = 0;
        }
        else
        {
            state = LV_INDEV_STATE_RELEASED; /*Save the key is released but keep the last key*/
            g_map_key_cur = -1;
        }
        break;
    case KB_K3:
        if (status)
        {
            last_key = LV_KEY_PREV;         /*Save the pressed key*/
            state = LV_INDEV_STATE_PRESSED; /*Save the key is pressed now*/
            g_map_key_cur = 1;
        }
        else
        {
            state = LV_INDEV_STATE_RELEASED; /*Save the key is released but keep the last key*/
            g_map_key_cur = -1;
        }
        break;
    case KB_K4:
        if (status)
        {
            last_key = LV_KEY_NEXT;         /*Save the pressed key*/
            state = LV_INDEV_STATE_PRESSED; /*Save the key is pressed now*/
            g_map_key_cur = 2;
        }
        else
        {
            state = LV_INDEV_STATE_RELEASED; /*Save the key is released but keep the last key*/
            g_map_key_cur = -1;
        }
        break;
    case KB_K5:
        if (status)
        {
            last_key = LV_KEY_ESC;          /*Save the pressed key*/
            state = LV_INDEV_STATE_PRESSED; /*Save the key is pressed now*/
            g_map_key_cur = 3;
        }
        else
        {
            state = LV_INDEV_STATE_RELEASED; /*Save the key is released but keep the last key*/
            g_map_key_cur = -1;
        }
        break;
    case KB_K9:
        if (status)
        {
            last_key = LV_KEY_RIGHT;        /*Save the pressed key*/
            state = LV_INDEV_STATE_PRESSED; /*Save the key is pressed now*/
            g_map_key_cur = 4;
        }
        else
        {
            state = LV_INDEV_STATE_RELEASED; /*Save the key is released but keep the last key*/
            g_map_key_cur = -1;
        }
        break;
    case KB_K8:
        if (status)
        {
            last_key = LV_KEY_UP;           /*Save the pressed key*/
            state = LV_INDEV_STATE_PRESSED; /*Save the key is pressed now*/
            g_map_key_cur = 5;
        }
        else
        {
            state = LV_INDEV_STATE_RELEASED; /*Save the key is released but keep the last key*/
            g_map_key_cur = -1;
        }
        break;
    case KB_K7:
        if (status)
        {
            last_key = LV_KEY_DOWN;         /*Save the pressed key*/
            state = LV_INDEV_STATE_PRESSED; /*Save the key is pressed now*/
            g_map_key_cur = 6;
        }
        else
        {
            state = LV_INDEV_STATE_RELEASED; /*Save the key is released but keep the last key*/
            g_map_key_cur = -1;
        }
        break;
    case KB_K6:
        if (status)
        {
            last_key = LV_KEY_ENTER;        // LV_KEY_BACKSPACE;               /*Save the pressed key*/
            state = LV_INDEV_STATE_PRESSED; /*Save the key is pressed now*/
            g_map_key_cur = 7;
        }
        else
        {
            state = LV_INDEV_STATE_RELEASED; /*Save the key is released but keep the last key*/
            g_map_key_cur = -1;
        }
        break;
    default:
        break;
    }
}

/**
 * Initialize the keyboard by zhangtianlong
 */
void keyboard_init(void)
{
    curent_thread = osiThreadCurrent();
    if (curent_thread == NULL)
    {
        //LSAPI_Log_Debug("ls_keypad_example: curent_thread null\n");
        QL_LOG(QL_LOG_LEVEL_DEBUG,"lvgl","osiThreadCurrent failed\n");
        return;
    }else{
        QL_LOG(QL_LOG_LEVEL_DEBUG,"lvgl","osiThreadCurrent OK\n");
    }
    drvKeypadInit();
    drvKeypadSetCB(_lsKeyCB, KEY_STATE_PRESS | KEY_STATE_RELEASE, NULL);
    //LSAPI_Log_Debug("ls_keypad_example init success\n");
    QL_LOG(QL_LOG_LEVEL_DEBUG,"lvgl","ls_keypad_example init success\n");
}

// littlevgl 按键回调，轮训，这里可以设置一个is_key_handle_finish，来控制值得改变。
// bool keyboard_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
// {
//     if (data->state != state)
//     {
//         data->state = state;
//         data->key = last_key;
//         QL_LOG(QL_LOG_LEVEL_INFO,"ls_keyboard","key board_read:%d, %d\n", data->key, data->state);
//         //LSAPI_MENU_Printf("key board_read:%d, %d\n", data->key, data->state);
//         return true;
//     }
//     return false;
// }

void keyboard_read_test(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    if (data->state != state)
    {
        data->state = state;
        data->key = last_key;
        QL_LOG(QL_LOG_LEVEL_INFO,"ls_keyboard","key board_read:%d, %d\n", data->key, data->state);
        //LSAPI_MENU_Printf("key board_read:%d, %d\n", data->key, data->state);
        return 1;
    }
    return 0;
}
// static lv_obj_t *btn_label = NULL;

// static LSAPI_OSI_Thread_t *drawid;
// static void general_event_handler(lv_obj_t *obj, lv_event_t event)
// {
//     char buffer[40] = "hello";
//     LSAPI_Log_Debug("key general_event_handler:%d\n", event);
//     switch (event)
//     {
//     case LV_EVENT_PRESSED:
//         memcpy(buffer, "Pressed", strlen("Pressed"));
//         break;

//     case LV_EVENT_SHORT_CLICKED:
//         memcpy(buffer, "Short clicked", strlen("Short clicked"));
//         break;

//     case LV_EVENT_CLICKED:
//         memcpy(buffer, "Clicked", strlen("Clicked"));
//         break;

//     case LV_EVENT_LONG_PRESSED:
//         memcpy(buffer, "Long press", strlen("Long press"));
//         break;

//     case LV_EVENT_LONG_PRESSED_REPEAT:
//         // memcpy(buffer,"Long press repeat");
//         break;

//     case LV_EVENT_VALUE_CHANGED:
//         //  sprintf(buffer,"Value changed: %s\n", lv_event_get_data() ? (const char
//         *)lv_event_get_data() : ""); break;

//     case LV_EVENT_RELEASED:
//         memcpy(buffer, "Released", strlen("Released"));
//         break;

//     case LV_EVENT_DRAG_BEGIN:
//         memcpy(buffer, "Drag begin", strlen("Drag begin"));
//         break;

//     case LV_EVENT_DRAG_END:
//         memcpy(buffer, "Drag end", strlen("Drag end"));
//         break;

//     case LV_EVENT_DRAG_THROW_BEGIN:
//         memcpy(buffer, "Drag throw begin", strlen("Drag throw begin"));
//         break;

//     case LV_EVENT_FOCUSED:
//         memcpy(buffer, "Focused", strlen("Focused"));
//         break;
//     case LV_EVENT_DEFOCUSED:
//         memcpy(buffer, "Defocused", strlen("Defocused"));
//         break;
//     case LV_EVENT_KEY: {
//         uint32_t *data = (uint32_t *)lv_event_get_data();
//         if (data)
//         {
//             LSAPI_Log_Debug("key lv_event_get_data:%d\n", data[0]);

//             if (data[0] == 77)
//             {
//                 lv_label_set_text(btn_label, "77");
//             }
//             else if (data[0] == 88)
//             {
//                 lv_label_set_text(btn_label, "88");
//             }
//             else
//             {
//                 //  memcpy(buffer,"keyOTHER",5);
//             }
//         }
//         else
//         {
//             // memcpy(buffer,"keynodata",9);
//         }
//     }
//     default:
//         break;
//     }
//     if (btn_label)
//     {
//         // lv_label_set_text(btn_label,buffer);
//     }
// }

// static lv_obj_t *win;
// static lv_group_t *g;

// void create_win(void)
// {
//     static lv_style_t win_style;

//     lv_style_copy(&win_style, &lv_style_transp);
//     win_style.body.padding.left = LV_DPI / 6;
//     win_style.body.padding.right = LV_DPI / 6;
//     win_style.body.padding.top = LV_DPI / 6;
//     win_style.body.padding.bottom = LV_DPI / 6;
//     win_style.body.padding.inner = LV_DPI / 6;

//     win = lv_win_create(lv_disp_get_scr_act(NULL), NULL);
//     lv_win_set_title(win, "Group test");
//     lv_page_set_scrl_layout(lv_win_get_content(win), LV_LAYOUT_PRETTY);
//     lv_win_set_style(win, LV_WIN_STYLE_CONTENT, &win_style);
//     lv_group_add_obj(g, lv_win_get_content(win));
//     lv_obj_set_event_cb(lv_win_get_content(win), general_event_handler);

//     //btn_label = lv_label_create(lv_scr_act(), NULL);
//     btn_label = lv_label_create(win, NULL);

//     lv_label_set_long_mode(btn_label, LV_LABEL_LONG_BREAK);
//     lv_obj_set_width(btn_label, 180);

//     lv_obj_align(btn_label, NULL, LV_ALIGN_CENTER, 0, 90);
//     lv_label_set_align(btn_label, LV_LABEL_ALIGN_CENTER);

//     lv_label_set_text(btn_label, "Watting BTN for clicked!");
// }

// static void draw_thread(void *data)
// {
//     LSAPI_OSI_Event_t waitevent;
//     for (;;)
//     {
//         LSAPI_Log_Debug("wait event OSWaitEntry enter\n");
//         LSAPI_OSI_EventWait(LSAPI_OSI_ThreadCurrent(), &waitevent);
//         LSAPI_Log_Debug("wait event id1 = %d\n", waitevent.id);
//         switch (waitevent.id)
//         {
//         case 9:
//             LSAPI_Log_Debug("wait event id 77\n");
//             {
//                 lv_label_set_text(btn_label, "77");
//             }
//             break;
//         case 10:
//             LSAPI_Log_Debug("wait event id 88\n");
//             {
//                 lv_label_set_text(btn_label, "88");
//             }
//             break;
//         }
//     }
//     LSAPI_OSI_ThreadExit();
// }

// void keyboard_test(void)
// {
//     keyboard_init(); //初始化按键
//     g = lv_group_create(); //创立一个对象组
//     lv_indev_drv_t real_kb_drv;
//     lv_indev_drv_init(&real_kb_drv);
//     real_kb_drv.type = LV_INDEV_TYPE_KEYPAD;
//     real_kb_drv.read_cb = keyboard_read;
//     lv_indev_t *real_kb_indev = lv_indev_drv_register(&real_kb_drv);
//     lv_indev_set_group(real_kb_indev, g); //将键盘输入设备注入这个设备组里
//     create_win(); //画两个显示器件对象
// }

#endif
//#endif
