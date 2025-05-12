#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../inc/ui/ui_screen3.h"
#include "../inc/ui/ui_screen3_chart.h"
#include "../inc/ui/fs_function.h"

lv_obj_t * ui_Screen3;//用来切屏
lv_obj_t * screen3_calender;
lv_obj_t * screen3_calender_dsc_label; //用来设置选中日期
lv_obj_t * btn_page_plus; //用来设置是否可选中
lv_obj_t * btn_page_reduce;//用来设置是否可选中

//用来设置数据
lv_obj_t * data_table1;
lv_obj_t * data_table2;
lv_obj_t * data_table3;
lv_obj_t * data_table4;
lv_obj_t * data_table5;
lv_obj_t * data_table6;
lv_obj_t * data_table7;
lv_obj_t * data_table8;

lv_obj_t * screen3_spinbox;
lv_obj_t * screen3_spinbox_dsc_label;
lv_obj_t * screen3_spinbox_label_textarea;
lv_obj_t * btnm;
lv_obj_t * scrren3_spinbox_btn;

lv_obj_t * calendar_dropdown_year;
lv_obj_t * calendar_dropdown_month;

//
lv_obj_t * chart;
lv_obj_t * chart_screen;
lv_chart_series_t * ser1;
lv_chart_series_t * ser2;
lv_chart_series_t * ser3;
lv_chart_series_t * ser4;

lv_obj_t * primary_y_label;
lv_obj_t * secondry_y_label;
lv_obj_t * chart_change_btn_label;

extern lv_obj_t * ui_Screen2;
extern lv_obj_t * ui_Screen4;
extern int current_id;
extern int end_count;
struct Return_Data global_file_data_p;
struct ChartData chartdata;//存储chart数据
uint8_t this_date_have_data = 0; //判断是否需要重新读取文件
uint16_t screen3_spinbox_btnm_teatarea_flag = 0;
uint8_t user_first_see_calendar_or_change_date = 1;
lv_calendar_date_t highlighted_days[31];
uint8_t highlighted_days_cnt = 0;

void ui_Screen3_calendar_set_highlight(void)
{
    //通过日期获取路径
    char year[10];
    char month[10];
    char filedir[20];
    lv_dropdown_get_selected_str(calendar_dropdown_year, year, sizeof(year));
    lv_dropdown_get_selected_str(calendar_dropdown_month, month, sizeof(month));
    sprintf(filedir,"SD:/%s/%s",year,month);
    //printf("%s\n",filedir);

    uint16_t a = atoi(year);
    int8_t b = atoi(month);
    //printf("%d.%02d\n",a,b);

    //lv_calendar_date_t highlighted_days[30];
    for(int i = 0; i < 31; i++){
        highlighted_days[i].year = a;
        highlighted_days[i].month = b;
        highlighted_days[i].day = 0;
    }
    //printf("%d.%02d\n",highlighted_days[0].year,highlighted_days[0].month);

    char filenames[30][7];
    for (int i = 0; i < 31; i++) {
        sprintf(filenames[i], "%02d.txt", i+1);
    }

    //打开文件读取目录
    QDIR * dir = ql_opendir(filedir);
    if(dir != NULL){
        highlighted_days_cnt = 0;
        while(1){

            qdirent *dirinfo = ql_readdir(dir);
            if(dirinfo == NULL){
                break;
            }

            for( int i = 0; i < 31; i++){
                if(strcmp(dirinfo->d_name, filenames[i]) == 0){
                    highlighted_days[highlighted_days_cnt].day = atoi(filenames[i]);
                    highlighted_days_cnt++;
                    break;
                }
            }
        }
        lv_calendar_set_highlighted_dates(screen3_calender, highlighted_days, highlighted_days_cnt);//高亮对应的日期
        ql_closedir(dir);
    }
    // else{
    //     LV_LOG_USER("no date need highlight\n");
    // }
}

