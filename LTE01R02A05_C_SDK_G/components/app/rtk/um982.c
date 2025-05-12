#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "ql_uart.h"
#include "ql_gpio.h"
#include "ql_api_osi.h"
#include "osi_api.h"

#include "minmea.h"

#include "../t2n/include/t2n.h"
#include "../t2n/include/ModbusS.h"
#include "../t2n/include/cfg.h"
#include "rtk_gps.h"
#include "udp_ota_shell.h"
#include "../ec600u_bl0939/ac_current.h"

#include "ql_log.h"

#pragma pack(1)
typedef struct
{
    uint32_t UTC;
    uint32_t LAT;
    uint8_t LAT_H;
    int32_t LNG;
    uint8_t LNG_H;
    uint16_t ALT;
    uint8_t ALT_H;
    uint8_t status;
    uint16_t speed;
    int16_t HDG;
    int16_t TP[4];
} cp_road_t;

#pragma pack()

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "rtk", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "rtk", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "rtk", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "rtk", msg, ##__VA_ARGS__)

extern uint16_t read_wiegand_io(void);

osiThread_t *rtk_thread = NULL;
// LSAPI_Device_t *rtk_uart = NULL;
// extern LSAPI_Device_t *GPS_REST;
const char *um982_config[] = {
    "version\r\n",
    "version\r\n",
    "MODE ROVER\r\n",
    "GPRMC COM1 1\r\n",
    "GPGGA COM1 1\r\n",
    "GPVTG COM1 1\r\n"
    "LOG HEADINGA ONTIME 1\r\n",
    "LOG BESTPOSA ONTIME 1\r\n",
    "GPGSV COM1 1\r\n",
    "GPGSA COM1 1\r\n",
    // "SAVECONFIG\r\n"
    };

extern void NmeaTcp_init(void);

// static void uart_event_cb(void *param, uint32_t evt)
// {
//     // log_printf(LDEBUG, LOG_TAG "uart2 event=%d", evt);
//     if (evt & LS_UART_EVENT_TX_COMPLETE)
//     {
//     }
//     if (evt & LS_UART_EVENT_RX_ARRIVED)
//     {
//         LSAPI_OSI_Event_t event;
//         event.id = 9; // 串口2-gps1接收事件id
//         if (rtk_thread != NULL)
//         {
//             LSAPI_OSI_EvnetSend(rtk_thread, &event);
//         }
//     }
// }
static int16_t speed_list[10] = {0};
static uint16_t speed_index = 0;
static uint16_t speed_count = 0;
static int16_t speed_avg(int16_t speed, int avg_cout)
{
    int speed_sum = 0;
    if (avg_cout <= 1)
    {
        return speed;
    }
    else if (avg_cout > 10)
    {
        avg_cout = 10;
    }
    speed_list[speed_index] = speed;
    int buf_size = sizeof(speed_list) / sizeof(speed_list[0]);
    if (avg_cout > buf_size)
    {
        avg_cout = buf_size;
    }
    if (++speed_index >= avg_cout)
    {
        speed_index = 0;
    }
    if (++speed_count >= avg_cout)
    {
        speed_count = avg_cout;
    }
    for (int i = 0; i < speed_count; i++)
    {
        speed_sum += speed_list[i];
    }
    return speed_sum / speed_count;
}

void config_timer_cb(void *param)
{
    if (rtk_thread != NULL)
    {
        osiEvent_t event;
        event.id = 12; // timer1事件id
        osiEventSend(rtk_thread, &event);
    }
}

static uint32_t date_time2utc(struct minmea_date *date, struct minmea_time *time)
{
    struct tm gps_time;
    uint32_t gps_utc;
    gps_time.tm_hour = time->hours;
    gps_time.tm_min = time->minutes;
    gps_time.tm_sec = time->seconds;
    gps_time.tm_year = date->year + 2000 - 1900;
    gps_time.tm_mon = date->month - 1;
    gps_time.tm_mday = date->day;
    gps_utc = mktime(&gps_time);
    return gps_utc;
}

extern int NmeaTcp_send(uint8_t gps_id, char input_line[]);
extern int drp_write(uint8_t endpoint, uint8_t flag, uint16_t tran_id, void *data, int length);
int8_t gps_gga_req = 0;        // 当前以gps1的数据为基础上传
int32_t gps_rtcm_age = 0;      // 移植
int32_t gps_rtcm_timeout = 10; // 移植
static uint16_t tran_id = 0;
/**
 * @brief nmea协议解析处理
 *
 * @param gps_parse
 * @param input_line 输入的一行数据
 * @return int
 */
