#ifndef __USER_H__
#define __USER_H__
#include "lvgl/src/font/lv_font.h"
/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
void hal_init(void);
lv_font_t *lv_get_font(const char *font_type);
#endif