//点击使spinbox数值+1
void lv_spinbox_increment_event_cb(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_SHORT_CLICKED || code  == LV_EVENT_LONG_PRESSED_REPEAT) {
        //如果可以点击进入回调函数，代表一定有数据
        LV_LOG_USER("increment the data");
        lv_spinbox_increment(screen3_spinbox);

        //根据增加后的页数设置table数据：
        uint8_t current_page = (uint8_t)lv_spinbox_get_value(screen3_spinbox);
        if(current_page <= 0){
            return;
        }
        uint8_t page_count = global_file_data_p.id_count/8 + 1;
        LV_LOG_USER("page:%d page_count:%d\n",current_page,page_count);
        setDataToTables(&global_file_data_p,current_page,page_count);
    }
}

//点击使spinbox数值-1
void lv_spinbox_decrement_event_cb(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        //LV_LOG_USER("decrement the data");
        lv_spinbox_decrement(screen3_spinbox);

        //根据增加后的页数设置table数据：
        uint8_t current_page = (uint8_t)lv_spinbox_get_value(screen3_spinbox);
        if(current_page <= 0){
            return;
        }
        uint8_t page_count = global_file_data_p.id_count/8 + 1;
        //LV_LOG_USER("page:%d page_count:%d\n",current_page,page_count);
        setDataToTables(&global_file_data_p,current_page,page_count);
    }
}

//使用keyborad对textarea输入数字，按下回车按钮后隐藏textarea和keyboard并将数字设置到spinbox上
void screen3_textarea_keyboard_event_cb(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {

        //当能进入这个回调，说明spinbox btn 可以点击，说明存在有用的数据
        //获取被点击的btn的id
        int id = lv_btnmatrix_get_selected_btn(target);
        // const char * txt = lv_btnmatrix_get_btn_text(target, id);
        // LV_LOG_USER("%s was pressed\n", txt);

        uint8_t textarea_page_count = global_file_data_p.id_count/8 + 1;
        uint8_t textarea_page = 0;

        //回车被点击
        if(id == 11){

            //隐藏textarea和keyboard
            lv_obj_add_flag(screen3_spinbox_label_textarea,LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(btnm,LV_OBJ_FLAG_HIDDEN);
            screen3_spinbox_btnm_teatarea_flag = 0;

            //获取此时textarea的文本值
            const char * textarea_value = lv_textarea_get_text(screen3_spinbox_label_textarea);

            //当存在文本时
            if (textarea_value[0] != '\0'){
                uint8_t value = atoi(textarea_value);
                if(value > 0 && value <textarea_page_count){
                    //代表键入的页数在范围内
                    textarea_page = value;
                }else if(value == 0){
                    textarea_page = 1;
                }else{
                    textarea_page = textarea_page_count;
                }

                lv_spinbox_set_value(screen3_spinbox,textarea_page);//将spinbox设置为对应的页数
                setDataToTables(&global_file_data_p,textarea_page,textarea_page_count);
            }
        }
    }
}

//点击spinbox右侧显示页数的btn 显示/隐藏手动输入页数界面
void lv_spinbox_textarea_event_cb(lv_event_t * e){
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED) {
        LV_LOG_USER("page label is clicked");
        if(screen3_spinbox_btnm_teatarea_flag == 0){

            lv_obj_clear_flag(screen3_spinbox_label_textarea,LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(btnm,LV_OBJ_FLAG_HIDDEN);
            LV_LOG_USER("can see spinbox_textarea and btnm");
            screen3_spinbox_btnm_teatarea_flag = 1;
        }else if(screen3_spinbox_btnm_teatarea_flag == 1){

            lv_obj_add_flag(screen3_spinbox_label_textarea,LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(btnm,LV_OBJ_FLAG_HIDDEN);
            LV_LOG_USER("can not see spinbox_textarea and btnm");
            screen3_spinbox_btnm_teatarea_flag = 0;
        }
    }
}

//当keyboard中键入数值时，同步更新到textarea中
void screen3_btnm_event_handler(lv_event_t * e){
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * ta = lv_event_get_user_data(e);
    const char * txt = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));

    if(strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) lv_textarea_del_char(ta);
    else if(strcmp(txt, LV_SYMBOL_NEW_LINE) == 0) lv_event_send(ta, LV_EVENT_READY, NULL);
    else lv_textarea_add_text(ta, txt);
}

