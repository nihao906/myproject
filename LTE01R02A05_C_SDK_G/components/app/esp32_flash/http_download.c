#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
// #include "../net/http2/include/http_api.h"

#include "ql_log.h"
#include "ql_api_datacall.h"
#include "ql_http_client.h"
#include "ql_fs.h"
#include "http_download.h"
#include "../sdmmc/sdmmc.h"
#include "esp32_flash.h"

typedef enum
{
    QHTTPC_EVENT_RESPONSE = 1001,
    QHTTPC_EVENT_FAILED,
    QHTTPC_EVENT_END,
} qhttpc_event_code_e;

typedef struct
{
    http_client_t http_client;
    ql_queue_t queue;

    ql_mutex_t simple_lock;
    bool dl_block;
    int dl_high_line;
    int dl_total_len;

    uint64_t file_size;         // 用于计算下载进度
    uint64_t file_write_len;

    QFILE upload_fd;
    QFILE dload_fd;
} qhttpc_ctx_t;
qhttpc_ctx_t EC600U_test_http_demo_client = {0}; // http通信结构体
#define HTTP_DLOAD_HIGH_LINE 40960
#define EC600U_HTTP_LOG(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "EC600U_HTTP_DLOAD", msg, ##__VA_ARGS__)

// #define EC600U_HTTP_LOG(msg, ...) do { \
// 	QL_LOG(QL_LOG_LEVEL_INFO, "EC600U_HTTP_DLOAD", msg, ##__VA_ARGS__);	\
// 	esp32_ota_log(LINFO, msg, ##__VA_ARGS__); \
// } while (0);


// int file_size = 0;
/*http事件回调函数*/
static void http_event_cb(http_client_t *client, int evt, int evt_code, void *arg)
{
    qhttpc_ctx_t *client_ptr = (qhttpc_ctx_t *)arg; // http通信结构体
    ql_event_t qhttpc_event_send = {0};             // 事件结构体

    EC600U_HTTP_LOG("enter");

    if (client_ptr == NULL)
        return;

    EC600U_HTTP_LOG("*client:0x%x, http_cli:0x%x", *client, client_ptr->http_client);

    if (*client != client_ptr->http_client)
        return;

    EC600U_HTTP_LOG("evt:%d, evt_code:%d", evt, evt_code);

    // evt为事件类型,获取对应信息输出到日志中
    switch (evt)
    {
        case HTTP_EVENT_SESSION_ESTABLISH: { // HTTP 会话建立事件
            // 事件状态
            if (evt_code != QL_HTTP_OK)
            {
                EC600U_HTTP_LOG("HTTP session create failed!!!!!");
                qhttpc_event_send.id = QHTTPC_EVENT_FAILED;
                qhttpc_event_send.param1 = (uint32)client_ptr;
                ql_rtos_queue_release(client_ptr->queue, sizeof(ql_event_t), (uint8 *)&qhttpc_event_send, QL_WAIT_FOREVER);
            }
            else EC600U_HTTP_LOG("HTTP session establish!");
        }
        break;
        case HTTP_EVENT_RESPONE_STATE_LINE: { // HTTP 响应状态行解析完成事件,
            if (evt_code == QL_HTTP_OK)
            {
                int resp_code = 0;      // 状态码信息
                int content_length = 0; // 响应主体的字节长度
                int chunk_encode = 0;   // 响应主体的分块传输编码类型
                char *location = NULL;  // 重定向或指示资源的位置

                // 获取已配置的http client上下文的配置信息,存储到resp_code
                ql_httpc_getinfo(client, HTTP_INFO_RESPONSE_CODE, &resp_code); // 获取响应的状态码信息
                EC600U_HTTP_LOG("response code:%d", resp_code);
                ql_httpc_getinfo(client, HTTP_INFO_CHUNK_ENCODE, &chunk_encode); // 响应主体的分块传输编码类型

                // 获取成功
                if (chunk_encode == 0)
                {
                    ql_httpc_getinfo(client, HTTP_INFO_CONTENT_LEN, &content_length); // 响应主体的字节长度
                    // file_size = content_length;
                    client_ptr->file_size = content_length;
                    EC600U_HTTP_LOG("content_length:%d", content_length);
                }
                else
                {
                    EC600U_HTTP_LOG("http chunk encode!!!");
                }

                if (resp_code >= 300 && resp_code < 400)
                {
                    ql_httpc_getinfo(client, HTTP_INFO_LOCATION, &location); // 重定向或指示资源的位置
                    EC600U_HTTP_LOG("redirect location:%s", location);
                    free(location);
                }
            }
        }
        break;
        case HTTP_EVENT_SESSION_DISCONNECT: { // HTTP 会话断开事件
            if (evt_code == QL_HTTP_OK)
            {
                EC600U_HTTP_LOG("===http transfer end!!!!");
                qhttpc_event_send.id = QHTTPC_EVENT_END;
            }
            else
            {
                EC600U_HTTP_LOG("===http transfer occur exception:%d!!!!!", evt_code);
                qhttpc_event_send.id = QHTTPC_EVENT_FAILED;
            }
            qhttpc_event_send.param1 = (uint32)client_ptr;
            ql_rtos_queue_release(client_ptr->queue, sizeof(ql_event_t), (uint8 *)&qhttpc_event_send, QL_WAIT_FOREVER);
            break;
        }
    }
}

