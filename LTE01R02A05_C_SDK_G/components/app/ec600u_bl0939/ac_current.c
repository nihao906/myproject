
#include <stdint.h>
#include <string.h>
#include <ql_fs.h>
#include "osi_api.h"
#include "ql_api_osi.h"
#include "ql_api_spi.h"
#include "osi_compiler.h"
#include "../rtk/rtk_gps.h"
// #include "cfg.h"
//  #include "../EC600U_t2n/include/t2n.h"
#include "ql_log.h"
#include "ac_current.h"
#include "../t2n/include/ModbusS.h"
#include "../rtk/gps_coord.h"
#include "../t2n/include/BL0939_tcp.h"
#include "math.h"

uint8_t DATA_status;
uint8_t times;
uint64_t first_dosing_time = 0;
uint64_t end_hammer_time = 0;

uint32_t duration = 0;
uint32_t subtotal_time = 0;
uint16_t depth = 0;
uint16_t timeline = 0;

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "bl0939", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "bl0939", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "bl0939", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "bl0939", msg, ##__VA_ARGS__)

dosing_config_t *dosing = (dosing_config_t *)&gWordVar[REG_DOSING_DATA_CFG];
hammer_config_t *hammer = (hammer_config_t *)&gWordVar[REG_HAMMER_DATA_CFG];
total_config_t *work = (total_config_t *)&gWordVar[REG_WORK_DATA_CFG];
gps_message_t *gps_message = (gps_message_t *)&gWordVar[REG_GPS_DATA_CFG];
config_t *config = (config_t *)&gWordVar[REG_CONFIG_DATA_CFG];
// #define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_VERBOSE, "bl0939", msg, ##__VA_ARGS__)
// #define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_VERBOSE, "bl0939", msg, ##__VA_ARGS__)
// #define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "bl0939", msg, ##__VA_ARGS__)
// #define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "bl0939", msg, ##__VA_ARGS__)

int enc_value = 0;

// static ql_task_t *current_task_handle = NULL;

#define BL0939_READ_TIMER 0x8000

osiThread_t *bl_thread_handle = NULL;
osiTimer_t *bl_timer = NULL;
static int spi_wait_write_read = 0;
ql_sem_t spi_write_semaphore;
ql_sem_t spi_read_semaphore;

#define QL_SPI_WAIT_NONE 0
#define QL_SPI_WAIT_WRITE 1
#define QL_SPI_WAIT_READ 2

extern uint16_t gWordVar[];
static osiSemaphore_t *data_sema = NULL;
static osiSemaphore_t *hammer_sema = NULL;
static osiSemaphore_t *work_sema = NULL;
static const uint8_t bl0939_cmd[36] =
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x55, 0x00, 0, 0, 0, 0,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x55, 0x07, 0, 0, 0, 0,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x55, 0x06, 0, 0, 0, 0};

static uint8_t spi_txbuf[36] OSI_CACHE_LINE_ALIGNED;
static uint8_t spi_rxbuf[36] OSI_CACHE_LINE_ALIGNED;

static void spi_cb_handler(ql_spi_irq_s cause)
{
    if (cause.rx_dma_done == 1 && spi_wait_write_read == QL_SPI_WAIT_READ)
    {
        spi_wait_write_read = QL_SPI_WAIT_NONE;
        ql_rtos_semaphore_release(spi_read_semaphore);
    }
    if (cause.tx_dma_done == 1 && spi_wait_write_read == QL_SPI_WAIT_WRITE)
    {
        spi_wait_write_read = QL_SPI_WAIT_NONE;
        ql_rtos_semaphore_release(spi_write_semaphore);
    }
    LOGD("spi_cb_handler rx_dma_done=%d tx_dma_done=%d\r\n", cause.rx_dma_done, cause.tx_dma_done);
}

static int spi_write(int fd, uint8_t *inbuf, int len)
{
    int ret;
    spi_wait_write_read = QL_SPI_WAIT_WRITE;
    ql_spi_request_sys_clk(fd);
    ret = ql_spi_write(fd, inbuf, len);
    ql_rtos_semaphore_wait(spi_write_semaphore, 10);
    LOGD("spi_write inbuf=%p,len=%d,ret=%d", inbuf, len, ret);
    return ret;
}

static int spi_write_read(int fd, uint8_t *outbuf, uint8_t *inbuf, int len)
{
    int ret;
    spi_wait_write_read = QL_SPI_WAIT_READ;
    ret = ql_spi_write_read(fd, inbuf, outbuf, len);
    ql_rtos_semaphore_wait(spi_read_semaphore, 10);
    LOGD("spi_write_read inbuf=%p,outbuf=%p,len=%d,ret=%d", inbuf, outbuf, len, ret);
    return ret;
}

