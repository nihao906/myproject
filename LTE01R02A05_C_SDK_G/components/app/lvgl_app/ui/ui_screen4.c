#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/ui/ui_screen4.h"

#include "osi_api.h"
#define SIZE 300

lv_obj_t * ui_Screen4;
level_instrument_t * level_instrument;
extern lv_obj_t * ui_Screen1;
extern lv_obj_t * ui_Screen3;

/**********************************************  自定义控件创建  *****************************************/
my_panel_t * panel_create(lv_obj_t * obj)
{
    /*创建基础红色背景*/
    lv_obj_t * red_bg = lv_obj_create(obj);
    lv_obj_align(red_bg,LV_ALIGN_CENTER,0,20);
    lv_obj_set_size(red_bg,SIZE,SIZE);
    lv_obj_set_style_radius(red_bg,200,0);
    lv_obj_set_style_bg_color(red_bg,lv_palette_main(LV_PALETTE_RED),0);
    lv_obj_set_style_opa(red_bg,150,0);
    lv_obj_set_style_border_width(red_bg,0,0);
    lv_obj_set_style_outline_width(red_bg,0,0);

    /*在红色背景上创建黄色背景*/
    lv_obj_t * yellow_bg = lv_obj_create(red_bg);
    lv_obj_center(yellow_bg);
    lv_obj_set_size(yellow_bg,SIZE/3*2,SIZE/3*2);
    lv_obj_set_style_radius(yellow_bg,125,0);
    lv_obj_set_style_bg_color(yellow_bg,lv_palette_main(LV_PALETTE_YELLOW),0);
    lv_obj_set_style_opa(yellow_bg,150,0);
    lv_obj_set_style_border_width(yellow_bg,0,0);
    lv_obj_set_style_outline_width(yellow_bg,0,0);

    /*在绿色背景上创建绿色背景*/
    lv_obj_t * green_bg = lv_obj_create(yellow_bg);
    lv_obj_center(green_bg);
    lv_obj_set_size(green_bg,SIZE/3,SIZE/3);
    lv_obj_set_style_radius(green_bg,125,0);
    lv_obj_set_style_bg_color(green_bg,lv_palette_main(LV_PALETTE_GREEN),0);
    lv_obj_set_style_opa(green_bg,150,0);
    lv_obj_set_style_border_width(green_bg,0,0);
    lv_obj_set_style_outline_width(green_bg,0,0);

    //创建十字线
    lv_obj_t * horizontal_line = lv_obj_create(obj);
    lv_obj_set_size(horizontal_line,SIZE+20,2);
    lv_obj_set_style_bg_color(horizontal_line,lv_color_hex(0x000000),0);
    lv_obj_align(horizontal_line,LV_ALIGN_CENTER,0,20);
    lv_obj_set_style_border_width(horizontal_line,0,0);
    lv_obj_set_style_outline_width(horizontal_line,0,0);
    lv_obj_set_style_shadow_width(horizontal_line,0,0);

    lv_obj_t * vertical_line = lv_obj_create(obj);
    lv_obj_set_size(vertical_line,2,SIZE+20);
    lv_obj_set_style_bg_color(vertical_line,lv_color_hex(0x000000),0);
    lv_obj_align(vertical_line,LV_ALIGN_CENTER,0,20);
    lv_obj_set_style_border_width(vertical_line,0,0);
    lv_obj_set_style_outline_width(vertical_line,0,0);
    lv_obj_set_style_shadow_width(vertical_line,0,0);

    my_panel_t * my_panel = (my_panel_t *)lv_mem_alloc(sizeof(my_panel_t));
    my_panel -> red_bg = red_bg;
    my_panel -> yellow_bg = yellow_bg;
    my_panel -> green_bg = green_bg;
    my_panel -> horizontal_line = horizontal_line;
    my_panel -> vertical_line = vertical_line;

    // LV_LOG_USER(
    //     "red_bg = %p\n yellow_bg = %p\n green_bg = %p\n horizontal_line = %p\n vertical_line = %p\n",
    //     my_panel -> red_bg,
    //     my_panel -> yellow_bg,
    //     my_panel -> green_bg,
    //     my_panel -> horizontal_line,
    //     my_panel -> vertical_line
    // );

    return my_panel;
}

lv_obj_t* led_create(lv_obj_t * obj)
{
    lv_obj_t * led = lv_led_create(obj);
    lv_obj_set_size(led, 10, 10);
    lv_obj_align(led,LV_ALIGN_CENTER,0,20);
    //LV_LOG_USER("led = %p", led);
    return led;
}

level_instrument_t * level_instrument_create(lv_obj_t * obj)
{
    level_instrument_t * level_instrument = (level_instrument_t *)lv_mem_alloc(sizeof(level_instrument_t));
    level_instrument -> panel = panel_create(obj);
    level_instrument -> led = led_create(obj);
    level_instrument ->size =270;
    level_instrument ->x = 0;
    level_instrument ->y = 0;
    return level_instrument;
}

/****************************************************自定义控件功能**********************************************************/

