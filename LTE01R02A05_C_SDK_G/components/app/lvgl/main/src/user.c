// /*********************
//  *      INCLUDES
//  *********************/
// #include "user.h"
// #define _DEFAULT_SOURCE /* needed for usleep() */
// #include <stdlib.h>
// #include <unistd.h>
// #define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
// #include <SDL2/SDL.h>

// #include "lv_drivers/display/monitor.h"
// #include "lv_drivers/indev/keyboard.h"
// #include "lv_drivers/indev/mouse.h"
// #include "lv_drivers/indev/mousewheel.h"
// #include "lv_examples/lv_demo.h"
// #include "lvgl/examples/lv_examples.h"
// #include "lvgl/lvgl.h"

// #include <string.h>

// /**
//  * A task to measure the elapsed time for LVGL
//  * @param data unused
//  * @return never return
//  */
// static int tick_thread(void *data) {
//   (void)data;

//   while (1) {
//     SDL_Delay(5);
//     lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
//   }

//   return 0;
// }

// /**
//  * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
//  * library
//  */
// void hal_init(void) {
//   /* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
//   monitor_init();
//   /* Tick init.
//    * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
//    * how much time were elapsed Create an SDL thread to do this*/
//   SDL_CreateThread(tick_thread, "tick", NULL);

//   /*Create a display buffer*/
//   static lv_disp_draw_buf_t disp_buf1;
//   static lv_color_t buf1_1[MONITOR_HOR_RES * 100];
//   static lv_color_t buf1_2[MONITOR_HOR_RES * 100];
//   lv_disp_draw_buf_init(&disp_buf1, buf1_1, buf1_2, MONITOR_HOR_RES * 100);

//   /*Create a display*/
//   static lv_disp_drv_t disp_drv;
//   lv_disp_drv_init(&disp_drv); /*Basic initialization*/
//   disp_drv.draw_buf = &disp_buf1;
//   disp_drv.flush_cb = monitor_flush;
//   disp_drv.hor_res = MONITOR_HOR_RES;
//   disp_drv.ver_res = MONITOR_VER_RES;
//   disp_drv.antialiasing = 1;

//   lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

//   lv_theme_t *th =
//       lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
//                             LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
//   lv_disp_set_theme(disp, th);

//   lv_group_t *g = lv_group_create();
//   lv_group_set_default(g);

//   /* Add the mouse as input device
//    * Use the 'mouse' driver which reads the PC's mouse*/
//   mouse_init();
//   static lv_indev_drv_t indev_drv_1;
//   lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
//   indev_drv_1.type = LV_INDEV_TYPE_POINTER;

//   /*This function will be called periodically (by the library) to get the mouse position and state*/
//   indev_drv_1.read_cb = mouse_read;
//   lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);

//   keyboard_init();
//   static lv_indev_drv_t indev_drv_2;
//   lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
//   indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
//   indev_drv_2.read_cb = keyboard_read;
//   lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
//   lv_indev_set_group(kb_indev, g);
//   mousewheel_init();
//   static lv_indev_drv_t indev_drv_3;
//   lv_indev_drv_init(&indev_drv_3); /*Basic initialization*/
//   indev_drv_3.type = LV_INDEV_TYPE_ENCODER;
//   indev_drv_3.read_cb = mousewheel_read;

//   lv_indev_t *enc_indev = lv_indev_drv_register(&indev_drv_3);
//   lv_indev_set_group(enc_indev, g);

//   /*Set a cursor for the mouse*/
//   LV_IMG_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
//   lv_obj_t *cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
//   lv_img_set_src(cursor_obj, &mouse_cursor_icon); /*Set the image source*/
//   lv_indev_set_cursor(mouse_indev, cursor_obj); /*Connect the image  object to the driver*/
// }

// /*!
//  * @function: lv_get_font
//  * @note: 获取指定的字体种类
//  * @param: font_type: 字体种类英文名称
//  * @return: 指向该种类变量的指针
//  */
// lv_font_t *lv_get_font(const char *font_type) {
//   if (font_type == NULL)
//     return NULL;
//   if (!strcmp("montserrat_8", font_type))
//     return &lv_font_montserrat_8;
//   if (!strcmp("montserrat_10", font_type))
//     return &lv_font_montserrat_10;
//   if (!strcmp("montserrat_12", font_type))
//     return &lv_font_montserrat_12;
//   if (!strcmp("montserrat_14", font_type))
//     return &lv_font_montserrat_14;
//   if (!strcmp("montserrat_16", font_type))
//     return &lv_font_montserrat_16;
//   if (!strcmp("montserrat_18", font_type))
//     return &lv_font_montserrat_18;
//   if (!strcmp("montserrat_20", font_type))
//     return &lv_font_montserrat_20;
//   if (!strcmp("montserrat_22", font_type))
//     return &lv_font_montserrat_22;
//   if (!strcmp("montserrat_24", font_type))
//     return &lv_font_montserrat_24;
//   if (!strcmp("montserrat_26", font_type))
//     return &lv_font_montserrat_26;
//   if (!strcmp("montserrat_28", font_type))
//     return &lv_font_montserrat_28;
//   if (!strcmp("montserrat_30", font_type))
//     return &lv_font_montserrat_30;
//   if (!strcmp("montserrat_32", font_type))
//     return &lv_font_montserrat_32;
//   if (!strcmp("montserrat_34", font_type))
//     return &lv_font_montserrat_34;
//   if (!strcmp("montserrat_36", font_type))
//     return &lv_font_montserrat_36;
//   if (!strcmp("montserrat_38", font_type))
//     return &lv_font_montserrat_38;
//   if (!strcmp("montserrat_40", font_type))
//     return &lv_font_montserrat_40;
//   if (!strcmp("montserrat_42", font_type))
//     return &lv_font_montserrat_42;
//   if (!strcmp("montserrat_44", font_type))
//     return &lv_font_montserrat_44;
//   if (!strcmp("montserrat_46", font_type))
//     return &lv_font_montserrat_46;
//   if (!strcmp("montserrat_48", font_type))
//     return &lv_font_montserrat_48;
//   if (!strcmp("montserrat_28_compressed", font_type))
//     return &lv_font_montserrat_28_compressed;
//   if (!strcmp("montserrat_12_subpx", font_type))
//     return &lv_font_montserrat_12_subpx;
//   if (!strcmp("unscii_8", font_type))
//     return &lv_font_unscii_8;
//   if (!strcmp("unscii_16", font_type))
//     return &lv_font_unscii_16;
//   if (!strcmp("dejavu_16_persian_hebrew", font_type))
//     return &lv_font_dejavu_16_persian_hebrew;
//   if (!strcmp("simsun_16_cjk", font_type))
//     return &lv_font_simsun_16_cjk;
//   return NULL;
// }