static int check_sum_ch(uint8_t *resp, uint8_t *cmd, int offset)
{
    uint8_t sum = ~(0x55 + cmd[offset - 1] + resp[offset] + resp[offset + 1] + resp[offset + 2]);
    if (resp[offset + 3] == sum)
    {
        return 1;
    }
    return 0;
}
uint32_t readUint24LE(uint8_t *buf, int offset)
{
    return buf[offset] | buf[offset + 1] << 8 | buf[offset + 2] << 16;
}
uint32_t readUint24BE(uint8_t *buf, int offset)
{
    return buf[offset] << 16 | buf[offset + 1] << 8 | buf[offset + 2];
}

float ac_current_coef[3] = {
    1.218f * 1.4 / (324004.f * 1 * 1000 / 1000) * 100.f,
    1.218f * 1.4 / (324004.f * 1 * 1000 / 1000) * 100.f,
    // 1.218f / (79931.f * 0.002 * 500) * 100.f
    1.218f * 2 / (324004.f * 1 * 1000 / 1000) * 100.f}; // 电流转换系数，单位mA，（测得实际电流与转换电流不一致，所以更改系数，系数计算参考bl0939手册）

// uint8_t bl0939_rxbuf[36];
int32_t ac_ad_values[3] = {0};

static void bl_read_data_timer(void *parm)
{
    osiTimerStart(bl_timer, 20);
    ql_event_t event;
    event.id = BL0939_READ_TIMER;
    if (bl_thread_handle)
    {
        ql_rtos_event_send(bl_thread_handle, &event);
    }
    // LOGD("bl_read_data_timer\r\n");
}

static uint32_t bl0939_read_reg(int fd, uint8_t reg)
{
    spi_txbuf[0] = 0x55;
    spi_txbuf[1] = reg;
    spi_txbuf[2] = 0;
    spi_txbuf[3] = 0;
    spi_txbuf[4] = 0;
    spi_txbuf[5] = 0;
    //	bl0939_spi_reset();
    spi_write_read(fd, spi_txbuf, spi_rxbuf, 6);
    spi_wait_write_read = QL_SPI_WAIT_READ;
    ql_rtos_semaphore_wait(spi_read_semaphore, 10);
    if (spi_rxbuf[5] == (uint8_t) ~(0x55 + reg + spi_rxbuf[2] + spi_rxbuf[3] + spi_rxbuf[4]))
    {
        return (uint32_t)spi_rxbuf[2] << 16 |
               (uint32_t)spi_rxbuf[3] << 8 |
               (uint32_t)spi_rxbuf[4] << 0;
    }
    return 0;
}
void bl0939_spi_reset(int fd)
{
    spi_txbuf[0] = 0xff;
    spi_txbuf[1] = 0xff;
    spi_txbuf[2] = 0xff;
    spi_txbuf[3] = 0xff;
    spi_txbuf[4] = 0xff;
    spi_txbuf[5] = 0xff;
    spi_write(fd, spi_txbuf, 6);
}
uint32_t r_temp = 0;
static int bl0939_write_reg(int fd, uint8_t reg, uint32_t val, int check)
{
    int try_times = 0;
    uint8_t h = val >> 16;
    uint8_t m = val >> 8;
    uint8_t l = val >> 0;
    do
    {
        // bl0939_spi_reset();
        spi_txbuf[0] = 0xA5;
        spi_txbuf[1] = reg;
        spi_txbuf[2] = h;
        spi_txbuf[3] = m;
        spi_txbuf[4] = l;
        spi_txbuf[5] = ~(0XA5 + reg + h + m + l);
        spi_write_read(fd, spi_txbuf, spi_rxbuf, 6);
        ql_rtos_semaphore_wait(spi_read_semaphore, 10);
        if (0 == check)
            return 0;
        r_temp = bl0939_read_reg(fd, reg);
        if (r_temp == val)
            return 0;
        ql_rtos_task_sleep_ms(100);
    } while (try_times++ < 5);
    return 1;
}

void bl0939_init(int fd)
{
    bl0939_spi_reset(fd);
    bl0939_write_reg(fd, 0x19, 0x005a5a5a, 0); // ��λ�û��Ĵ���
    bl0939_write_reg(fd, 0x1a, 0x00000055, 1); // ���д����
    bl0939_write_reg(fd, 0x10, 0xffff, 0);     // Threshold A
    bl0939_write_reg(fd, 0x1E, 0xffff, 1);     // Threshold B
    bl0939_write_reg(fd, 0x18, 0x00002000, 1); // cf
    bl0939_write_reg(fd, 0x1B, 0x000047ff, 0); // cf
    bl0939_write_reg(fd, 0x1a, 0x00000000, 1); // д����
}

