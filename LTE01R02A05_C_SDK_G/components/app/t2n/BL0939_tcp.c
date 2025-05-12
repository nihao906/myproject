#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "ql_api_osi.h"
#include "ql_log.h"
#include "../ec600u_bl0939/ac_current.h"
#include "../t2n/t2n_priv.h"

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "BL0939_tcp", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "BL0939_tcp", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "BL0939_tcp", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "BL0939_tcp", msg, ##__VA_ARGS__)

extern dosing_config_t *dosing;
extern hammer_config_t *hammer;
extern total_config_t *work;
extern gps_message_t *gps_message;
void writedataUInt32LE(uint8_t *buffer, uint32_t value, uint16_t offset)
{
    buffer[offset] = value & 0xFF;
    buffer[offset + 1] = (value >> 8) & 0xFF;
    buffer[offset + 2] = (value >> 16) & 0xFF;
    buffer[offset + 3] = (value >> 24) & 0xFF;
}
void writedataUInt16LE(uint8_t *buffer, uint16_t value, uint16_t offset)
{
    buffer[offset] = value & 0xFF;
    buffer[offset + 1] = (value >> 8) & 0xFF;
}
void writedataUInt8LE(uint8_t *buffer, uint8_t value, uint16_t offset)
{
    buffer[offset] = value;
}

/*发送的时间的单位均为0.02s，长度为uint16_t    除了开始时间以及结束时间是系统获取的时间戳*/
/*发送pile数据事件：1.判断处于换桩状态
                    2.行走电机处于工作状态大于60秒填料次数大于10

*/
void report_pile_data(void)
{
    uint8_t buffer[50];
    uint16_t offset = 0;

    // 上报数据
    writedataUInt32LE(buffer, (uint32_t)(first_dosing_time / 1000), offset); // 开始的时间戳
    offset = offset + 4;

    writedataUInt32LE(buffer, (uint32_t)(end_hammer_time / 1000), offset); // 结束的时间戳
    offset = offset + 4;

    writedataUInt32LE(buffer, (uint32_t)(gps_message->x * 1000000000), offset);
    offset = offset + 4;

    writedataUInt8LE(buffer, (uint8_t)(gps_message->x * 1000000000 / 0x100000000), offset);
    offset = offset + 1;

    writedataUInt32LE(buffer, (uint32_t)(gps_message->y * 1000000000), offset);
    offset = offset + 4;

    writedataUInt8LE(buffer, (uint8_t)(gps_message->y * 1000000000 / 0x100000000), offset);
    offset = offset + 1;

    writedataUInt32LE(buffer, (uint32_t)(gps_message->height), offset); // 高度占三字节
    offset = offset + 3;

    writedataUInt8LE(buffer, gps_message->gps_status, offset);
    offset = offset + 1;

    writedataUInt16LE(buffer, (uint16_t)(gps_message->pitch * 100), offset);
    offset = offset + 2;

    writedataUInt16LE(buffer, work->id, offset);
    offset = offset + 2;

    writedataUInt16LE(buffer, (uint16_t)dosing->work_time, offset);
    offset = offset + 2;

    writedataUInt16LE(buffer, (uint16_t)dosing->total_time, offset);
    offset = offset + 2;

    writedataUInt16LE(buffer, dosing->cnt, offset);
    offset = offset + 2;

    writedataUInt16LE(buffer, (uint16_t)hammer->total_time, offset);
    offset = offset + 2;

    writedataUInt16LE(buffer, hammer->cnt, offset);
    offset = offset + 2;

    writedataUInt16LE(buffer, (uint16_t)work->total_time, offset);
    offset = offset + 2;

    writedataUInt16LE(buffer, (uint16_t)(depth), offset);
    offset = offset + 2;

    int flag = 0;
    static int report_id = 0;
    flag = drp_write(19, 2, report_id++, buffer, sizeof(buffer));
    if (flag == 0)
    {
        LOGI("drp_write ep = 19,id=%d", report_id);
    }
}

/*发送process是数据事件：1.加料电机状态改变
                        2.夯击电机状态改变
*/
ql_timeval_t N_time;
uint32_t now = 0;
void report_process_data(void)
{
    uint8_t data_buffer[50];
    uint16_t data_offset = 0;
    uint32_t last_time = 0;

    ql_gettimeofday(&N_time);
    now = (uint32_t)N_time.sec * 1000 + N_time.usec / 1000; // 当前时间  获取的是毫秒 是ec600u时间戳
    if (now - last_time > 100)
    {
        long long dgree;
        writedataUInt32LE(data_buffer, (uint32_t)(gps_message->utc), data_offset);
        data_offset = data_offset + 4;

        dgree = gps_message->x * 1e9;
        writedataUInt32LE(data_buffer, (uint32_t)(dgree), data_offset);
        data_offset = data_offset + 4;

        dgree /= 0x100000000;
        writedataUInt8LE(data_buffer, (uint8_t)(dgree), data_offset);
        data_offset = data_offset + 1;

        dgree = gps_message->y * 1e9;
        writedataUInt32LE(data_buffer, (uint32_t)(dgree), data_offset);
        data_offset = data_offset + 4;

        dgree /= 0x100000000;
        writedataUInt8LE(data_buffer, (uint8_t)(dgree), data_offset);
        data_offset = data_offset + 1;

        writedataUInt32LE(data_buffer, gps_message->height, data_offset); // 高度占三字节
        data_offset = data_offset + 3;

        writedataUInt8LE(data_buffer, gps_message->gps_status, data_offset);
        data_offset = data_offset + 1;

        writedataUInt16LE(data_buffer, (short)(gps_message->pitch*100), data_offset);
        data_offset = data_offset + 2;

        writedataUInt16LE(data_buffer, work->id, data_offset);
        data_offset = data_offset + 2;

        writedataUInt16LE(data_buffer, timeline, data_offset);
        data_offset = data_offset + 2;

        writedataUInt8LE(data_buffer, times, data_offset); //--同一根桩，相同事件第几次发生  -=-填料次数---夯击次数
        data_offset = data_offset + 1;

        writedataUInt8LE(data_buffer, DATA_status, data_offset); //--事件ID 0空闲 1 填料 2夯击
        data_offset = data_offset + 1;

        writedataUInt16LE(data_buffer, (uint16_t)(duration), data_offset); // worktime  单位0.02秒
        data_offset = data_offset + 2;

        writedataUInt16LE(data_buffer, (uint16_t)(subtotal_time), data_offset); // total_time
        data_offset = data_offset + 2;

        writedataUInt16LE(data_buffer, depth, data_offset);
        data_offset = data_offset + 2;

        int flag = 0;
        static int report_data_id = 0;
        flag = drp_write(18, 2, report_data_id++, data_buffer, sizeof(data_buffer));
        if (flag == 0)
        {
            LOGI("drp_write ep=18,report_data_id=%d,", report_data_id);
        }
    }
    else
    {
        LOGI("time error");
    }
    last_time = now;
}