//设置控件对应部件大小
void level_instrument_set_size(level_instrument_t * level_instrument, lv_coord_t size, level_instrument_part_t part)
{
    if(part == LEVEL_INSTUMENT_PANEL_ALL){
        lv_obj_set_size(level_instrument -> panel ->red_bg , size, size);
        lv_obj_set_size(level_instrument -> panel ->yellow_bg , size/3*2, size/3*2);
        lv_obj_set_size(level_instrument -> panel ->green_bg , size/3, size/3);
        lv_obj_set_size(level_instrument -> panel ->horizontal_line , size+10, 2);
        lv_obj_set_size(level_instrument -> panel ->vertical_line , 2, size+10);
        level_instrument->size = size;//更新大小
    }else if(part == LEVEL_INSTUMENT_PANEL_RED){
        lv_obj_set_size(level_instrument -> panel ->red_bg , size, size);
    }else if(part == LEVEL_INSTUMENT_PANEL_YELLOW){
        lv_obj_set_size(level_instrument -> panel ->yellow_bg , size/3*2, size/3*2);
    }else if(part == LEVEL_INSTUMENT_PANEL_GREEN){
        lv_obj_set_size(level_instrument -> panel ->green_bg , size/3, size/3);
    }else if(part == LEVEL_INSTRUMENT_CROSSHAIR){
        lv_obj_set_size(level_instrument->panel->horizontal_line,size,2);
        lv_obj_set_size(level_instrument->panel->vertical_line,2,size);
    }else if(part == LEVEL_INSTRUMENT_LED){
        lv_obj_set_size(level_instrument -> led , size, size);
    }else{
        return;
    }
}

//设置光标在面板上的位置(光标以坐标轴方向为准,基于原点偏移)
void level_instrument_set_cursor_pos(level_instrument_t * level_instrument,lv_coord_t x_offset, lv_coord_t y_offset)
{
    y_offset = -y_offset;
    int size = level_instrument->size;
    //LV_LOG_USER("panel size = %d\n",size);

    //判断坐标是否超过panel
    if (((size * size) >= x_offset * x_offset + y_offset * y_offset) &&
        LV_ABS(x_offset) <= size/2 && 
        LV_ABS(y_offset) <= size/2){
        //光标在内部
        level_instrument->x = x_offset;
        level_instrument->y = y_offset;
        lv_obj_align(level_instrument->led,LV_ALIGN_CENTER,x_offset,y_offset);
        //LV_LOG_USER("cursor is in panel\n");
    }else{
        //光标在外部
        //LV_LOG_USER("cursor is out panel\n");
    } 
}

//设置光标颜色
void level_instrument_set_cursor_color(level_instrument_t * level_instrument,lv_color_t color)
{
    lv_led_set_color(level_instrument->led,color);
}

//设置光标亮度
void level_instrument_set_cursor_brightness(level_instrument_t * level_instrument,uint8_t bright)
{
    lv_led_set_brightness(level_instrument->led,bright);
}

//设置控件整体的对齐
void level_instrument_set_align(level_instrument_t * level_instrument,lv_align_t align,lv_coord_t x_offset,lv_coord_t y_offset)
{
    lv_obj_align(level_instrument->panel->red_bg,align,x_offset,y_offset);
    lv_obj_align(level_instrument->panel->green_bg,align,x_offset,y_offset);
    lv_obj_align(level_instrument->panel->yellow_bg,align,x_offset,y_offset);
    lv_obj_align(level_instrument->panel->horizontal_line,align,x_offset,y_offset);
    lv_obj_align(level_instrument->panel->vertical_line,align,x_offset,y_offset);
    lv_obj_align(level_instrument->led,align,x_offset+level_instrument->x,y_offset+level_instrument->y);
}

//设置面板透明度
void level_instrument_set_opa(level_instrument_t * level_instrument,lv_opa_t opa)
{
    lv_obj_set_style_opa(level_instrument->panel->red_bg,opa,0);
    lv_obj_set_style_opa(level_instrument->panel->green_bg,opa,0);
    lv_obj_set_style_opa(level_instrument->panel->yellow_bg,opa,0);
    lv_obj_set_style_opa(level_instrument->panel->horizontal_line,opa,0);
    lv_obj_set_style_opa(level_instrument->panel->vertical_line,opa,0);
}

/********************************************************************************************************/
extern osiThread_t *uart1_thread;

static void btn_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        osiEvent_t event;
		event.id = 0x3203;
		osiEventSend(uart1_thread, &event);
    }   
}

static void btn2_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        osiEvent_t event;
		event.id = 0x3299;
		osiEventSend(uart1_thread, &event);
    }   
}

void ui_screen4_init(void)
{
    // level_instrument = level_instrument_create(ui_Screen4);
    // level_instrument_set_size(level_instrument,270,LEVEL_INSTUMENT_PANEL_ALL);

    lv_obj_t * btn = lv_btn_create(ui_Screen4);
    lv_obj_align(btn,LV_ALIGN_CENTER,-80,0);
    lv_obj_set_size(btn,100,50);
    lv_obj_add_event_cb(btn,btn_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label,"flash");

    lv_obj_t * btn2 = lv_btn_create(ui_Screen4);
    lv_obj_align(btn2,LV_ALIGN_CENTER,80,0);
    lv_obj_set_size(btn2,100,50);
    lv_obj_add_event_cb(btn2,btn2_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t * label2 = lv_label_create(btn2);
    lv_label_set_text(label2,"download");
}