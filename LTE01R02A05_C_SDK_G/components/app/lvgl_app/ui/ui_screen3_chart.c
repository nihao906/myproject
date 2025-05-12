#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/ui/ui_screen3_chart.h"
#include "../inc/ui/fs_function.h"

extern struct ChartData chartdata;;
lv_obj_t * chart;
lv_chart_series_t * ser_depth;
lv_chart_series_t * ser_V;
lv_chart_series_t * ser_I;

void chart_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    //struct ChartUserData * chart_user_data = (struct ChartUserData *)lv_event_get_user_data(e);
    lv_obj_t * chart_screen = (lv_obj_t*)lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED){
        // LV_LOG_USER("delete chart btn clicked,user data p:%p,chart p:%p,depth p:%p,V p:%p\n",
        lv_chart_remove_series(chart,ser_depth);
        lv_chart_remove_series(chart,ser_V);
        lv_chart_remove_series(chart,ser_I);
        lv_obj_del(chart_screen);
        lv_obj_del(obj);

        ser_depth = NULL;
        ser_V = NULL;
        ser_I = NULL;
        chart = NULL;

        lv_memset_00(&chartdata,sizeof(struct ChartData));
    }
}

//在chart生成时，设置x坐标轴的单位
void chart_x_set(lv_event_t * e)
{
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
    if(dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X) {
        lv_snprintf(dsc->text, sizeof(dsc->text), "%s", chartdata.time[dsc->value]);
    }
}

//当在chart上点击数据点时，将该横坐标对应的纵坐标值分别在各个ser的数据点旁边显示
void chart_see_two_ser_values_after_clicked(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * chart = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_invalidate(chart);
    }
    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_coord_t * s = lv_event_get_param(e);
        *s = LV_MAX(*s, 20);
    }
    else if(code == LV_EVENT_DRAW_POST_END) {
        int id = lv_chart_get_pressed_point(chart);
        if(id == LV_CHART_POINT_NONE) return;

        LV_LOG_USER("Selected point %d", id);

        lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);
        while(ser) {
            lv_point_t p;
            lv_chart_get_point_pos_by_id(chart, ser, id, &p);

            lv_coord_t * y_array = lv_chart_get_y_array(chart, ser);
            lv_coord_t value = y_array[id];

            char buf[16];
            lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DUMMY"$%d", value);

            lv_draw_rect_dsc_t draw_rect_dsc;
            lv_draw_rect_dsc_init(&draw_rect_dsc);
            draw_rect_dsc.bg_color = lv_color_black();
            draw_rect_dsc.bg_opa = LV_OPA_50;
            draw_rect_dsc.radius = 3;
            draw_rect_dsc.bg_img_src = buf;
            draw_rect_dsc.bg_img_recolor = lv_color_white();

            lv_area_t a;
            a.x1 = chart->coords.x1 + p.x - 20;
            a.x2 = chart->coords.x1 + p.x + 20;
            a.y1 = chart->coords.y1 + p.y - 30;
            a.y2 = chart->coords.y1 + p.y - 10;

            const lv_area_t * clip_area = lv_event_get_clip_area(e);
            lv_draw_rect(&a, clip_area, &draw_rect_dsc);

            ser = lv_chart_get_series_next(chart, ser);
        }
    }
    else if(code == LV_EVENT_RELEASED) {
        lv_obj_invalidate(chart);
    }
}

void ui_screen3_chart_init(int num)
{
    /*基于当前活动屏幕创建chart所在的屏*/
    lv_obj_t * chart_screen = lv_obj_create(lv_scr_act());
    lv_obj_center(chart_screen);
    lv_obj_set_size(chart_screen,480,320);
    lv_obj_clear_flag(chart_screen,LV_OBJ_FLAG_SCROLLABLE);

    /*创建关闭btn*/
    lv_obj_t *chart_close_btn = lv_btn_create(lv_scr_act());
    lv_obj_center(chart_close_btn);
    lv_obj_set_size(chart_close_btn,100,40);
    lv_obj_align(chart_close_btn,LV_ALIGN_TOP_RIGHT,-40,0);
    lv_obj_t *chart_close_btn_label = lv_label_create(chart_close_btn);
    lv_label_set_text(chart_close_btn_label,LV_SYMBOL_CLOSE);
    lv_obj_center(chart_close_btn_label);
    lv_obj_set_style_text_font(chart_close_btn_label,&lv_font_montserrat_24,0);
    lv_obj_add_event_cb(chart_close_btn,chart_btn_event_handler,LV_EVENT_ALL,(void*)chart_screen);

    chart = lv_chart_create(chart_screen);
    lv_obj_set_size(chart, 420, 250);
    lv_obj_align(chart,LV_ALIGN_BOTTOM_MID,-5,-5);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   /*Show lines and points too*/
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 10);//左侧纵坐标范围，深度
    lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, 0, 1600);//右侧纵坐标范围，累计喷浆量
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 5, 1, 6, 1, true, 50);//给坐标轴添加刻度线
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_SECONDARY_Y, 5, 1, 6, 1, true, 50);
    lv_chart_set_zoom_x(chart, 400);//设置x轴缩进
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 5, 1, num, 1, true, 20); //设置横坐标刻度(1个点无法生成line)
    lv_chart_set_point_count(chart, num);
    ser_depth = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);//深度，深蓝色
    ser_V = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_SECONDARY_Y);//累计喷浆，绿色
    ser_I = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);//电流 红色

    /*根据chart数据生成line*/
    for(int i = 0; i < num; i++){
        lv_chart_set_next_value(chart,ser_depth,chartdata.depth[i]);
        lv_chart_set_next_value(chart,ser_V,chartdata.V[i]);
        lv_chart_set_next_value(chart,ser_I,chartdata.I[i]);
    }
    lv_chart_refresh(chart);

    //在绘图时重绘x轴坐标描述
    lv_obj_add_event_cb(chart, chart_x_set, LV_EVENT_DRAW_PART_BEGIN,NULL);

    //点击数据点时在旁边显示值
    lv_obj_add_event_cb(chart, chart_see_two_ser_values_after_clicked, LV_EVENT_ALL, NULL);
}