static int parse_nmea_0183(gps_t *gps_parse, char input_line[])
{
    if (input_line == NULL)
    {
        return -1;
    }

    int id = minmea_sentence_id(input_line, false);
    LOGD("parse_nmea0183 = [%s],id=%d", input_line, id);
    switch (id)
    {
    case MINMEA_SENTENCE_RMC:
    {
        struct minmea_sentence_rmc frame;
        NmeaTcp_send(gps_parse->id, input_line);
        gWordVar[REG_VIEW_STAS] = gps_parse->total_gsv;
        gps_parse->total_gsv = 0;
        if (minmea_parse_rmc(&frame, input_line))
        {
            // log_printf(LDEBUG, LOG_TAG "$xxRMC: raw coordinates and speed: (%lld/%d,%lld/%d) %d/%d\n",
            //            frame.latitude.value, frame.latitude.scale, frame.longitude.value,
            //            frame.longitude.scale, frame.speed.value, frame.speed.scale);
            // log_printf(
            //     LDEBUG,
            //     LOG_TAG
            //     "$xxRMC fixed-point coordinates LAT:(%lld),LOG:(%lld)SPEED:(%d) %d\n",
            //     minmea_coord_scale(&frame.latitude, 1000000000), minmea_coord_scale(&frame.longitude, 1000000000),
            //     minmea_rescale(&frame.speed, 1000));
            // gWordVar[REG_SPEED] = frame.speed.value * 1852 / 1000;
            // log_printf(LDEBUG, LOG_TAG "$xxRMC floating point degree coordinates and speed: (%f,%f) %f\n",
            //            minmea_tocoord(&frame.latitude), minmea_tocoord(&frame.longitude),
            //            minmea_tofloat(&frame.speed));
            LOGD("$xxRMC gps date=%d-%02d-%02d time=%02d:%02d:%02d \n",
                 frame.date.year, frame.date.month, frame.date.day, frame.time.hours, frame.time.minutes, frame.time.seconds);
            gps_parse->UTC = date_time2utc(&frame.date, &frame.time);
            gps_parse->date = frame.date;
            gps_parse->time = frame.time;
            // gps_parse->speed = minmea_rescale(&frame.speed, 1852);
            //  gWordVar[REG_SPEED] = gps_parse->speed;
        }
        else
        {
            LOGD("$xxRMC sentence is not parsed\n");
        }
    }
    break;

    case MINMEA_SENTENCE_GGA:
    {
        struct minmea_sentence_gga frame;
        NmeaTcp_send(gps_parse->id, input_line);
        if (minmea_parse_gga(&frame, input_line))
        {
            // log_printf(LDEBUG, LOG_TAG "$xxGGA: fix quality: %d\n", frame.fix_quality);
            gWordVar[REG_GPS_STATUS] = frame.fix_quality;
            gWordVar[REG_USE_STAS] = frame.satellites_tracked;
            gps_parse->fix_quality = frame.fix_quality;
            gps_parse->LAT = minmea_coord_scale(&frame.latitude, 1000000000);
            gps_parse->LNG = minmea_coord_scale(&frame.longitude, 1000000000);
            gps_parse->ALT = minmea_rescale(&frame.altitude, 1000);
            gps_parse->Q = frame.fix_quality;
            LOGD("$xxGGA fixed-point Q:(%d),LAT:(%lld),LOG:(%lld)ALT:(%d)Speed:(%d)\n",
                 gps_parse->Q, gps_parse->LAT, gps_parse->LNG, gps_parse->ALT, gps_parse->speed);

            /* 移植Lua部分代码
              if string.find(line,"$GPGGA",1,7) == 1 and self.gps_data.lat ~= 0 and self.gps_data.lng ~= 0
              then --last gps out
              self.gps_timeout = 0
              self.rtcm_age = self.rtcm_age + 1
                -- p("self.rtcm_age",self.rtcm_age)
                if self.gga_req ~= 0 or self.rtcm_age >= self.rtcm_timeout then
                  p("rtcm_timeout",self.rtcm_age,line)
                  t2n:drp_write(10,0,line,#line)
                  -- p(self.rtcm_timeout)
                  self.rtcm_timeout = self.rtcm_timeout * 2
                  if self.rtcm_timeout > 240 then
                    self.rtcm_timeout = 240
                    self.rtcm_age = 120
                  end
                  self.gga_req = 0
                end

                -- p("data", self.gps_data)
                -- self:emit("data", self.gps_data)
                --p(string.format("lon=%.9f,lat=%.9f,alt=%.4f
              time=%d",lon_sum/k,lat_sum/k,alt_sum/(sum_cnt*1000),sum_cnt)) end
            */
            // int32_t lat = minmea_tocoord(&frame.latitude) * 10000000;
            // int32_t lng = minmea_tocoord(&frame.longitude) * 10000000;
            if (gps_parse->LAT != 0 && gps_parse->LNG != 0)
            { // 经纬度都不等于0的时候
               // cp_road_t cp_report;
                gps_rtcm_age++;
                gWordVar[RTCM_AGE] = gps_rtcm_age;
                LOGD("$GPGGA sentence gps_gga_req:%d, gps_rtcm_age:%d, gps_rtcm_timeout:%d, id:%d\n",
                     gps_gga_req, gps_rtcm_age, gps_rtcm_timeout, gps_parse->id);
                if (gps_gga_req != 0 || gps_rtcm_age >= gps_rtcm_timeout)
                { // t2n recv "GGA_REQ" gps1

                    int len = strlen(input_line);
                    input_line[len] = '\r';
                    input_line[len + 1] = '\n';
                    static int gps_gga_req_cnt = 0;
                    drp_write(10, 0, gps_gga_req_cnt++, input_line, len + 2);
                    LOGD("gps_gga_req_send:%d,%s\n", gps_rtcm_timeout, input_line);
                    gps_rtcm_age = gps_rtcm_timeout;
                    gps_rtcm_timeout = gps_rtcm_timeout * 2;
                    if (gps_rtcm_timeout > 240)
                    {
                        gps_rtcm_timeout = 240;
                        gps_rtcm_age = 120;
                    }
                    gps_gga_req = 0;
                }
                LOGD("cp_report");
            //     cp_report.UTC = gps_parse->UTC;
            //     cp_report.LNG = gps_parse->LNG % 0x100000000ul;
            //     cp_report.LNG_H = gps_parse->LNG / 0x100000000ul;
            //     cp_report.LAT = gps_parse->LAT % 0x100000000ul;
            //     cp_report.LAT_H = gps_parse->LAT / 0x100000000ul;
            //     cp_report.ALT = gps_parse->ALT % 0x10000;
            //     cp_report.ALT_H = gps_parse->ALT / 0x10000;
            //     cp_report.speed = gps_parse->speed < 65536 ? gps_parse->speed : 65535;
            //     cp_report.HDG = gps_parse->azimuth;
            //     cp_report.status = gps_parse->fix_quality;
            //     cp_report.TP[0] = gWordVar[REG_TEMPS];
            //     if (senser_cfg->cmv_type > 0)
            //     {
            //         cp_report.TP[1] = gWordVar[REG_REAL_PRESSURE_VAL];
            //         cp_report.TP[2] = gWordVar[REG_FEQ];
            //         cp_report.TP[3] = gWordVar[REG_AMPLITUDE];
            //         LOGD("drp_write start");
            //         // if (drp_write(12, 2, tran_id++, &cp_report, sizeof(cp_report)) == 0)
            //         // { // 成功则显示推送信息
            //         //     LOGD("drp_write ep=12,tran_id=%d UTC:[%d], TP:[%d], flow2[%d]\n", cp_report.UTC, cp_report.TP[0]);
            //         // }
            //     }
            //     else if (senser_cfg->temp_count > 1)
            //     {
            //         cp_report.TP[1] = gWordVar[REG_TEMPS + 1];
            //         cp_report.TP[2] = gWordVar[REG_TEMPS + 2];
            //         // if (drp_write(13, 2, tran_id++, &cp_report, sizeof(cp_report) - 2) == 0)
            //         // {
            //         //     LOGD("drp_write ep=13,tran_id=%d UTC:[%d], TP:[%d], flow2[%d]\n", cp_report.UTC, cp_report.TP[0]);
            //         // }
            //     }
            //     else
            //     {
            //         // if (drp_write(11, 2, tran_id++, &cp_report, sizeof(cp_report) - 6) == 0)
            //         // {
            //         //     LOGD("drp_write ep=11,tran_id=%d UTC:[%d], TP:[%d], flow2[%d]\n", cp_report.UTC, cp_report.TP[0]);
            //         // }
            //     }
            }
            // gWordVar[REG_WIEGAND_DI_VALUE] = read_wiegand_io();
        }
        else
        {
            LOGD("$xxGGA sentence is not parsed\n");
        }
        // gps_parse->ALT = minmea_rescale(&frame.altitude, 10);
    }
    break;

    case MINMEA_SENTENCE_GST:
    {
        struct minmea_sentence_gst frame;
        if (minmea_parse_gst(&frame, input_line))
        {
            LOGD(
                "$xxGST: raw latitude,longitude and altitude error deviation: (%d/%d,%d/%d,%d/%d)\n",
                frame.latitude_error_deviation.value, frame.latitude_error_deviation.scale,
                frame.longitude_error_deviation.value, frame.longitude_error_deviation.scale,
                frame.altitude_error_deviation.value, frame.altitude_error_deviation.scale);
            LOGD(
                "$xxGST fixed point latitude,longitude and altitude error deviation"
                " scaled to one decimal place: (%d,%d,%d)\n",
                minmea_rescale(&frame.latitude_error_deviation, 10),
                minmea_rescale(&frame.longitude_error_deviation, 10),
                minmea_rescale(&frame.altitude_error_deviation, 10));
            LOGD(
                "$xxGST floating point degree latitude, longitude and altitude error deviation: "
                "(%f,%f,%f)",
                minmea_tofloat(&frame.latitude_error_deviation),
                minmea_tofloat(&frame.longitude_error_deviation),
                minmea_tofloat(&frame.altitude_error_deviation));
        }
        else
        {
            LOGD("$xxGST sentence is not parsed\n");
        }
    }
    break;

    case MINMEA_SENTENCE_GSV:
    {
        struct minmea_sentence_gsv frame;
        if (minmea_parse_gsv(&frame, input_line))
        {

            LOGD("$xxGSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
            LOGD("$xxGSV: sattelites in view: %d\n", frame.total_sats);
            gps_parse->azimuth = frame.sats[0].azimuth;
            for (int i = 0; i < 4; i++)
            {
                if (frame.sats[i].nr > 0)
                {
                    gps_parse->total_gsv++;
                }
                LOGD("$xxGSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
                     frame.sats[i].nr, frame.sats[i].elevation, frame.sats[i].azimuth,
                     frame.sats[i].snr);
            }
        }
        else
        {
            LOGD("$xxGSV sentence is not parsed\n");
        }
    }
    break;

    case MINMEA_SENTENCE_VTG:
    {
        struct minmea_sentence_vtg frame;
        if (minmea_parse_vtg(&frame, input_line))
        {
            LOGD("$xxVTG: true track degrees = %f\n",
                 minmea_tofloat(&frame.true_track_degrees));
            LOGD("        magnetic track degrees = %f\n",
                 minmea_tofloat(&frame.magnetic_track_degrees));
            LOGD("        speed knots = %f\n", minmea_tofloat(&frame.speed_knots));
            LOGD("        speed kph = %f\n", minmea_tofloat(&frame.speed_kph));
            // gps_parse->speed = minmea_rescale(&frame.speed_kph, 1000);
            // gWordVar[REG_SPEED] = gps_parse->speed;
            if (senser_cfg->speed_dec > 50)
            {
                senser_cfg->speed_dec = 50;
            }
            int16_t speed = minmea_rescale(&frame.speed_kph, 1000) - senser_cfg->speed_dec;
            if (speed < 0)
            {
                speed = 0;
            }else if (senser_cfg->speed_max > 0 && speed > senser_cfg->speed_max)
            {
                speed = senser_cfg->speed_max;
            }
            gps_parse->speed = speed_avg(speed, senser_cfg->speed_avg); // minmea_rescale(&frame.speed_kph, 1000) - 30; // 20231019 摊铺机速度
            gWordVar[REG_SPEED] = gps_parse->speed;
        }
        else
        {
            LOGD("$xxVTG sentence is not parsed\n");
        }
    }
    break;

    case MINMEA_SENTENCE_ZDA:
    {
        struct minmea_sentence_zda frame;
        if (minmea_parse_zda(&frame, input_line))
        {
            LOGD("$xxZDA: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d\n", frame.time.hours,
                 frame.time.minutes, frame.time.seconds, frame.date.day, frame.date.month,
                 frame.date.year, frame.hour_offset, frame.minute_offset);
        }
        else
        {
            LOGD("$xxZDA sentence is not parsed\n");
        }
    }
    break;

    case MINMEA_SENTENCE_GSA:
    {
        struct minmea_sentence_gsa frame;
        if (minmea_parse_gsa(&frame, input_line))
        {
            LOGD(
                "$xxGSA: "
                "mode:%d\nfix_type:%d\nsats[0]:%d\nsats[1]:%d\nsats[2]:%d\nsats[3]:%d\nsats[4]:%"
                "d\nsats[5]:%d\nsats[6]:%d\nsats[7]:%d\nsats[8]:%d\nsats[9]:%d\nsats[10]:%d\nsats["
                "11]:%d\npdop:%f\nhdop:%f\nvdop:%f\n",
                frame.mode, frame.fix_type, frame.sats[0], frame.sats[1], frame.sats[2],
                frame.sats[3], frame.sats[4], frame.sats[5], frame.sats[6], frame.sats[7],
                frame.sats[8], frame.sats[9], frame.sats[10], frame.sats[11],
                minmea_tofloat(&frame.pdop), minmea_tofloat(&frame.hdop),
                minmea_tofloat(&frame.vdop));
        }
        else
        {
            LOGD("$xxGSA sentence is not parsed\n");
        }
    }
    break;

    case MINMEA_SENTENCE_GLL:
    {
        struct minmea_sentence_gll frame;
        if (minmea_parse_gll(&frame, input_line))
        {
            gps_parse->Q = frame.status;
            LOGD("$xxGLL: latitude:%f\nlongitude:%f\nstatus:%d\nmode:%d\n",
                 minmea_tofloat(&frame.latitude), minmea_tofloat(&frame.longitude), frame.status,
                 frame.mode);
            LOGD(
                "$xxGLL: time.hours:%d\ntime.minutes:%d\ntime.seconds:%d\ntime.microseconds:%d\n",
                frame.time.hours, frame.time.minutes, frame.time.seconds, frame.time.microseconds);
        }
        else
        {
            LOGD("$xxGLL sentence is not parsed\n");
        }
    }
    break;

    case MINMEA_INVALID:
    {
        // log_printf(LDEBUG, LOG_TAG "$xxxxx sentence is not valid\n");
    }
    break;

    default:
    {
        // log_printf(LDEBUG, LOG_TAG "$xxxxx sentence is not parsed\n");
    }
    break;
    }
    return 0;
}

void ql_uart_notify_cb(unsigned int ind_type, ql_uart_port_number_e port, unsigned int size)
{
    switch (ind_type)
    {
    case QUEC_UART_RX_OVERFLOW_IND: // rx buffer overflow
    case QUEC_UART_RX_RECV_DATA_IND:
    {
        osiEvent_t event;
        if (port == QL_UART_PORT_2)
        {
            event.id = 9;
        }
        else if (port == QL_UART_PORT_3)
        {
            event.id = 10;
        }
        else if (port == QL_USB_PORT_NMEA)
        {
            event.id = 11;
        }
        LOGI("UART port %d receive ind type:0x%x, receive data size:%d", port, ind_type, size);
        if (rtk_thread)
        {
            osiEventSend(rtk_thread, &event);
        }
        break;
    }
    case QUEC_UART_TX_FIFO_COMPLETE_IND:
    {
        LOGI("tx fifo complete");
        break;
    }
    }
    // free(recv_buff);
    // recv_buff = NULL;
}

gps_t gps1_parse; // gps1解析后的结果
// gps_t gps2_parse;                        // gps2解析后的结果
#define GPS1_BUFF_SIZE 2048            // gps1串口接收缓冲区大小
#define GPS2_BUFF_SIZE 2048            // gps2串口接收缓冲区大小
static char gps1_buff[GPS1_BUFF_SIZE]; // gps1串口接收缓冲区
// static char gps2_buff[GPS2_BUFF_SIZE];   // gps2串口接收缓冲区
static char gps2_txbuff[GPS2_BUFF_SIZE]; // gps2串口接收缓冲区
int uart2_rtcm_enable = 0;               // uart2是否需要 rtcm
// int uart3_rtcm_enable = 0;               // uart3是否需要 rtcm
//  static char gps_buf2[1024]; // gps2串口接收缓冲区
/**
 * @brief gps任务处理线程
 *
 * @param param
 */

// extern gps_message_t *gps_message = (gps_message_t*)&gWordVar[REG_GPS_DATA_CFG];
              
void parse_HEADINGA(gps_t *gps_parse, char input_line[])
{
    if (strncmp(input_line, "#HEADINGA", strlen("#HEADINGA")) != 0) 
        return;

    char *token;
    token = strtok(input_line, ";");    // 第一个
    LOGI( "%s\n", token );
    for (int i = 2; token && i <= 6; i++ )    // i表示第几个 （4, 5, 6字段分别是length, heading, pitch）
    {
        token = strtok(NULL, ",");
        // LOGI( "i=%d:%s\n", i, token );
        switch (i)
        {
            case 4:
                LOGI("length %s\n", token);
                sscanf(token, "%f", &gps_parse->baseline);
                break;
            case 5:
                LOGI("heading %s\n", token);
                sscanf(token, "%f", &gps_parse->Heading);
                break;
            case 6:
                LOGI("pitch %s\n", token);
                sscanf(token, "%f", &gps_parse->pitch);
                // gps_message->pitch = gps_parse->pitch;
                break;
        }
    }
    // gps_calc_target_point_(gps_parse);
}
static void rtk_gps_task(void *param)
{

    LOGD("rtk_gps_task start...\n");
    int ret = 0;
    osiTimer_t *ptimer_t = NULL;
    //    ql_gpio_set_level(GPIO_23, LVL_HIGH);
    //    ql_rtos_task_sleep_ms(100);
    // ql_pin_set_func(13,4);
    ql_gpio_set_level(GPIO_23, LVL_LOW);
    ql_uart_config_s uart_cfg = {
        .baudrate = QL_UART_BAUD_115200,
        .flow_ctrl = QL_FC_NONE,
        .data_bit = QL_UART_DATABIT_8,
        .stop_bit = QL_UART_STOP_1,
        .parity_bit = QL_UART_PARITY_NONE};
    ret = ql_uart_set_dcbconfig(QL_UART_PORT_2, &uart_cfg);
    if (ret != 0)
    {
        LOGE("ql_uart_set_dcbconfig ret: 0x%x", ret);
    }
    ret = ql_uart_open(QL_UART_PORT_2);
    if (ret != 0)
    {
        LOGE("ql_uart_open ret: 0x%x", ret);
    }
    LOGD("uart2 init success\n");
    ql_uart_set_event_mask(QL_UART_PORT_2, QL_UART_EVENT_RX_ARRIVED | QL_UART_EVENT_RX_OVERFLOW);
    ret = ql_uart_register_cb(QL_UART_PORT_2, ql_uart_notify_cb);
    if (ret != 0)
    {
        LOGE("ql_uart_register_cb ret: 0x%x", ret);
    }
    uart2_rtcm_enable = 1;

    // ret = ql_uart_set_dcbconfig(QL_UART_PORT_3, &uart_cfg);
    // if (ret != 0)
    // {
    //     LOGE("ql_uart_set_dcbconfig ret: 0x%x", ret);
    // }
    // ret = ql_uart_open(QL_UART_PORT_3);
    // if (ret != 0)
    // {
    //     LOGE("ql_uart_open ret: 0x%x", ret);
    // }
    // ql_uart_set_event_mask(QL_UART_PORT_3, QL_UART_EVENT_RX_ARRIVED | QL_UART_EVENT_RX_OVERFLOW);
    // ret = ql_uart_register_cb(QL_UART_PORT_3, ql_uart_notify_cb);
    // if (ret != 0)
    // {
    //     LOGE("ql_uart_register_cb ret: 0x%x", ret);
    // }
    // uart3_rtcm_enable = 1;
    // LOGD("uart3 init success\n");

    ret = ql_uart_open(QL_USB_PORT_NMEA);
    if (ret != 0)
    {
        LOGE("ql_uart_open_usb ret: 0x%x", ret);
    }
    ql_uart_set_event_mask(QL_USB_PORT_NMEA, QL_UART_EVENT_RX_ARRIVED | QL_UART_EVENT_RX_OVERFLOW);
    ret = ql_uart_register_cb(QL_USB_PORT_NMEA, ql_uart_notify_cb);
    if (ret != 0)
    {
        LOGE("ql_uart_register_cb ret: 0x%x", ret);
    }

    rtk_thread = osiThreadCurrent();
    LOGD("uart2 init success\n");
    NmeaTcp_init();
    int sync_timeout = 0;
    int reset_times = 0;
    int config_index = 0;
    
    gps_point_coord_init();
   
    while (1)
    {
        osiEvent_t waitevent;
        bool ret = osiEventTryWait(rtk_thread, &waitevent, 1000);
        if (ret)
        { // 响应到事件发生
            switch (waitevent.id)
            {

            case 9:
            { // uart1-gps1接收事件处理函数
                static int pos = 0;
                static int end_pos = 0;
                int len = ql_uart_read(QL_UART_PORT_2, (uint8_t *)&gps1_buff[end_pos], GPS1_BUFF_SIZE - end_pos);
                LOGI("uart2 read len=%d end_pos=%d\n", len, end_pos);
                if (len > 0)
                {
                    ql_uart_write(QL_USB_PORT_NMEA, &gps1_buff[end_pos], len);
                    reset_times = 0;
                    end_pos += len;
                    pos = 0;
                    gps1_buff[end_pos] = '\0';
                    LOGD("uart2 len=%d,msg=%s\n", len, gps1_buff);
                    while (pos < end_pos - 1)
                    {
                        if ((gps1_buff[pos] == '$') && (gps1_buff[pos + 1] == 'G') || (gps1_buff[pos] == '#') && (gps1_buff[pos + 1] == 'H')) //-- $G || $B nema ??
                        {
                            // itn len = get_line(pos,gps1_buff,end_pos)
                            char *start = &gps1_buff[pos];
                            char *end = strchr(start, '\n');
                            sync_timeout = 0;
                            if (end != NULL)
                            {
                                *end = '\0';
                                if (*(end - 1) == '\r')
                                {
                                    *(end - 1) = '\0';
                                }
                                LOGD("gps1 nmea = [%s]", start);
                                gps1_parse.id = GPS_ID_1;

                                if ((gps1_buff[pos] == '$') && (gps1_buff[pos + 1] == 'G'))
                                    parse_nmea_0183(&gps1_parse, start);
                                else if ((gps1_buff[pos] == '#') && (gps1_buff[pos + 1] == 'H')) 
                                    parse_HEADINGA(&gps1_parse, start);

                                // parse_nmea_0183(&gps1_parse, start);
                                gps_cale_point(&gps1_parse);
                                judg_location();
                                get_gps_xy();
                                // log_printf(LDEBUG, LOG_TAG "uart LAT=%d, LNG=%d, ALT=%d, Status=%d\n",
                                //            gps_parse_1.LAT, gps_parse_1.LNG, gps_parse_1.ALT, gps_parse_1.Q);
                                pos += (end - start) + 1;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            pos++;
                        }
                    }
                    if (pos < end_pos)
                    {
                        int len = strlen((char *)&gps1_buff[pos]);
                        memmove(gps1_buff, (char *)&gps1_buff[pos], len);
                        end_pos = end_pos - pos;
                    }
                    else
                    {
                        // memset(gps1_buff, 0, end_pos);
                        end_pos = 0;
                    }
                    // if (sync_timeout++ > 60)
                    // {
                    //   gWordVar[REG_UBLOX_CFG_REQ] = 0x4321;
                    //   sync_timeout = 0;
                    // }
                }
            }
            break;
            // case 10: { // uart2-gps1接收事件处理函数
            //     static int pos = 0;
            //     static int end_pos = 0;
            //     LOGI("uart3 read event pos=%d,end_pos=%d\n", pos, end_pos);
            //     int len = ql_uart_read(QL_UART_PORT_3, (uint8_t *)&gps2_buff[end_pos], GPS2_BUFF_SIZE - end_pos);
            //     if (len > 0)
            //     {
            //         ql_uart_write(QL_USB_PORT_NMEA, &gps2_buff[end_pos], len);
            //         reset_times = 0;
            //         end_pos += len;
            //         pos = 0;
            //         gps2_buff[end_pos] = '\0';
            //         LOGD("uart3 len=%d,msg=%s\n", len, gps2_buff);
            //         while (pos < end_pos - 1)
            //         {
            //             if ((gps2_buff[pos] == '$') && (gps2_buff[pos + 1] == 'G')) //-- $G || $B nema ??
            //             {
            //                 // itn len = get_line(pos,gps2_buff,end_pos)
            //                 char *start = &gps2_buff[pos];
            //                 char *end = strchr(start, '\n');
            //                 sync_timeout = 0;
            //                 if (end != NULL)
            //                 {
            //                     *end = '\0';
            //                     if (*(end - 1) == '\r')
            //                     {
            //                         *(end - 1) = '\0';
            //                     }
            //                     // log_printf(LDEBUG, LOG_TAG "nmea = [%s]", start);
            //                     LOGD("gps2 nmea = [%s]", start);
            //                     gps2_parse.id = GPS_ID_2;
            //                     parse_nmea_0183(&gps2_parse, start);
            //                     // log_printf(LDEBUG, LOG_TAG "uart LAT=%d, LNG=%d, ALT=%d, Status=%d\n",
            //                     //            gps_parse_1.LAT, gps_parse_1.LNG, gps_parse_1.ALT, gps_parse_1.Q);
            //                     pos += (end - start) + 1;
            //                 }
            //                 else
            //                 {
            //                     break;
            //                 }
            //             }
            //             else
            //             {
            //                 pos++;
            //             }
            //         }
            //         if (pos < end_pos)
            //         {
            //             int len = strlen((char *)&gps2_buff[pos]);
            //             memmove(gps2_buff, (char *)&gps2_buff[pos], len);
            //             end_pos = end_pos - pos;
            //         }
            //         else
            //         {
            //             // memset(gps1_buff, 0, end_pos);
            //             end_pos = 0;
            //         }
            //         // if (sync_timeout++ > 60)
            //         // {
            //         //   gWordVar[REG_UBLOX_CFG_REQ] = 0x4321;
            //         //   sync_timeout = 0;
            //         // }
            //     }
            // }
            // break;
            case 11:
            {
                int len = ql_uart_read(QL_USB_PORT_NMEA, gps2_txbuff, sizeof(gps2_txbuff));
                LOGI("uart nmea read len=%d\n", len);
                ql_uart_write(QL_UART_PORT_2, gps2_txbuff, len);
            }
            break;
            case 12:
                if (config_index < sizeof(um982_config) / sizeof(um982_config[0]))
                {
                    ql_uart_write(QL_UART_PORT_2, um982_config[config_index], strlen(um982_config[config_index]));
                    LOGD("cmd=%s\n", um982_config[config_index]);
                    config_index++;
                    osiTimerStart(ptimer_t, 100);
                }
                else
                {
                    config_index = -1;
                    uart2_rtcm_enable = 1;
                }
                break;
            case 9999:
                config_index = 0;
                uart2_rtcm_enable = 0;
                if (ptimer_t == NULL)
                {
                    ptimer_t = osiTimerCreate(rtk_thread, config_timer_cb, NULL);
                }
                LOGD("start config\n");
                osiTimerStart(ptimer_t, 100);
                break;
            default:
                break;
            }
            waitevent.id = 0;
        }
        else
        {
            LOGW("gps timeout\n");
            if (config_index == 0)
            {
                if (ptimer_t == NULL)
                {
                    ptimer_t = osiTimerCreate(rtk_thread, config_timer_cb, NULL);
                }
                osiTimerStart(ptimer_t, 100);
            }

            // if (++reset_times > 10)
            // {
            //   int value = 1;
            //   LSAPI_Device_Write(GPS_REST, &value, 1);
            //   LSAPI_OSI_ThreadSleep(50);
            //   value = 0;
            //   LSAPI_Device_Write(GPS_REST, &value, 1);
            //   reset_times = 0;
            // }
        }
    }
err1:
    LOGI("gps_app_thread end...\n");
    osiThreadExit();
}

void rtk_rtcm_intput(uint8_t *data, int length)
{
    if (length <= 0)
    {
        return;
    }
    if (uart2_rtcm_enable)
    {
        ql_uart_write(QL_UART_PORT_2, data, length);
    }
    gps_rtcm_age = 0;
    if (gps_rtcm_timeout <= 240)
    {
        gps_rtcm_timeout = 10;
    }
}

void rtk_gps_task_init(void)
{
    QlOSStatus err = 0;
    ql_task_t task_ref;

    err = ql_rtos_task_create(&task_ref, 4096, APP_PRIORITY_NORMAL, "rtk_gps", rtk_gps_task, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        LOGE("creat rtk_gps_task fail err = %d", err);
    }
    LOGI("creat rtk_gps_task sucess!");
}