void get_gps_xy(void);
void bl0939_data_init(void)
{
    dosing->current = 0;
    dosing->total_time = 0;
    dosing->work_time = 0;
    dosing->stop_time = 0;
    dosing->cnt = 0;
    config->dosing_up = 200;
    config->dosing_down = 50;
    config->dosing_con_time = 60;
    dosing->walk_state = STOP;

    hammer->current = 0;
    hammer->total_time = 0;
    hammer->work_time = 0;
    hammer->up_time = 0;
    hammer->stop_time = 0;
    hammer->cnt = 0;
    config->hammer_up = 500;
    config->hammer_down = 50;
    config->hammer_start = 300;
    config->hammer_con_time = 60;
    hammer->walk_state = STOP;

    work->work_current = 0;
    work->id = 0;
    work->total_time = 0;
    work->state = STOP;
    config->work_up = 200;
    config->work_down = 50;
    config->work_con_time = 100;
    config->work_total_con_time = 100 * 50;
    config->coefficient_down = 90;
    config->coefficient_up = 110;
}
void bl0939_task(void)
{

    int ret;
    ql_spi_config_s spi_config = {0};
    int spi_no = QL_SPI_PORT1;
    spi_config.port = spi_no;
    spi_config.spiclk = QL_SPI_CLK_1_5625MHZ;
    spi_config.framesize = 8;
    spi_config.input_mode = QL_SPI_INPUT_TRUE;
    spi_config.cs_polarity0 = QL_SPI_CS_ACTIVE_LOW;
    spi_config.cs_polarity1 = QL_SPI_CS_ACTIVE_LOW;
    spi_config.cpol = QL_SPI_CPOL_LOW;
    spi_config.cpha = QL_SPI_CPHA_2Edge; // mode 1
    spi_config.input_sel = QL_SPI_DI_1;
    spi_config.transmode = QL_SPI_DMA_IRQ;
    spi_config.cs = QL_SPI_CS0;
    spi_config.clk_delay = QL_SPI_CLK_DELAY_0;
    LOGD("bl0939_task entry");
    ql_rtos_task_sleep_ms(5000);
    ql_spi_init_ext(spi_config);
    ql_spi_cs_auto(spi_no);

    // uint32_t addr = (uint32_t)__spi_dma_out_buf;
    // addr += 32 - addr % 32;
    // spi_txbuf = (uint8_t *)(addr);
    // LOGD("spi_txbuf=%p",spi_txbuf);
    // addr = (uint32_t)__spi_dma_in_buf;
    // addr += 32 - addr % 32;
    // spi_rxbuf = (uint8_t *)(addr);
    // LOGD("spi_rxbuf=%p", spi_rxbuf);
    // LOGD("ql_spi_init done");
    ql_rtos_semaphore_create(&spi_read_semaphore, 0);
    ql_rtos_semaphore_create(&spi_write_semaphore, 0);
    LOGD("semaphore create done");
    ql_spi_irq_s mask = {
        .rx_dma_done = 1,
        .tx_dma_done = 1,
    };

    ql_spi_set_irq(spi_no, mask, spi_cb_handler);
    LOGD("ql_spi_set_irq done");
    bl0939_init(spi_no);
    bl_thread_handle = osiThreadCurrent();
    bl_timer = osiTimerCreate(bl_thread_handle, bl_read_data_timer, (void *)spi_no); // 创建定时器
    if (NULL == bl_timer)
    {
        LOGE("bl_timer create failed\n");
        osiThreadExit();
    }
    ret = osiTimerStart(bl_timer, 20);
    if (ret)
    {
        LOGE("osiTimerStart failed ret=%d\n", ret);
    }
    LOGD("bl0939_task init done");

    while (1)
    {
        ql_event_t event;
        ret = ql_event_wait(&event, 1000);
        if (ret == 0)
        {
            switch (event.id)
            {
            case BL0939_READ_TIMER:
                // LOGD("bl0939_task while");
                memcpy(spi_txbuf, bl0939_cmd, sizeof(bl0939_cmd));
                ret = spi_write_read(spi_no, spi_txbuf, spi_rxbuf, 36);
                if (ret == 0)
                {
                    for (int ch = 0; ch < 3; ch++)
                    {
                        if (check_sum_ch(spi_rxbuf, spi_txbuf, 8 + ch * 12))
                        {
                            ac_ad_values[ch] = readUint24BE(spi_rxbuf, 8 + ch * 12);
                            // LOGI("channel : %d   ac_ad_values: %ld\n ", ch, ac_ad_values[ch]);
                            gWordVar[AC_CURRENT_REG_ADDR + ch] = ac_ad_values[ch] * ac_current_coef[ch];
                            LOGI("channel : %d   valued: %d", ch, gWordVar[AC_CURRENT_REG_ADDR + ch]);
                        }
                        else
                        {
                            int value = readUint24BE(spi_rxbuf, 8 + ch * 12);
                            LOGW("check bed ch: %d   ac_ad_values: %d\n ", ch, value);
                        }
                    }
                }
                else
                {
                    LOGE("spi_write_read err ret=%d\r\n", ret);
                }
                osiSemaphoreRelease(data_sema);
                // osiSemaphoreRelease(hammer_sema);
                // osiSemaphoreRelease(work_sema);
                break;
            }
        }
        else
        {
            LOGE("timer not work ??", ret);
            osiTimerStart(bl_timer, 20);
        }
    }
}
int ham_count = 0;
int ham_flag = 0;
uint16_t currrent[150];
uint16_t hammer_average = 0;
uint32_t sum_hammmer_current = 0;
int h_i = 0;
void get_hammer_current_average(void)
{

    ham_count++;
    if (ham_count < 150)
    {
        currrent[ham_count] = gWordVar[AC_CURRENT_REG_ADDR + 1];
    }
    if (ham_count == 150)
    {
        ham_count = 0;
        ham_flag = 1;
    }
    if (ham_flag == 0)
    {
        sum_hammmer_current += currrent[ham_count];
        hammer_average = sum_hammmer_current / ham_count;
    }
    if (ham_flag == 1)
    {
        for (h_i = 0; h_i < 150; h_i++)
        {
            sum_hammmer_current += currrent[h_i];
            if (h_i == 149)
            {
                hammer_average = sum_hammmer_current / 150;
                sum_hammmer_current = 0;
            }
        }
    }
}