static int EC600U_http_download_data(http_client_t *client, void *arg, char *data, int size, unsigned char end){
    int ret = size;
    uint32 msg_cnt = 0;                             // 消息计数
    char *read_buff = NULL;                         // 读数组指针
    qhttpc_ctx_t *client_ptr = (qhttpc_ctx_t *)arg; // 数据结构体
    ql_event_t qhttpc_event_send = {0};

    EC600U_HTTP_LOG("enter");

    if (client_ptr == NULL)
        return 0;

    EC600U_HTTP_LOG("*client:0x%x, http_cli:0x%x", *client, client_ptr->http_client);

    if (*client != client_ptr->http_client)
        return 0;

    read_buff = (char *)malloc(size + 1);
    if (read_buff == NULL)
    {
        EC600U_HTTP_LOG("mem faild");
        return 0;
    }

    memcpy(read_buff, data, size); // 将data指向的数据的size大小字节传递给read_buff

    // 获取消息队列中的消息数量，传递给msg_cnt
    if (QL_OSI_SUCCESS != ql_rtos_queue_get_cnt(client_ptr->queue, &msg_cnt))
    {
        free(read_buff);
        EC600U_HTTP_LOG("ql_rtos_queue_get_cnt faild");
        return 0;
    }
    EC600U_HTTP_LOG("msg_cnt %d, total_len+size %d", msg_cnt, (client_ptr->dl_total_len + size));

    // 上锁
    ql_rtos_mutex_lock(client_ptr->simple_lock, QL_WAIT_FOREVER);
    if (msg_cnt >= (MAX_HTTP_CLIENT_REQUEST_QUEUE_SIZE - 1) || (client_ptr->dl_total_len + size) >= client_ptr->dl_high_line)  
    {
        EC600U_HTTP_LOG("http block! msg_cnt:%d/%d  total_len+size:%d/%d", msg_cnt, MAX_HTTP_CLIENT_REQUEST_QUEUE_SIZE, client_ptr->dl_total_len + size, client_ptr->dl_high_line);
        client_ptr->dl_block = true;
        ret = QL_HTTP_ERROR_WONDBLOCK;
    }
    // 解锁
    ql_rtos_mutex_unlock(client_ptr->simple_lock);

    qhttpc_event_send.id = QHTTPC_EVENT_RESPONSE;
    qhttpc_event_send.param1 = (uint32)client_ptr;
    qhttpc_event_send.param2 = (uint32)read_buff;
    qhttpc_event_send.param3 = (uint32)size;

    // 释放消息队列
    if (QL_OSI_SUCCESS != ql_rtos_queue_release(client_ptr->queue, sizeof(ql_event_t), (uint8 *)&qhttpc_event_send, 0))
    {
        free(read_buff);
        EC600U_HTTP_LOG("ql_rtos_queue_release faild");
        return 0;
    }

    ql_rtos_mutex_lock(client_ptr->simple_lock, 100);
    client_ptr->dl_total_len += size;
    ql_rtos_mutex_unlock(client_ptr->simple_lock);

    EC600U_HTTP_LOG("http response :%d bytes data", size);

    return ret;
}