//点击显示日期的btn可以弹出/隐藏日历界面，当弹出日历时，高亮对应日期
uint16_t calendar_flag = 0;
void screen3_calender_create_cb(lv_event_t *e){
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED) {
        LV_LOG_USER("calender create btn clicked");
        if(calendar_flag == 0){
            LV_LOG_USER("can see calender");
            lv_obj_clear_flag(screen3_calender,LV_OBJ_FLAG_HIDDEN);

            //当用户第一次显示日历或者切换日期时
            if(user_first_see_calendar_or_change_date){
                LV_LOG_USER("user fisrt see calendar or change date\n");
                user_first_see_calendar_or_change_date = 0;
                ui_Screen3_calendar_set_highlight();
            }else{
                lv_calendar_set_highlighted_dates(screen3_calender, highlighted_days, highlighted_days_cnt);
            }
            calendar_flag = 1;
        }else if(calendar_flag == 1){
            LV_LOG_USER("can not see calender");
            lv_obj_add_flag(screen3_calender,LV_OBJ_FLAG_HIDDEN);
            calendar_flag = 0;
        }
    }
}

void screen3_calender_dropdown_cb(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    //lv_obj_t * obj = lv_event_get_target(e); //检测是哪个控件触发的事件

    if(code == LV_EVENT_VALUE_CHANGED){
//#if EXIST_RECORD_DATA_FILE
        ui_Screen3_calendar_set_highlight();
//#endif
    }
}