void get_bl0939_data_task(void)
{

    uint64_t last_dosing_work_time = 0;
    uint64_t last_dosing_stop_time = 0;
    uint64_t last_hammer_work_time = 0;
    uint64_t last_hammer_up_time = 0;
    uint64_t last_hammer_stop_time = 0;

    uint64_t dising_start_time = 0;
    uint64_t dising_end_time = 0;

    uint64_t hammer_start_time = 0;
    uint64_t hammer_up_time = 0;
    uint64_t hammer_end_time = 0;

    uint64_t time_work = 0;

    uint64_t last_work_time = 0;
    uint32_t error_time = 0;
    uint32_t last_error_time = 0;
    uint64_t start_error_time = 0;
    uint64_t hammer_time = 0;
    uint64_t last_hammer_st_time = 0;
    uint64_t hammer_st_time = 0;
    uint64_t hammer_et_time = 0;
    uint64_t last_hammer_time = 0;
    uint64_t hammer_cou = 0;
    uint64_t error_cou = 0;

    int dosing_flag = 0;
    int hammer_flag = 0;
    int hammer_flag_start = 0;
    int hammer_flag_stop = 0;

    ql_timeval_t time;
    ql_timeval_t H_time;
    ql_timeval_t D_time;
    ql_timeval_t ST_time;
    ql_timeval_t ET_time;

    uint32_t dos_contime = 0;
    uint32_t dos_stop_contime = 0;
    uint32_t ham_contime = 0;
    uint32_t ham_stop_contime = 0;

    uint32_t sum_hammmer_current = 0;
    int hammer_count = 0;
    uint16_t hammer_con_current = 0;
    int32_t hammer_average_up;
    int32_t hammer_average_down;

    // 如果无人发送，三十秒发一次 0
    // 五种状态，01234   0：空闲 1：加料开 2：加料停 3：夯击开始 4：夯击关闭

    while (1)
    {
        if (osiSemaphoreTryAcquire(data_sema, 500))
        {
            ql_gettimeofday(&ST_time);
            start_error_time = (uint64_t)ST_time.sec * 1000 + ST_time.usec / 1000;

            if(now == 0)
            {
                now = start_error_time;
            }
            error_time = (start_error_time - now);
            LOGI("error_time: %d", error_time);
            last_error_time = error_time;
            if(last_error_time >= 30000)
            {
                LOGI("text report 1");
                DATA_status = 0;
                // report_process_data();
                now = start_error_time;
            }
            // LOGI("text report 0");


            dosing->current = gWordVar[AC_CURRENT_REG_ADDR + 0];

            // 根据电流判断加料持续多长时间，不加料保持时间
            if(dosing->current > config->dosing_up) // 加料开始
            {
                ql_gettimeofday(&time);
                dising_start_time = (uint64_t)time.sec * 1000 + time.usec / 1000;
                // LOGI("dosing start_time: %d", dosing.start_time);
                // LOGI("last_dosing_work_time: %d", last_dosing_work_time);
                if(last_dosing_work_time == 0)
                {
                    dosing->work_time = 0;
                    first_dosing_time = dising_start_time;
                }
                else
                {
                    dosing->work_time += (dising_start_time - last_dosing_work_time); // 加料的累计时间
                    dos_contime += (dising_start_time - last_dosing_work_time);
                }
                if(dosing_flag == 1 && dos_contime > config->dosing_con_time )
                {
                    LOGI("text report 2");
                    hammer_time = 0;
                    hammer->cnt = 0;

                    dosing_flag = 0;
                    dos_contime = 0;
                    // LOGI("dosing cnt: %d", dosing->cnt);
                    DATA_status = 1;
                    times = (uint8_t)dosing->cnt;
                    duration = dosing->work_time;
                    subtotal_time = work->total_time;
                    report_process_data();
                    dosing->cnt++;
                }
                LOGI("dosing WORK_time: %d", dosing->work_time);
                dosing->walk_state = STOP;
                dosing_flag = 2;
                last_dosing_work_time = dising_start_time;
            }
            if(dosing->current < config->dosing_down) // 加料停止
            {
                // memset(&time, 0, sizeof(ql_timeval_t));
                ql_gettimeofday(&time);
                dising_end_time = (uint64_t)time.sec * 1000 + time.usec / 1000;

                if(last_dosing_stop_time == 0)
                {
                    dosing->stop_time = 0;
                }
                else
                {
                    dosing->stop_time += (dising_end_time - last_dosing_stop_time);
                    dos_contime += (dising_start_time - last_dosing_work_time);
                }
                LOGI("dosing stop_time: %d", dosing->stop_time);
                last_dosing_stop_time = dising_end_time;
                if(dosing_flag == 2 && dos_contime > config->dosing_con_time)
                {
                    dosing_flag = 0;
                    // dos_contime = 0;
                    LOGI("text report 3");
                    DATA_status = 2;
                    times = (uint8_t)dosing->cnt;
                    duration = dosing->work_time;
                    subtotal_time = work->total_time;
                    report_process_data();
                }
                if(dos_contime > 1000)
                {
                    dos_contime = 0;
                    dosing_flag = 1;
                }
                dosing->walk_state = WORK;
                
            }
            // }
            // if (osiSemaphoreTryAcquire(hammer_sema, 500))
            // {
            hammer->current = gWordVar[AC_CURRENT_REG_ADDR + 1];
#if 1
            if(hammer->current > config->hammer_start) // 夯击电机开启
            {
                ql_gettimeofday(&H_time);
                hammer_start_time = (uint64_t)H_time.sec * 1000 + H_time.usec / 1000;
                end_hammer_time = hammer_start_time;
                if(last_hammer_work_time == 0)
                {
                    hammer->work_time = 0;
                }
                if(last_hammer_work_time != 0)
                {
                    hammer->work_time += (hammer_start_time - last_hammer_work_time);
                }

                get_hammer_current_average();
                LOGI("hammer_average: %d", hammer_average);
                hammer_average_up = (hammer_average * config->coefficient_up) / 100;
                hammer_average_down = (hammer_average * config->coefficient_down) / 100;

                if(hammer_flag_stop == 1)
                {
                    hammer_flag_stop = 0;
                    DATA_status = 3;
                    times = (uint8_t)hammer->cnt;
                    duration = hammer->work_time;
                    subtotal_time = work->total_time;
                    report_process_data();
                    LOGI("text report 4");
                }

                if(hammer->current > hammer_average_up)
                {
                    hammer_flag = 1;
                    if(hammer_flag_start == 2 && hammer_flag == 1)
                    {
                        ql_gettimeofday(&ET_time);
                        hammer_st_time = (uint64_t)ET_time.sec * 1000 + ET_time.usec / 1000;

                        hammer->cnt++;
                        hammer_flag = 0;
                        hammer_flag_start = 0;
                        ham_contime = 0;
                        hammer_time = 0;

                        // DATA_status = 3;
                        // times = (uint8_t)hammer->cnt;
                        // duration = hammer->work_time;
                        // subtotal_time = work->total_time;
                        // report_process_data();
                    }
                }
                if(hammer->current < hammer_average_down)
                {
                    hammer_flag_start = 2;
                }
                last_hammer_work_time = hammer_start_time;
                hammer->walk_state = STOP;
                hammer_flag_stop = 2;
                // 超时判断

                if(hammer_st_time == 0)
                {
                    ql_gettimeofday(&ET_time);
                    hammer_et_time = (uint64_t)ET_time.sec * 1000 + ET_time.usec / 1000;
                    if(last_hammer_time == 0)
                    {
                        hammer_time = 0;
                    }
                    else
                    {
                        hammer_time += (hammer_et_time - last_hammer_time);
                    }
                }

                last_hammer_time = hammer_et_time;

                if(hammer_st_time != 0)
                {

                    hammer_time = (start_error_time - hammer_st_time);
                }
                if(hammer_time > 2000)
                {
                    LOGI("hammer 1");
                    hammer_cou += (hammer_time - last_hammer_st_time);
                    if(hammer_cou > 1000)
                    {
                        hammer_cou = 0;
                        LOGI("hammer 2");

                        hammer->cnt++;
                        // report_process_data();
                    }
                    last_hammer_st_time = hammer_time;
                }

                // if(hammer_time < 2000)
                // {
                //     hammer_time = 0;
                // }
                LOGI("last_hammer_st_time : %d", last_hammer_st_time);
                LOGI("hammer_cou : %d", hammer_cou);
                LOGI("hammer_CNT : %d", hammer->cnt);
                LOGI("hammer_time : %d", hammer_time);
                LOGI("last_hammer_time : %d", last_hammer_time);
                LOGI("hammer_st_time : %d", hammer_st_time);
                // last_hammer_time = hammer_et_time;
            }

            if(hammer->current < config->hammer_down) // 夯击电机停止
            {
                ql_gettimeofday(&H_time);
                hammer_end_time = (uint64_t)H_time.sec * 1000 + H_time.usec / 1000;
                // LOGI("hammer.end_time : %d", hammer->end_time);
                // LOGI("Last hammer end time : %d",last_dosing_stop_time);

                if(last_hammer_stop_time == 0)
                {
                    hammer->stop_time = 0;
                }
                else
                {
                    hammer->stop_time += (hammer_end_time - last_hammer_stop_time);
                }
                LOGI("hammer.stop_time : %d", hammer->stop_time);
                last_hammer_stop_time = hammer_end_time;
                if(hammer_flag_stop == 2)
                {
                    LOGI("text report 5");
                    hammer_flag_stop = 0;
                    // hammer_flag_stop = 0;
                    DATA_status = 3;
                    times = (uint8_t)hammer->cnt;
                    duration = hammer->work_time;
                    subtotal_time = work->total_time;
                    report_process_data();
                    
                }
                hammer->walk_state = WORK;
                hammer_flag_stop = 1;

                // sum_hammmer_current = 0;
                // hammer_count = 0;
                // hammer_con_current = 0;
            }
            LOGI("hammer_flag_stop : %d", hammer_flag_stop);
#endif
            // // 当吊锤上升时，根据电流判断吊锤开启与上升，记录总时间
            // if (hammer->current > config->hammer_start && hammer->current < config->hammer_up) // 吊锤开启
            // {
            //     ql_gettimeofday(&H_time);
            //     hammer_start_time = (uint64_t)H_time.sec * 1000 + H_time.usec / 1000;
            //     if (last_hammer_work_time == 0)
            //     {
            //         hammer->work_time = 0;
            //     }
            //     else
            //     {
            //         hammer->work_time += (hammer_start_time - last_hammer_work_time);
            //     }
            //     last_hammer_work_time = hammer_start_time;

            //     hammer_flag_start = 2;
            //     hammer->walk_state = STOP;
            // }
            // if (hammer->current > config->hammer_up) // 吊锤上升
            // {
            //     ql_gettimeofday(&H_time);
            //     hammer_up_time = (uint64_t)H_time.sec * 1000 + H_time.usec / 1000;
            //     end_hammer_time = hammer_up_time;
            //     if (last_hammer_up_time == 0)
            //     {
            //         hammer->up_time = 0;
            //     }
            //     else
            //     {
            //         hammer->up_time += (hammer_up_time - last_hammer_up_time);
            //         ham_contime += (hammer_up_time - last_hammer_up_time);
            //     }
            //     LOGI("hammer.up_time : %d", hammer->up_time);
            //     // ql_rtos_task_sleep_ms(10);
            //     last_hammer_up_time = hammer_up_time;

            //     if (hammer_flag_start == 2 && hammer_flag == 1 && ham_contime > config->hammer_con_time)
            //     {
            //         hammer->cnt++;
            //         hammer_flag = 0;
            //         hammer_flag_start = 0;
            //         ham_contime = 0;

            //         DATA_status = 2;
            //         times = (uint8_t)hammer->cnt;
            //         duration = hammer->total_time;
            //         subtotal_time = work->total_time;
            //         report_process_data();
            //     }
            //     hammer->walk_state = STOP;
            // }
            // if (hammer->current < config->hammer_down) // 吊锤下降砸地
            // {
            //     ql_gettimeofday(&H_time);
            //     hammer_end_time = (uint64_t)H_time.sec * 1000 + H_time.usec / 1000;
            //     // LOGI("hammer.end_time : %d", hammer->end_time);
            //     // LOGI("Last hammer end time : %d",last_dosing_stop_time);
            //     if (last_hammer_stop_time == 0)
            //     {
            //         hammer->stop_time = 0;
            //     }
            //     else
            //     {
            //         hammer->stop_time += (hammer_end_time - last_hammer_stop_time);
            //     }
            //     LOGI("hammer.stop_time : %d", hammer->stop_time);
            //     last_hammer_stop_time = hammer_end_time;
            //     if (hammer->stop_time > config->hammer_con_time)
            //     {
            //         hammer->walk_state = WORK;
            //         hammer_flag = 1;
            //     }
            // }
            // // }

            // if (osiSemaphoreTryAcquire(work_sema, 500))
            // {
            work->work_current = gWordVar[AC_CURRENT_REG_ADDR + 2];
            if (work->work_current > config->work_up)
            {
                // if (dosing->walk_state == WORK && hammer->walk_state == WORK)
                // {
                ql_gettimeofday(&D_time);
                time_work = (uint64_t)D_time.sec * 1000 + D_time.usec / 1000;

                if (last_work_time == 0)
                {
                    work->work_time = 0;
                }
                else
                {
                    work->work_time += (time_work - last_work_time); // xingzou的累计时间
                }
                LOGI("work.work_time : %d", work->work_time);
                if (hammer->walk_state == STOP || dosing->walk_state == STOP)
                {
                    work->work_time = 0;
                }
                last_work_time = time_work;
            }

            //     // }
            // }
        }
        dosing->total_time = (dosing->work_time / 20);
        LOGI("dosing total_time: %d", dosing->total_time);
        hammer->total_time = (hammer->work_time / 20);
        LOGI("hammer.total_time : %d", hammer->total_time);
        work->total_time = ((end_hammer_time - first_dosing_time) / 20);
        LOGI("work.total_time : %d", work->total_time);

        // LOGI("STATUE : %d", DATA_status);
        // LOGI("TIMES : %d", times);
        // 总时间=加料开始停止锤结束
        // }
        // }
    }
}