void EC600U_http_download_data_func(void*param)
{
    int ret = 0;
    int size = 0;
    QFILE fd = 0;
    bool dload_block = false;
    char *read_buff = NULL;
    qhttpc_ctx_t *client_ptr = NULL;
    ql_event_t *qhttpc_event_recv = (ql_event_t *)param;

    if (qhttpc_event_recv == NULL || qhttpc_event_recv->param1 == 0 || qhttpc_event_recv->param2 == 0 || qhttpc_event_recv->param3 == 0)
    {
        EC600U_HTTP_LOG("qhttpc_event_recv == NULL || qhttpc_event_recv->param1 == 0 || qhttpc_event_recv->param2 == 0 || qhttpc_event_recv->param3 == 0");
        return;
    }

    /*将数据通过http下载到sd卡中的文件里*/
    client_ptr = (qhttpc_ctx_t *)qhttpc_event_recv->param1;
    read_buff = (char *)qhttpc_event_recv->param2;
    size = (int)qhttpc_event_recv->param3;
    fd = (QFILE)client_ptr->dload_fd;
    ret = ql_fwrite(read_buff, size, 1, fd);
    if (ret > 0)
    {
        EC600U_HTTP_LOG("write data to dload file sucessfully!!!!");
        EC600U_HTTP_LOG("http write :%d bytes data", ret);
        client_ptr->file_write_len += ret;
        esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_DOWNLOADING, ESP32_FLASH_STATUS_DOING, (uint16_t)(client_ptr->file_write_len * 100.0 / client_ptr->file_size));
    }
    else
        EC600U_HTTP_LOG("write data to dload file failed!!!!");

    free(read_buff);

    // 获取当前的队列中的元素个数，如果小于SIZE，并且原来的http是阻塞的，则可以取消阻塞
    // 注，原来是只要发现是阻塞的就取消阻塞，这是有问题的，它假设了获取队列元素和该程序的执行是原子的，即执行完该函数一定有空间给http
    // 但是实际情况是，可能这个函数是在queue长度为2的时候取的，然而还没来来得及处理，http太快了，已经把队列填满了，因此这里直接取消阻塞会导致队列满
    uint32_t queue_cnt = 0;
    ql_rtos_queue_get_cnt(EC600U_test_http_demo_client.queue, &queue_cnt);

    ql_rtos_mutex_lock(client_ptr->simple_lock, 100);
    client_ptr->dl_total_len -= size;
    if (client_ptr->dl_total_len < 0)
        client_ptr->dl_total_len = 0;
    if (client_ptr->dl_block == true && client_ptr->dl_total_len < client_ptr->dl_high_line && queue_cnt < MAX_HTTP_CLIENT_REQUEST_QUEUE_SIZE)
    {
        dload_block = client_ptr->dl_block;
        client_ptr->dl_block = false;
    }
    ql_rtos_mutex_unlock(client_ptr->simple_lock);

    if (dload_block == true)
    {
        ql_httpc_continue_dload(&client_ptr->http_client); // 恢复client对应的http下载响应
        EC600U_HTTP_LOG("ql_httpc_continue_dload");
    }
}


