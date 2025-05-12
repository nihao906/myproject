#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_log.h"
#include "../inc/ui/ui_screen3.h"
#include "../inc/ui/fs_function.h"

#define     LOG_INFO(msg, ...)          QL_LOG(QL_LOG_LEVEL_INFO, "fsfunction", msg, ##__VA_ARGS__)
#define 	LOG	 1
struct Return_Data global_file_data_p;
int end_count = 0;
int current_id = 0;

/**
 * func:根据日期查询是否存在对应的数据文件
*/
int8_t findDataFile(uint16_t year,uint8_t month,uint8_t day)
{
    /*查询目录路径*/
    char dir_path[32];
    sprintf(dir_path,"SD:%d/%02d",year,month);
#if LOG
	LOG_INFO("dir_path = %s",dir_path);
#endif
    QDIR *dir = ql_opendir(dir_path);
    if(dir == NULL){
#if LOG
		LOG_INFO("ql_opendir fail");
#endif
        return -1;
    }
    ql_closedir(dir);

    /*查询文件路径*/
    char file_path[32];
    sprintf(file_path,"SD:%d/%02d/%02d.txt",year,month,day);
#if LOG
	LOG_INFO("file_path = %s",file_path);
#endif
    QFILE file_fd = ql_fopen(file_path,"rb");
    if(file_fd <= 0){
#if LOG
		LOG_INFO("ql_fopen fail");
#endif
        return -2;
    }

    /*查询文件是否为空*/
	if(ql_fseek(file_fd,0,QL_SEEK_END) <= 0){
#if LOG
		LOG_INFO("ql_fseek fail");
#endif
		ql_fclose(file_fd);
        return -3;
	}
    /*存在该目录下的非空文件*/
    ql_fclose(file_fd);
    return 1;
}

/**
 * 解析单独的一条数据
*/
void dealdata(char*buffer,int last_file,int last_i,struct Return_Data *newStruct)
{

	int id = 0 ;
	time_t utc_time = 0;
	struct tm *tm_now;
	float max_depth = 0;
	float max_V = 0;
	float average_I = 0;

	sscanf(buffer + last_i,"%d,%lld,%f,%f,%f",&id,&utc_time,&max_depth,&max_V,&average_I);
#if LOG
	LOG_INFO("id = %d,utc_time = %lld,max_depth = %f,max_V = %f,average_I = %f",id,utc_time,max_depth,max_V,average_I);
#endif
	tm_now = gmtime(&utc_time);

	//处理数据
	if(current_id != id){
		current_id = id;
		//LV_LOG_USER("新的id\n");

	    newStruct->dataArr[end_count].count ++;
		newStruct->dataArr[end_count].id = id;
		newStruct->dataArr[end_count].max_depth = max_depth;
		newStruct->dataArr[end_count].max_V = max_V;
		newStruct->dataArr[end_count].average_I = average_I;

		newStruct->dataArr[end_count].start_hour = tm_now ->tm_hour;
		newStruct->dataArr[end_count].start_min = tm_now ->tm_min;
		newStruct->dataArr[end_count].start_second = tm_now ->tm_sec;
		newStruct->dataArr[end_count].end_hour= tm_now ->tm_hour;
		newStruct->dataArr[end_count].end_min = tm_now ->tm_min;
		newStruct->dataArr[end_count].end_second = tm_now ->tm_sec;
 	    newStruct->dataArr[end_count].offset = last_file;
 	    //LV_LOG_USER("当前id在文件中的指针偏移量为：%d\n",newStruct->dataArr[end_count].offset);

		newStruct->id_count = end_count+1;
		end_count++;
	}else if(current_id == id){
		//LV_LOG_USER("相同id\n");

		newStruct->dataArr[end_count-1].max_depth = (max_depth > newStruct->dataArr[end_count-1].max_depth) ? max_depth : newStruct->dataArr[end_count-1].max_depth;
        newStruct->dataArr[end_count-1].max_V = (max_V > newStruct->dataArr[end_count-1].max_V) ? max_V : newStruct->dataArr[end_count-1].max_V;
        newStruct->dataArr[end_count-1].average_I = (newStruct->dataArr[end_count-1].average_I * newStruct->dataArr[end_count-1].count + average_I)/(newStruct->dataArr[end_count-1].count +1);
        newStruct->dataArr[end_count-1].count += 1;
        if(newStruct->dataArr[end_count-1].end_hour != tm_now ->tm_hour) newStruct->dataArr[end_count-1].end_hour = tm_now ->tm_hour;
        if(newStruct->dataArr[end_count-1].end_min != tm_now ->tm_min) newStruct->dataArr[end_count-1].end_min = tm_now ->tm_min;
        if(newStruct->dataArr[end_count-1].end_second != tm_now ->tm_sec) newStruct->dataArr[end_count-1].end_second = tm_now ->tm_sec;
	}

}