double gps_xy[2];
int none_flag = 1;
static double double_abs(double a, double b)
{
    if (a > b)
        return a - b;
    else
        return b - a;
}

int gps_distance_check(void)
{
    if (gps_message->gps_status != 4) // gps信号不是精准定位则不启用
    {
        none_flag = 1;
        return 0;
    }
    if (none_flag)
        return 0;
    if (double_abs(gps_message->x, gps_xy[0]) > GPS_DISTANCE_TH)
        return 1;
    if (double_abs(gps_message->y, gps_xy[1]) > GPS_DISTANCE_TH)
        return 1;
    return 0;
}
void hammer_cnt_check(void)
{
    int hammer_mincnt = 0;
    int hammer_maxcnt = 0;
    hammer_mincnt = hammer->total_time / 25;
    hammer_maxcnt = hammer->total_time / 100;
    if (hammer->cnt <= hammer_mincnt || hammer->cnt >= hammer_maxcnt)
    {
        hammer->cnt = hammer->total_time / 45;
    }
}
struct Point res_point;
void inc_id_funcation(void)
{
    int err = 0;
    // 增加桩号
    if (hammer->total_time > 500 || dosing->cnt > 10)
    {
        work->id++;
    }
    // work->id++;
    // LOGI("work->id: %d", work->id);
    err = ql_nvm_fwrite("work->id", &work->id, sizeof(work->id), 1);

    gps_xy[0] = res_point.x;
    gps_xy[1] = res_point.y;

    // gps_xy[0] = gps_message->x;
    // gps_xy[1] = gps_message->y;

    none_flag = 0;
    // 清空数据
    dosing->total_time = 0;
    dosing->work_time = 0;
    dosing->stop_time = 0;
    dosing->cnt = 0;
    hammer->total_time = 0;
    hammer->work_time = 0;
    hammer->up_time = 0;
    hammer->stop_time = 0;
    hammer->cnt = 0;

    work->work_time = 0;

    DATA_status = 0;
    times = 0;
}
// 换桩逻辑
// 发送信号改成一个，信号改成持续的，并且在数据刷新之后再去发送，三个二十毫秒才算有效，结果再去做逻辑判断,对于??状态的持续的判断值需要能存
// 行走电机开，持续时间，断断续续开，每次开持续两秒，，累计时间大于五秒或者位置大于五十厘米，夯锤一开行走电机累计时间清零
// 一段时间gps，两种状态的gps，次数有多少点加进去，大于大于五十厘米，连续的三四个点
// 判断位置要判断上次干过，标志位，桩的位置求平均值，在gps刷新数据时地方判断一秒钟，连续五个点都判断
// 桩号增加：如果夯击累计时间大于十秒，加料也得开几次，然后桩号增加
// 根据经纬度连续五次判断gps有没有移动，移动则换桩
double res_point_x = 0;
double res_point_y = 0;
double point_x = 0;
double point_y = 0;
uint8_t status = 0;
uint32_t utc = 0;
float pitch = 0;
int32_t height = 0;
void get_gps_xy(void)
{
    point_x = result_point.x;
    point_y = result_point.y;
    res_point_x = target_res_point.x;
    res_point_y = target_res_point.y;
    status = gps1_parse.Q;
    utc = gps1_parse.UTC;
    pitch = gps1_parse.Heading;
    height = gps1_parse.ALT;

    gps_message->x = res_point_x; // 经纬度
    gps_message->y = res_point_y;
    gps_message->x_res = point_x; // 直角
    gps_message->y_res = point_y;

    gps_message->gps_status = status; // gps状态
    gps_message->utc = utc;           // utc时间
    gps_message->pitch = pitch;
    gps_message->height = height;

    //     LOGI("gps_message->x: %f, gps_message->y: %f", gps_message->x, gps_message->y);
    //     LOGI("gps_message->x_res: %f gps_message->y_res: %f", gps_message->x_res, gps_message->y_res);
    //     LOGI("gps1_parse.Q: %d", gps1_parse.Q);
    //     LOGI("gps_message->gps_status: %d", gps_message->gps_status);
    // LOGI("gps_message->height: %d", gps1_parse.ALT);
}