int check_data_call(void)
{
    int ret = 0;
    int retry = 10;
    uint8_t nSim = 0;                  // sim卡号
    int profile_idx = 1;               // PDP索引
    ql_data_call_info_s info;          // 数据信息

    EC600U_HTTP_LOG("check_data_call:");
    esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_WAITING_NET, ESP32_FLASH_STATUS_DOING, 0);
    while (retry--) {
        ret = ql_get_data_call_info(nSim, profile_idx, &info);
        if (info.v4.state == 1 || info.v6.state == 1) break;
        EC600U_HTTP_LOG("no data call, waiting...(remain %d)", retry);
        ql_rtos_task_sleep_s((10-retry) * 2);
    }
    if (retry <= 0)
    {
        EC600U_HTTP_LOG("no data call, retry too many times, failed!");
        ret = -1;
        goto exit;
    }
    EC600U_HTTP_LOG("info.profile_idx: %d, info.ip_version: %d", info.profile_idx, info.ip_version);
	EC600U_HTTP_LOG("info->v4.state: %d, info.v6.state: %d", info.v4.state, info.v6.state);
    if(info.v4.state)
    {
		EC600U_HTTP_LOG("info.v4.addr.ip: %s", ip4addr_ntoa(&(info.v4.addr.ip)));
		EC600U_HTTP_LOG("info.v4.addr.pri_dns: %s", ip4addr_ntoa(&(info.v4.addr.pri_dns)));
		EC600U_HTTP_LOG("info.v4.addr.sec_dns: %s", ip4addr_ntoa(&(info.v4.addr.sec_dns)));
    }
    if(info.v6.state)
    {
		EC600U_HTTP_LOG("info.v6.addr.ip: %s", ip6addr_ntoa(&(info.v6.addr.ip)));
		EC600U_HTTP_LOG("info.v6.addr.pri_dns: %s", ip6addr_ntoa(&(info.v6.addr.pri_dns)));
		EC600U_HTTP_LOG("info.v6.addr.sec_dns: %s", ip6addr_ntoa(&(info.v6.addr.sec_dns)));
    }

exit:
    if (ret == 0)
    {
        esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_WAITING_NET, ESP32_FLASH_STATUS_FINISH, 0);
    }
    else 
    {
        esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_WAITING_NET, ESP32_FLASH_STATUS_ERR, 0);
    }
    return ret;
}


