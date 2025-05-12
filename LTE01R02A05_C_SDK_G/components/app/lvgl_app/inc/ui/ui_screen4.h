#include "lvgl.h"

#define THEME_COLOR_WHITE_DEF 0xFFFFFF
#define THEME_COLOR_BTN_BLUE_DEF 0x2196F3

extern const lv_font_t lv_font_montserrat_16;

typedef enum{
    LEVEL_INSTUMENT_PANEL_ALL = 0,
    LEVEL_INSTUMENT_PANEL_RED,
    LEVEL_INSTUMENT_PANEL_YELLOW,
    LEVEL_INSTUMENT_PANEL_GREEN,
    LEVEL_INSTRUMENT_LED,
    LEVEL_INSTRUMENT_CROSSHAIR
} level_instrument_part_t;

typedef struct {
    lv_obj_t * red_bg;
    lv_obj_t * yellow_bg;
    lv_obj_t * green_bg;
    lv_obj_t * horizontal_line;
    lv_obj_t * vertical_line;
}my_panel_t;

typedef struct {
    my_panel_t * panel;
    lv_obj_t * led;
    int size;
    int x;
    int y;
}level_instrument_t;

extern level_instrument_t * level_instrument;

void ui_screen4_init(void);

//设置控件对应部件大小
void level_instrument_set_size(level_instrument_t * level_instrument, lv_coord_t size, level_instrument_part_t part);

//设置光标在面板上的位置(光标以坐标轴方向为准,基于原点偏移)
void level_instrument_set_cursor_pos(level_instrument_t * level_instrument,lv_coord_t x_offset, lv_coord_t y_offset);

//设置光标颜色
void level_instrument_set_cursor_color(level_instrument_t * level_instrument,lv_color_t color);

//设置光标亮度
void level_instrument_set_cursor_brightness(level_instrument_t * level_instrument,uint8_t bright);

//设置控件整体的对齐
void level_instrument_set_align(level_instrument_t * level_instrument,lv_align_t align,lv_coord_t x_offset,lv_coord_t y_offset);

//设置面板透明度
void level_instrument_set_opa(level_instrument_t * level_instrument,lv_opa_t opa);