//选择日历的日期，将日历设置为选择的日期，并在显示日期的btn上同步更新
void screen3_calender_event_cb(lv_event_t *e){

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * tables[8] = {data_table1,data_table2,data_table3,data_table4,data_table5,data_table6,data_table7,data_table8};

    //点击相同日期也会触发
    if(code == LV_EVENT_VALUE_CHANGED) {

        //隐藏日历
        //LV_LOG_USER("calendar date value changed,hide calendar\n");
        lv_obj_add_flag(screen3_calender,LV_OBJ_FLAG_HIDDEN);
        calendar_flag = 0;

        //获取点击的日期
        lv_calendar_date_t clicked_date;
        lv_calendar_get_pressed_date(screen3_calender, &clicked_date);
        lv_label_set_text_fmt(screen3_calender_dsc_label,"%d.%02d.%02d",clicked_date.year,clicked_date.month,clicked_date.day);
        //LV_LOG_USER("year:%d,month:%d,day:%d\n",clicked_date.year,clicked_date.month,clicked_date.day);

        //遍历高亮日期数组，查询是否存在相同的项,如果存在相同的，则代表点击的日期有效,否则代表无该文件
        uint8_t highlight_flag = 0;
        for(int i = 0; i < 31; i++){
            if( clicked_date.year == highlighted_days[i].year &&
                clicked_date.month == highlighted_days[i].month &&
                clicked_date.day == highlighted_days[i].day)
            {
                highlight_flag = 1;
                break;
            }
        }

        uint8_t different_date_flag = 1;
        char * label_date = lv_label_get_text(screen3_calender_dsc_label);
        int year,month,day;
        sscanf(label_date,"%d.%d.%d",&year,&month,&day);
        
        //LV_LOG_USER("label_date:%s,year:%d,month:%d,day:%d\n",label_date,year,month,day);
        if(year == clicked_date.year && month == clicked_date.month && day == clicked_date.day) different_date_flag = 0;

        //当既不是相同日期又存在文件时，需要从新给全局遍历赋值
        if(highlight_flag && different_date_flag){
            //LV_LOG_USER("different date and file exist\n");

            //将结构体变量初始化为0
            lv_memset_00(&global_file_data_p,sizeof(struct Return_Data));

            //判断文件是否可读
            this_date_have_data = findDataFile(clicked_date.year,clicked_date.month,clicked_date.day);
            if(this_date_have_data)
            {
                //LV_LOG_USER("unempty file\n");
                //生成文件路径
                char file_path[32];
                sprintf(file_path,"E:/%d/%02d/%02d.txt",(int)clicked_date.year,(int)clicked_date.month,(int)clicked_date.day);
                //LV_LOG_USER("file_path:%s,id num = %d\n",file_path,global_file_data_p.id_count);

                //打开文件
                // lv_fs_file_t file_in_screen3_calender_event_cb = {
                //     .drv = NULL,
                //     .file_d = NULL
                // };
                QFILE fd = ql_fopen(file_path,"rb");// lv_fs_open(&file_in_screen3_calender_event_cb,file_path,LV_FS_MODE_RD);

                //读取文件赋值给变量
                readDataFromFile(fd,&global_file_data_p);

                ql_fclose(fd);//lv_fs_close(&file_in_screen3_calender_event_cb);
                current_id =0;
                end_count = 0;

                //根据变量处理控件
                //计算总页数，设置spinbox页数label以及spinbox范围
                uint8_t page_count = global_file_data_p.id_count/8 + 1;
                lv_label_set_text_fmt(screen3_spinbox_dsc_label,"/ %d",page_count);
                lv_spinbox_set_range(screen3_spinbox, 1,page_count);
                //LV_LOG_USER("different id num is: %d, page_count = %d\n", global_file_data_p.id_count,page_count);

                //根据当前页数和总页数，设置tabel的数据
                uint8_t current_page = lv_spinbox_get_value(screen3_spinbox);
                setDataToTables(&global_file_data_p,current_page,page_count);

                //使能点击flag
                lv_obj_add_flag(btn_page_plus,LV_OBJ_FLAG_CLICKABLE);
                lv_obj_add_flag(btn_page_reduce,LV_OBJ_FLAG_CLICKABLE);
                lv_obj_add_flag(scrren3_spinbox_btn,LV_OBJ_FLAG_CLICKABLE);

                lv_obj_t * tables[8] = {data_table1,data_table2,data_table3,data_table4,data_table5,data_table6,data_table7,data_table8};
                for(int i = 0; i < 8; i++){
                    lv_obj_add_flag(tables[i],LV_OBJ_FLAG_CLICKABLE);
                }
                return;

            }
            //如果文件空
            else
            {
                //LV_LOG_USER("empty file\n");
                goto exit;
            }
        }
        else if(!different_date_flag)
        {
            //代表点击了相同的日期，直接返回
            //LV_LOG_USER("click same date\n");
            return;
        }
        else
        {
            //代表点了不同的日期但是日期对应没有文件
            //LV_LOG_USER("click different date but no file\n");
            goto exit;
        }

    }
exit:
    //清空table,清除table的可点击flag
    for(int i = 0; i < 8; i++){
        lv_obj_clear_flag(tables[i],LV_OBJ_FLAG_CLICKABLE);
        for(int j = 0; j < 6; j++){
            lv_table_set_cell_value_fmt(tables[i],0,j,"");
        }
    }
    //清除spinbox的btn的可点击flag
    lv_obj_clear_flag(btn_page_plus,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(btn_page_reduce,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(scrren3_spinbox_btn,LV_OBJ_FLAG_CLICKABLE);
    //设置spinbox的label
    lv_label_set_text(screen3_spinbox_dsc_label,"N/A");
    return;
}

//当点击data_table时，创建chart界面
void screen3_data_table_chart_create(lv_event_t * e){

    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(event_code == LV_EVENT_CLICKED){
        //LV_LOG_USER("get code and get target\n");
#if 1
        /*获取id，如果id为空则直接返回，不生成chart*/
        int id = atoi(lv_table_get_cell_value(target, 0, 0));
        if(id == 0){
            LV_LOG_USER("id is empty\n");
            return;
        }

        /*根据id获取文件指针位置和偏移量*/
        int offset = 0;
        for( int i = 0; i < global_file_data_p.id_count; i++){
            if(id == global_file_data_p.dataArr[i].id){
                offset = global_file_data_p.dataArr[i].offset;
                break;
            }
        }
        //LV_LOG_USER("id = %d, offset = %d\n",id,offset);

        /*获取该id的chart数据*/
        //生成文件路径并打开文件
        int year,month,day;
        char file_path[18];
        sscanf(lv_label_get_text(screen3_calender_dsc_label), "%d.%d.%d", &year,&month,&day);
        sprintf(file_path,"E:/%d/%02d/%02d.txt",year,month,day);
        //LV_LOG_USER("path:%s\n",file_path);

        // lv_fs_file_t file_in_screen3_data_table_chart_create = {
        //     .drv = NULL,
        //     .file_d = NULL
        // };
        // lv_fs_open(&file_in_screen3_data_table_chart_create,file_path,LV_FS_MODE_RD);
        QFILE fd = ql_fopen(file_path,"rb");

        //读取该id的chart数据
        lv_memset_00(&chartdata,sizeof(struct ChartData));
        setDataToChart(fd,offset,id,&chartdata);

        ql_fclose(fd);//lv_fs_close(&file_in_screen3_data_table_chart_create);
#endif
#if 0
        printf("same_id_count = %d\n",chartdata.same_id_count+1);
        for(int i = 0; i < chartdata.same_id_count+1; i++){
            printf("depth:%.3f\t,V:%.3f,I:%.3f,time:%s\n",
                chartdata.depth[i],
                chartdata.V[i],
                chartdata.I[i],
                chartdata.time[i]
            );
        }
#endif
        int num = chartdata.same_id_count+1;
        if(num <= 1){
            LV_LOG_USER("too few points\n");
            return;
        }
        ui_screen3_chart_init(num);
    }
}
/****************************************************************************************************************************************/
void ui_screen3_init(void)
{
    /*****************************************************显示日期的label**********************************************************/
    lv_obj_t * screen3_calender_btn = lv_btn_create(ui_Screen3);
    lv_obj_set_size(screen3_calender_btn, 100, 34);
    lv_obj_set_pos(screen3_calender_btn,60,0);
    //点击btn显示或隐藏calendar,并高亮有文件的日期
    lv_obj_add_event_cb(screen3_calender_btn, screen3_calender_create_cb, LV_EVENT_CLICKED, NULL);

    screen3_calender_dsc_label = lv_label_create(screen3_calender_btn);
    lv_obj_center(screen3_calender_dsc_label);
/*
    // 获取当前时间戳
    time_t t = time(NULL);

    // 使用localtime函数将时间戳转换为本地时间结构
    struct tm *local_time = localtime(&t);

    // 从本地时间结构中提取年、月和日
    int year = local_time->tm_year + 1900;   // 年份需要加上1900
    int month = local_time->tm_mon + 1;      // 月份从0开始，需要加1
    int day = local_time->tm_mday;
    char calendar[11];
    sprintf(calendar,"%d.%d.%d",year,month,day);
*/
    lv_label_set_text(screen3_calender_dsc_label,"2024.01.10");

    /*****************************************************显示页数的spinbox**********************************************************/
    //创建承载spinbox和描述label的btn
    scrren3_spinbox_btn = lv_btn_create(ui_Screen3);
    lv_obj_set_pos(scrren3_spinbox_btn,330,0);
    lv_obj_set_size(scrren3_spinbox_btn,80,30);
    lv_obj_set_style_bg_color(scrren3_spinbox_btn,lv_color_hex(0xFFFFFFF),0);
    //点击显示输入文本框和键盘
    lv_obj_add_event_cb(scrren3_spinbox_btn, lv_spinbox_textarea_event_cb, LV_EVENT_CLICKED, NULL);

    //创建spinbox
    screen3_spinbox = lv_spinbox_create(scrren3_spinbox_btn);
    lv_spinbox_set_digit_format(screen3_spinbox, 2, 2);
    lv_obj_set_width(screen3_spinbox, 46);
    lv_obj_align(screen3_spinbox,LV_ALIGN_CENTER,-15,0);
    lv_spinbox_set_value(screen3_spinbox,1);//设置初始为第一页
    lv_obj_set_style_border_opa(screen3_spinbox,0,0);
    lv_obj_set_style_outline_opa(screen3_spinbox,0,0);

    //创建显示总页数的label
    screen3_spinbox_dsc_label = lv_label_create(scrren3_spinbox_btn);
    lv_obj_set_size(screen3_spinbox_dsc_label,50,34);
    lv_obj_align_to(screen3_spinbox_dsc_label,screen3_spinbox,LV_ALIGN_OUT_RIGHT_MID,0,8);
    lv_obj_set_style_text_font(screen3_spinbox_dsc_label,&lv_font_montserrat_16,0);
    lv_label_set_text(screen3_spinbox_dsc_label,"/100");
    lv_obj_set_style_border_opa(screen3_spinbox_dsc_label,0,0);
    lv_obj_set_style_outline_opa(screen3_spinbox_dsc_label,0,0);

    /*****************************************************控制翻页的button**********************************************************/
    btn_page_plus = lv_btn_create(ui_Screen3);
    lv_obj_set_size(btn_page_plus, 30, 30);
    lv_obj_set_pos(btn_page_plus,410,0);
    lv_obj_set_style_bg_img_src(btn_page_plus, LV_SYMBOL_PLUS, 0);

    btn_page_reduce = lv_btn_create(ui_Screen3);
    lv_obj_set_size(btn_page_reduce, 30, 30);
    lv_obj_set_pos(btn_page_reduce,300,0);
    lv_obj_set_style_bg_img_src(btn_page_reduce, LV_SYMBOL_MINUS, 0);

    //spinbox页数+1
    lv_obj_add_event_cb(btn_page_plus, lv_spinbox_increment_event_cb, LV_EVENT_ALL,NULL);
    //spinbox页数-1
    lv_obj_add_event_cb(btn_page_reduce, lv_spinbox_decrement_event_cb, LV_EVENT_ALL,NULL);

    /*****************************************************记录数据的table**********************************************************/
    char text[6][16] = {"桩点号","最大喷浆量","最大深度","平均电流","开始时间","结束时间"};
    int align[6] = {-210,-125,-40,35,120,210};
    int size[6] = {50,80,70,70,70,70};
    for(int i = 0; i < 6; i++){
        lv_obj_t * screen3_label = lv_label_create(ui_Screen3);
        lv_obj_set_size(screen3_label,size[i],30);
        lv_obj_align(screen3_label,LV_ALIGN_TOP_MID, align[i], 45);
        lv_obj_clear_flag(screen3_label,LV_OBJ_FLAG_SCROLLABLE);
        lv_label_set_text(screen3_label,text[i]);
        lv_obj_set_style_text_font(screen3_label, &lv_font_chinese_12_name, 0);
    }

    data_table1 = lv_table_create(ui_Screen3);
    data_table2 = lv_table_create(ui_Screen3);
    data_table3 = lv_table_create(ui_Screen3);
    data_table4 = lv_table_create(ui_Screen3);
    data_table5 = lv_table_create(ui_Screen3);
    data_table6 = lv_table_create(ui_Screen3);
    data_table7 = lv_table_create(ui_Screen3);
    data_table8 = lv_table_create(ui_Screen3);

    lv_obj_t*screen3_data_tables[] = {data_table1, data_table2, data_table3, data_table4,data_table5, data_table6, data_table7, data_table8};
    int align_data[] = {-224,-192,-160,-128,-96,-64,-32,0};
    for(int r=0;r<8;r++){
        lv_obj_align(screen3_data_tables[r],LV_ALIGN_BOTTOM_MID, 10, align_data[r]);//设置位置
        lv_table_set_row_cnt(screen3_data_tables[r],1);//设置行列数
        lv_table_set_col_cnt(screen3_data_tables[r],6);
        lv_obj_set_height(screen3_data_tables[r],35);
        lv_obj_set_style_border_width(screen3_data_tables[r],1,LV_PART_MAIN);
        lv_obj_set_style_pad_top(screen3_data_tables[r],0,LV_PART_ITEMS);
        lv_obj_clear_flag(screen3_data_tables[r],LV_OBJ_FLAG_SCROLLABLE);
        lv_table_set_col_width(screen3_data_tables[r],0,75);
        lv_table_set_col_width(screen3_data_tables[r],1,87);
        lv_table_set_col_width(screen3_data_tables[r],2,75);
        lv_table_set_col_width(screen3_data_tables[r],3,80);
        lv_table_set_col_width(screen3_data_tables[r],4,100);
        lv_table_set_col_width(screen3_data_tables[r],5,100);
        //当点击table时，生成对应的chart
        lv_obj_add_event_cb(screen3_data_tables[r], screen3_data_table_chart_create, LV_EVENT_CLICKED,NULL);
    }

    /*****************************************************calendar**********************************************************/
    screen3_calender = lv_calendar_create(ui_Screen3);
    lv_obj_set_size(screen3_calender, 480, 287);
    lv_obj_align(screen3_calender,LV_ALIGN_BOTTOM_MID,0,0);
    lv_obj_add_flag(screen3_calender,LV_OBJ_FLAG_HIDDEN);
    //选择日期，读取对应日期文件，设置table，更新label
    lv_obj_add_event_cb(screen3_calender, screen3_calender_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * calendar_dropdown_background = lv_calendar_header_dropdown_create(screen3_calender);
    calendar_dropdown_year = lv_obj_get_child(calendar_dropdown_background,0);
    calendar_dropdown_month = lv_obj_get_child(calendar_dropdown_background,1);
    //当日历的年月改变时，读取相对应的路径并高亮对应日期
    lv_obj_add_event_cb(calendar_dropdown_year, screen3_calender_dropdown_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(calendar_dropdown_month, screen3_calender_dropdown_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    /*****************************************************textarea+keyboard**********************************************************/
    //创建输入文本框并隐藏
    screen3_spinbox_label_textarea = lv_textarea_create(ui_Screen3);
    lv_textarea_set_one_line(screen3_spinbox_label_textarea, true);
    lv_obj_align(screen3_spinbox_label_textarea, LV_ALIGN_TOP_MID, 0, 32);
    lv_obj_set_width(screen3_spinbox_label_textarea,480);
    lv_obj_add_state(screen3_spinbox_label_textarea, LV_STATE_FOCUSED);
    lv_obj_add_flag(screen3_spinbox_label_textarea,LV_OBJ_FLAG_HIDDEN);

    //为输入文本框创建输入键盘
    static const char * btnm_map[] = {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n",
        "7", "8", "9", "\n",
        LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""};

    btnm = lv_btnmatrix_create(ui_Screen3);
    lv_obj_set_size(btnm, 480, 250);
    lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_clear_flag(btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
    lv_btnmatrix_set_map(btnm, btnm_map);
    lv_obj_add_flag(btnm,LV_OBJ_FLAG_HIDDEN);

    //回车时设置label，隐藏ta和btnm
    lv_obj_add_event_cb(btnm, screen3_textarea_keyboard_event_cb, LV_EVENT_ALL, NULL);

    //将键盘输入的数据键入到textarea上
    lv_obj_add_event_cb(btnm, screen3_btnm_event_handler, LV_EVENT_VALUE_CHANGED, screen3_spinbox_label_textarea);
}