/* url地址 写入文件地址 阶段，为了表示下载的是哪个文件 */
int http_dowload(char *event_param1, char *file_name){

    int ret = 0;                       // 状态标志位
    int this_ret = 0;
    int i = 0;                         // 超时计数器
    int profile_idx = 1;               // PDP索引
    char ip4_addr_str[16] = {0};       // 存储ip地址字符串
    struct stat dload_stat;            // 下载状态
    uint8_t nSim = 0;                  // sim卡号
    int flags_break = 0;               // 退出循环标志
    ql_event_t qhttpc_event_msg = {0}; // http客户端事件信息

    EC600U_HTTP_LOG("http_dowload(%s, %s)", event_param1, file_name);
    esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_DOWNLOADING, ESP32_FLASH_STATUS_DOING, 0);

    int num_count = 1;
    while(num_count--){
        int http_method = HTTP_METHOD_NONE; 
        // 重置http结构体数据
        memset(&EC600U_test_http_demo_client, 0x00, sizeof(qhttpc_ctx_t));

        EC600U_test_http_demo_client.dl_block = false;                    // 设置下载标识符
        EC600U_test_http_demo_client.dl_high_line = HTTP_DLOAD_HIGH_LINE; // 设置下载操作的高水位线
        // 创建互斥锁，0代表成功
        ret = ql_rtos_mutex_create(&EC600U_test_http_demo_client.simple_lock);
        if (ret)
        {
            EC600U_HTTP_LOG("ql_rtos_mutex_create failed!!!!");
            this_ret = -2;
            goto exit;
        }
        // 创建一个操作系统的消息队列, 最大消息大小为结构体ql_event_t的大小，最大消息数量为2
        ret = ql_rtos_queue_create(&EC600U_test_http_demo_client.queue, sizeof(ql_event_t), MAX_HTTP_CLIENT_REQUEST_QUEUE_SIZE);
        if (ret){
            EC600U_HTTP_LOG("ql_rtos_queue_create failed!!!!");
            this_ret = -2;
            goto exit;
        }
        ret = ql_httpc_new(&EC600U_test_http_demo_client.http_client, http_event_cb, (void *)&EC600U_test_http_demo_client);
        if (ret){
            EC600U_HTTP_LOG("http client create failed!!!!");
            this_ret = -3;
            goto exit;
        }
        // 写入模式打开文件，将句柄赋值给dload_fd
        sdmmc_create_file_dir(file_name);
        EC600U_test_http_demo_client.dload_fd = ql_fopen(file_name, "wb+");
        if (EC600U_test_http_demo_client.dload_fd < 0)
        {
            EC600U_HTTP_LOG("open dload file failed!!!!");
            // 释放http client上下文,以及http client上下文所占用内存块
            ql_httpc_release(&EC600U_test_http_demo_client.http_client);
            this_ret = -4;
            goto exit;
        }
        EC600U_HTTP_LOG("open dload file sucessfully!!!!");

        // 通过opt_tag类型信息,配置http client上下文,第二个参数为参数处理类型，第三个参数为具体的参数
        ql_httpc_setopt(&EC600U_test_http_demo_client.http_client, HTTP_CLIENT_OPT_SIM_ID, nSim);
        ql_httpc_setopt(&EC600U_test_http_demo_client.http_client, HTTP_CLIENT_OPT_PDPCID, profile_idx);
        ql_httpc_setopt(&EC600U_test_http_demo_client.http_client, HTTP_CLIENT_OPT_WRITE_FUNC, EC600U_http_download_data); // 客户端写入处理
        ql_httpc_setopt(&EC600U_test_http_demo_client.http_client, HTTP_CLIENT_OPT_WRITE_DATA, (void *)&EC600U_test_http_demo_client);  // 客户端写入数据

        //char url[] = "http://220.180.239.212:8300/10K.txt";                                       // url地址
        http_method = HTTP_METHOD_GET;                                                            // http方法设为get
        ql_httpc_setopt(&EC600U_test_http_demo_client.http_client, HTTP_CLIENT_OPT_METHOD, http_method); // 指定请求方法
        ql_httpc_setopt(&EC600U_test_http_demo_client.http_client, HTTP_CLIENT_OPT_URL, (char *)event_param1);    // 设置请求的url

        // 关联http client此上下文和http的读写回调函数
        if (ql_httpc_perform(&EC600U_test_http_demo_client.http_client) == QL_HTTP_OK) // 关联成功
        {
            EC600U_HTTP_LOG("wait http perform end!!!!!!"); // 等待http请求执行结束
            flags_break = 0; // 重置中断标志位
            // 无限循环，等待程序中跳出
            for (;;)
            {
                memset(&qhttpc_event_msg, 0x00, sizeof(ql_event_t)); // 重置事件变量

                // 获取消息队列传递给结构体变量中的属性，param2为指向接收消息的缓冲区，param4为超时等待
                ql_rtos_queue_wait(EC600U_test_http_demo_client.queue, (uint8 *)&qhttpc_event_msg, sizeof(ql_event_t), QL_WAIT_FOREVER);

                switch (qhttpc_event_msg.id)
                {
                    case QHTTPC_EVENT_RESPONSE: // 表示已收到来自服务器的响应
                        EC600U_http_download_data_func((void *)&qhttpc_event_msg);
                        break;
                    case QHTTPC_EVENT_END: // 表示请求的全部处理已经完成QHTTPC_EVENT_END
                        flags_break = 1; // 退出循环标志为置1
                        break;
                    case QHTTPC_EVENT_FAILED:
                        this_ret = -6;
                        goto exit;
                        break;
                    default:
                        break;
                }
                // 如果退出标志为1，代表全部处理已经完成，退出该for循环
                if (flags_break)
                {
                    break;
                }
            }
        }
        else
        { // 关联失败
            EC600U_HTTP_LOG("http perform failed!!!!!!!!!!");
            this_ret = -5;
            goto exit;
        }
        // 重置下载状态变量
        memset(&dload_stat, 0x00, sizeof(struct stat));
        EC600U_HTTP_LOG("=========dload_file_size before:%d", dload_stat.st_size);
        // 根据文件id获取文件状态存到dload_stat
        ql_fstat(EC600U_test_http_demo_client.dload_fd, &dload_stat);
        EC600U_HTTP_LOG("=========dload_file_size after:%d", dload_stat.st_size);
        if (EC600U_test_http_demo_client.dload_fd >= 0)
        {
            ql_fclose(EC600U_test_http_demo_client.dload_fd); // 关闭下载文件
            EC600U_test_http_demo_client.dload_fd = -1;
        }
        // 释放http client上下文,以及http client上下文所占用内存块
        ql_httpc_release(&EC600U_test_http_demo_client.http_client);
        EC600U_test_http_demo_client.http_client = 0;

        // 当消息队列非空时
        if (EC600U_test_http_demo_client.queue != NULL)
        {
            while (1)
            {
                // 重置客户端信息结构体
                memset(&qhttpc_event_msg, 0x00, sizeof(ql_event_t));

                // 如果没等到消息队列，则退出循环
                if (QL_OSI_SUCCESS != ql_rtos_queue_wait(EC600U_test_http_demo_client.queue, (uint8 *)&qhttpc_event_msg, sizeof(ql_event_t), 0))
                    break;

                // 否则根据消息队列的id进行处理
                switch (qhttpc_event_msg.id)
                {
                case QHTTPC_EVENT_RESPONSE: {
                    free((void *)(qhttpc_event_msg.param2));
                }
                break;
                default:
                    break;
                }
            }
            // 删除消息队列
            ql_rtos_queue_delete(EC600U_test_http_demo_client.queue);
            EC600U_test_http_demo_client.queue = NULL;
        }
        // 删除互斥锁
        ql_rtos_mutex_delete(EC600U_test_http_demo_client.simple_lock);
        EC600U_test_http_demo_client.simple_lock = NULL;

        // 延时3s进入下一次循环
        ql_rtos_task_sleep_ms(10);
    }
    