int gps_move_flag = 0;
// 计算两点之间的距离
bool getGreatCircleDistance(double startLng, double startLat, double endLng, double endLat)
{
    double distance = sqrt(((startLng - endLng) * (startLng - endLng)) + ((startLat - endLat) * (startLat - endLat)));
    LOGI("distance: %f", distance);
    if (distance > 1)
    {
        return true;
    }
    return false;
}
int gps_count = 0;

void judg_location(void)
{
    struct Point point;

    // for (int i = 0; i < 5; i++)
    // {
    point.x += result_point.x;
    point.y += result_point.y;

    // res_point.x = point.x / i;
    // res_point.y = point.y / i;

    gps_count++;
    // LOGI("gps_count: %d", gps_count);
    // }
    if (gps_xy[0] == 0 && gps_xy[1] == 0)
    {
        gps_xy[0] = result_point.x;
        gps_xy[1] = result_point.y;
        gps_move_flag = 0;
    }
    else if (gps_count >= 5 && gps1_parse.Q == 4)
    {
        res_point.x = point.x / gps_count;
        res_point.y = point.y / gps_count;

        if (getGreatCircleDistance(gps_xy[0], gps_xy[1], res_point.x, res_point.y))
        {
            gps_move_flag = 1;
        }
        else
        {
            gps_move_flag = 0;
        }
        gps_count = 0;
    }
}
void work_state_task(void)
{
    // get_gps_xy();
    static uint16_t last_work_state = STOP;
    while (1)
    {

        // // get_gps_xy();
        if (last_work_state == STOP) // 判断电机启动
        {

            if (gWordVar[AC_CURRENT_REG_ADDR + 2] > config->work_up && dosing->walk_state == WORK && hammer->walk_state == WORK)
            {
                if (work->work_time > config->work_con_time)
                {
                    work->state = WORK;
                }
            }
        }

        if (last_work_state == WORK) // 判断电机停止
        {
            if (gWordVar[AC_CURRENT_REG_ADDR + 2] < config->work_down)
            {
                work->state = STOP;
            }
        }

        if ((work->state == WORK && last_work_state == STOP) || gps_move_flag == 1) // 判断换桩
        {
            // LOGI("GPS_move_flag: %d", gps_move_flag);
            if (work->work_time > config->work_total_con_time)
            {
                // hammer_cnt_check();
                report_pile_data();
                inc_id_funcation();
                // LOGI("work->state: %x", work->state);
                LOGI("huanzhuang success");
            }
        }

        last_work_state = work->state;
        // ql_rtos_task_sleep_ms(100);
        // LOGI("work->state: %x", work->state);
        // LOGI("last_work_state: %x", last_work_state);
        // LOGI("dosing->work_state: %x", dosing->walk_state);
        // LOGI("hammer->walk_state: %x", hammer->walk_state);
    }
}
void bl0939_task_init(void)
{
    int ret = 0;
    int ret0 = 0;
    QlOSStatus err = 0;
    data_sema = osiSemaphoreCreate(1, 0);
    hammer_sema = osiSemaphoreCreate(1, 0);
    work_sema = osiSemaphoreCreate(1, 0);
    LOGI("creat bl0939_task init!");
    bl0939_data_init();

    ret = ql_nvm_fread("work->id", &work->id, sizeof(work->id), 1);
    if (ret != sizeof(work->id))
    {
        LOGI("read work->id fail");
        work->id = 0;
    }
    ret0 = ql_nvm_fread("current_cfg", config, sizeof(config_t), 1);
    if (ret0 != sizeof(config_t) || config->dosing_up > 3000 || config->dosing_down > 3000 || config->hammer_start > 3000 || config->hammer_up > 3000 || config->hammer_down > 3000 || config->work_down > 3000 ||
        config->work_up > 3000 || config->dosing_con_time > 5000 || config->hammer_con_time > 5000 || config->work_con_time > 5000 || config->work_total_con_time > 5000 || config->coefficient_down > 200 || config->coefficient_up > 200)
    {
        LOGI("read current_cfg fail");
        config->dosing_up = 300;
        config->dosing_down = 100;
        config->hammer_up = 500;
        config->hammer_down = 200;
        config->hammer_start = 300;
        config->work_up = 200;
        config->work_down = 50;
        config->dosing_con_time = 60;
        config->hammer_con_time = 60;
        config->work_con_time = 100;
        config->work_total_con_time = 100 * 50;
        config->coefficient_down = 90;
        config->coefficient_up = 110;
    }

    ql_task_t bl0939_task_ref;
    err = ql_rtos_task_create(&bl0939_task_ref, 8 * 1024, APP_PRIORITY_BELOW_NORMAL, "bl0939", bl0939_task, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        LOGI("creat bl0939_task fail err = %d", err);
    }
    LOGI("creat bl0939_task sucess!");

    ql_task_t get_bl0939_data_ref;
    err = ql_rtos_task_create(&get_bl0939_data_ref, 4096, APP_PRIORITY_BELOW_NORMAL, "bl0939_data", get_bl0939_data_task, NULL, 4);
    if (err != QL_OSI_SUCCESS)
    {
        LOGI("creat get_bl0939_data_task fail err = %d", err);
    }
    LOGI("creat get_bl0939_data_task sucess!");

    ql_task_t work_task_ref;
    err = ql_rtos_task_create(&work_task_ref, 4096, APP_PRIORITY_BELOW_NORMAL, "work_state", work_state_task, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        LOGI("creat work_state_task fail err = %d", err);
    }
    LOGI("creat work_state_task sucess!");
}