/**
 * 解析文件内容，将数据存储在指针指向的结构体变量中
*/
void readDataFromFile(QFILE fd, struct Return_Data * data)
{
    char buffer[CHAR_READ_LENGTH];
    int last_file = 0;
    int char_length = 0;
    int last_i = 0;

    ql_fseek(fd,0,QL_SEEK_END);
    int end_pos = ql_ftell(fd);
    ql_fseek(fd,0,QL_SEEK_SET);

    while(1){
        ql_fread(buffer + char_length, CHAR_READ_LENGTH - char_length, 1, fd);//int br = 0; lv_fs_read(file,buffer+char_length,CHAR_READ_LENGTH-char_length,&br);
#if LOG
		LOG_INFO("buffer: %s",buffer);
#endif
        last_i = 0;

        for(int i = 0; i < CHAR_READ_LENGTH; i++){
            if(buffer[i]=='\n'){
                dealdata(buffer,last_file,last_i,data);
                last_file = last_file + i - last_i + 2;//当前数据在文件中的偏移量=本身+上一行数据的数据长度
	    		last_i = i + 1;//下一次读取的起始位置
            }
        }

        if(buffer[CHAR_READ_LENGTH-1] != '\n'){
            char_length = CHAR_READ_LENGTH-last_i;
            int pos = ql_ftell(fd);// int pos = 0; lv_fs_tell(file,&pos);
            last_file = pos - char_length;
            strncpy(buffer,buffer + last_i, CHAR_READ_LENGTH - last_i);
        }

        // if (feof(file->file_d)) { // 如果已经读完文件，则退出循环
        // 	break;
        // }
        if(ql_ftell(fd) == end_pos){
            return;
        }
    }
}

/**
 * 传入数据结构体，把数据设置到8个table上
*/
void setDataToTables(struct Return_Data* dataArray, int page, int page_count)
{
    char max_V[10];
    char max_depth[10];
    char average_I[10];
	int end_pos = 0;
    lv_obj_t * data_tables_in_func[] = {data_table1, data_table2, data_table3, data_table4,data_table5, data_table6, data_table7, data_table8 };

	if(page<page_count){
		end_pos = 8;
	}else if(page == page_count){
		end_pos = dataArray->id_count - 8*(page-1);
		//清空8个table
		for(int m=0;m<8;m++){
			for(int n=0;n<6;n++){
				lv_table_set_cell_value_fmt(data_tables_in_func[m],0,n,"");
			}
		}
	}

	for(int i = 0; i < end_pos; i++){
		sprintf(max_V,"%.3f",dataArray->dataArr[(page-1)*8+i].max_V);
	    sprintf(max_depth,"%.3f",dataArray->dataArr[(page-1)*8+i].max_depth);
	    sprintf(average_I,"%.2f",dataArray->dataArr[(page-1)*8+i].average_I);

	    lv_table_set_cell_value_fmt(data_tables_in_func[i],0,0,"%d",dataArray->dataArr[(page-1)*8+i].id);
	    lv_table_set_cell_value_fmt(data_tables_in_func[i],0,1,"%s",max_V);
	    lv_table_set_cell_value_fmt(data_tables_in_func[i],0,2,"%s",max_depth);
	    lv_table_set_cell_value_fmt(data_tables_in_func[i],0,3,"%s",average_I);
	    lv_table_set_cell_value_fmt(data_tables_in_func[i],0,4,"%d:%d:%d",
	                                dataArray->dataArr[(page-1)*8+i].start_hour,
	                                dataArray->dataArr[(page-1)*8+i].start_min,
	                                dataArray->dataArr[(page-1)*8+i].start_second);
	    lv_table_set_cell_value_fmt(data_tables_in_func[i],0,5,"%d:%d:%d",
	                                dataArray->dataArr[(page-1)*8+i].end_hour,
	                                dataArray->dataArr[(page-1)*8+i].end_min,
	                                dataArray->dataArr[(page-1)*8+i].end_second);
	}
}

/**
 * 读取数据设置到chart上
*/
void setDataToChart(QFILE fd, int offset ,int current_id,struct ChartData* chart_data)
{

	char buffer[CHAR_READ_LENGTH];//存储每次读到的字符串
	int char_length = 0;//记录不完整字符串的长度
	int same_id_count = 0;

	/*获取长度*/
	ql_fseek(fd,0,QL_SEEK_END);
	int end_pos = ql_ftell(fd);

    ql_fseek(fd,offset,LV_FS_SEEK_SET);//lv_fs_seek(file,offset,LV_FS_SEEK_SET);
    int last_i = 0;
    while(1){
        last_i = 0;
        ql_fread(buffer + char_length, CHAR_READ_LENGTH - char_length, 1, fd);//lv_fs_read(file,buffer+char_length,CHAR_READ_LENGTH-char_length,NULL);
		//buffer[CHAR_READ_LENGTH] = '\0';

        for(int i = 0; i < CHAR_READ_LENGTH; i++){
            if(buffer[i]=='\n'){
                int id = 0 ;
				time_t utc_time = 0;
				struct tm *tm_now;
				float max_depth = 0;
				float max_V = 0;
				float average_I = 0;
				sscanf(buffer + last_i,"%d,%lld,%f,%f,%f",&id,&utc_time,&max_depth,&max_V,&average_I);
				tm_now = gmtime(&utc_time);

                if(current_id == id){
                    if(same_id_count < SAME_ID_MAX_NUM){

						chart_data->depth[same_id_count] = max_depth;
						chart_data->V[same_id_count] = max_V;
						chart_data->I[same_id_count] = average_I;
						sprintf(chart_data->time[same_id_count], "%d:%d:%d", tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
						chart_data->same_id_count = same_id_count;
						same_id_count++;
					}
                    last_i = i + 1;
                }else{
                    return;
                }
            }
        }

		if(buffer[CHAR_READ_LENGTH-1] != '\n'){
    		char_length = CHAR_READ_LENGTH-last_i;
			strncpy(buffer,buffer+last_i,CHAR_READ_LENGTH-last_i);
		}

		if(ql_ftell(fd) >= end_pos){
			return;
		}
    }

	return;
}