exit:
    // 关闭文件
    if (EC600U_test_http_demo_client.dload_fd >= 0)
    {
        ql_fclose(EC600U_test_http_demo_client.dload_fd); 
        EC600U_test_http_demo_client.dload_fd = -1;
    }

    // 若消息队列非空
    if (EC600U_test_http_demo_client.queue != NULL)
    {
        while (1)
        {
            // 重置客户端信息结构体
            memset(&qhttpc_event_msg, 0x00, sizeof(ql_event_t));

            // 如果没等到消息队列，则退出循环
            if (QL_OSI_SUCCESS != ql_rtos_queue_wait(EC600U_test_http_demo_client.queue, (uint8 *)&qhttpc_event_msg, sizeof(ql_event_t), 0))
                break;

            // 否则根据消息队列的id进行处理
            switch (qhttpc_event_msg.id)
            {
            case QHTTPC_EVENT_RESPONSE: {
                free((void *)(qhttpc_event_msg.param2));
            }
            break;
            default:
                break;
            }
        }
        ql_rtos_queue_delete(EC600U_test_http_demo_client.queue);
    }
    
    if (EC600U_test_http_demo_client.simple_lock != NULL)
    {
        ql_rtos_mutex_delete(EC600U_test_http_demo_client.simple_lock);
    }

    if (this_ret == 0)
    {
        esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_DOWNLOADING, ESP32_FLASH_STATUS_FINISH, 0);
    }
    else 
    {
        esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_DOWNLOADING, ESP32_FLASH_STATUS_ERR, (uint16_t)this_ret);
    }
    return this_ret;
}
