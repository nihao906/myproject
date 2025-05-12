
#include <stdint.h>
#include <string.h>
#include "osi_api.h"
#include "ql_api_osi.h"

#include "ql_api_spi.h"
#include "mcp2515.h"
// #include "hw.h"
// #include "cfg.h"
#include "../t2n/include/t2n.h"
#include "ql_log.h"
#include "ac_current.h"

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "can", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "can", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "can", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "can", msg, ##__VA_ARGS__)
#define QL_SPI_WAIT_NONE 0
#define QL_SPI_WAIT_WRITE 1
#define QL_SPI_WAIT_READ 2
osiThread_t *can_task_handle = NULL;

extern uint16_t gWordVar[];

struct can_status *can_status[2] = {(struct can_status *)&gWordVar[612], (struct can_status *)&gWordVar[620]};
// struct can_dev_cfg *can_dev_cfg = (struct can_dev_cfg *)&gWordVar[0x1000];

// ql_sem_t spi_write_semaphore;
// ql_sem_t spi_read_semaphore;
// static int spi_wait_write_read;

// static void spi_cb_handler(ql_spi_irq_s cause)
// {
//     if (cause.rx_dma_done == 1 && spi_wait_write_read == QL_SPI_WAIT_READ)
//     {
//         spi_wait_write_read = 0;
//         ql_rtos_semaphore_release(spi_read_semaphore);
//     }
//     if (cause.tx_dma_done == 1 && spi_wait_write_read == QL_SPI_WAIT_WRITE)
//     {
//         spi_wait_write_read = 0;
//         ql_rtos_semaphore_release(spi_write_semaphore);
//     }
//     // LOGD("spi_cb_handler rx_dma_done=%d tx_dma_done=%d\r\n", cause.rx_dma_done, cause.tx_dma_done);
// }

int spi_write_port2(int fd, uint8_t *outbuf, int len)
{
    int ret;
    // spi_wait_write_read = QL_SPI_WAIT_WRITE;
    // ql_spi_request_sys_clk(fd);
    ret = ql_spi_write(fd, outbuf, len);
    // ql_rtos_semaphore_wait(spi_write_semaphore, 10);
    // LOGD("spi_write inbuf=%p,len=%d,ret=%x", outbuf, len, ret);
    return ret;
}

int spi_write_read_port2(int fd, uint8_t *outbuf, uint8_t *inbuf, int len)
{
    int ret;
    // spi_wait_write_read = QL_SPI_WAIT_READ;
    ret = ql_spi_write_read(fd, inbuf, outbuf, len);
    // ql_rtos_semaphore_wait(spi_read_semaphore, 10);
    // LOGD("spi_write_read inbuf=%p,outbuf=%p,len=%d,ret=%x", inbuf, outbuf, len, ret);
    return ret;
}

static void gpio_mcp_int_cb(void *ctx)
{
    ql_event_t event;
    event.id = 1000;
    if (can_task_handle)
    {
        ql_rtos_event_send(can_task_handle, &event);
    }

    // LOGI("gpio_mcp_int_cb");
}

osiTimer_t *can_led_timer = NULL;
void can_led_timer_cb(void *param)
{
    ql_event_t event;
    event.id = 1001;
    if (can_task_handle)
    {
        ql_rtos_event_send(can_task_handle, &event);
    }
}
void can_task(void *data)
{
    int ret;
    // ret = ql_pin_set_func(QL_CUR_SPI2_CS_PIN, QUEC_PIN_SPI2_FUNC);
    // if (ret != QL_GPIO_SUCCESS)
    // {
    //     LOGE("set pin err");
    // }
    // ret = ql_pin_set_func(QL_CUR_SPI2_CLK_PIN, QUEC_PIN_SPI2_FUNC);
    // if (ret != QL_GPIO_SUCCESS)
    // {
    //     LOGE("set pin err");
    // }
    // ret = ql_pin_set_func(QL_CUR_SPI2_DO_PIN, QUEC_PIN_SPI2_FUNC);
    // if (ret != QL_GPIO_SUCCESS)
    // {
    //     LOGE("set pin err");
    // }
    // ret = ql_pin_set_func(QL_CUR_SPI2_DI_PIN, QUEC_PIN_SPI2_FUNC);
    // if (ret != QL_GPIO_SUCCESS)
    // {
    //     LOGE("set pin err");
    // }
    // ret = ql_pin_set_func(GPIO_10, 0); // set pin4 sp1_cs to gpio fun
    // if (ret != QL_GPIO_SUCCESS)
    // {
    //     LOGE("set pin err");
    // }
    // ql_gpio_deinit(GPIO_10);
    // ql_gpio_init(GPIO_10, GPIO_INPUT, PULL_UP, LVL_HIGH);

    ql_spi_config_s spi_config = {0};
    int spi_no = QL_SPI_PORT2;
    spi_config.port = spi_no;
    spi_config.spiclk = QL_SPI_CLK_1_5625MHZ;
    spi_config.framesize = 8;
    spi_config.input_mode = QL_SPI_INPUT_TRUE;
    spi_config.cs_polarity0 = QL_SPI_CS_ACTIVE_LOW;
    spi_config.cs_polarity1 = QL_SPI_CS_ACTIVE_LOW;
    spi_config.cpol = QL_SPI_CPOL_LOW;
    spi_config.cpha = QL_SPI_CPHA_1Edge;
    spi_config.input_sel = QL_SPI_DI_1;
    spi_config.transmode = QL_SPI_DIRECT_POLLING;
    spi_config.cs = QL_SPI_CS0;
    spi_config.clk_delay = QL_SPI_CLK_DELAY_0;
    ql_spi_init_ext(spi_config);
    ql_spi_cs_auto(spi_no);
    // ql_rtos_semaphore_create(&spi_read_semaphore, 0);
    // ql_rtos_semaphore_create(&spi_write_semaphore, 0);

    // ql_spi_irq_s mask = {
    //     .rx_dma_done = 1,
    //     .tx_dma_done = 1,
    // };

    // ql_spi_set_irq(spi_no, mask, spi_cb_handler);

    can_config_t can_cfg;
    memset(&can_cfg, 0, sizeof(can_cfg));
    can_cfg.baud_rate = 500000;
    can_cfg.can_mode = MODE_NORMAL;
    can_cfg.rx_ctrl[0] = RXB_RX_ANY;
    can_cfg.rx_ctrl[1] = RXB_RX_ANY;
    can_cfg.rx_mask[0] = 0x1fffffff | CAN_EFF_FLAG;
    can_cfg.rx_mask[1] = 0x1fffffff | CAN_EFF_FLAG;
    can_cfg.rx_filter[0] = 0x18FEEE00 | CAN_EFF_FLAG;
    can_cfg.rx_filter[1] = 0x18FEEF00 | CAN_EFF_FLAG;
    can_cfg.rx_filter[2] = 0X18FEE500 | CAN_EFF_FLAG;
    ret = mcp_can_init(spi_no, &can_cfg);
    if (ret != 0)
    {
        LOGE("mcp_can_init fail ret=%d\n", ret);
    }
    // ql_rtos_task_get_current_ref(&can_task_handle);
    can_task_handle = osiThreadCurrent();
    // ql_pin_set_func(15, 4); // set GPIO22
    ql_int_register(GPIO_22, EDGE_TRIGGER, DEBOUNCE_DIS, EDGE_FALLING, PULL_UP, gpio_mcp_int_cb, NULL);
    ql_int_enable(GPIO_22);
    int tx_led_status = 0;
    int rx_led_status = 0;
    // can_led_timer = osiTimerCreate(can_task_handle, can_led_timer_cb, NULL); // 创建定时器
    // if (NULL == can_led_timer)
    // {
    //     LOGE("can_led_timer create failed\n");
    //     osiThreadExit();
    // }
    // ret = osiTimerStart(can_led_timer, 100);
    // if (ret != TRUE)
    // {
    //     LOGE("can_led_timer start failed\n");
    // }
    uint32_t sum_channel0 = 0;
    uint32_t sum_channel1 = 0;
    uint32_t sum_channel2 = 0;
    int count = 0;
    uint16_t channel0 = 0;
    uint16_t channel1 = 0;
    uint16_t channel2 = 0;
    while (1)
    {
        // ql_event_t event;
        // ql_LvlMode int_value;
        // if (ql_event_wait(&event, 100) != 0) // 中断有时会丢失，所以超时也读取接收缓冲区
        // {
        //     event.id = 1000;
        //     ql_gpio_get_level(GPIO_22, &int_value);
        // }
        // else
        // {
        //     int_value = 0;
        //     event.id = 0;
        // }
        // // event.id = 0;
        // switch (event.id)
        // {

        // case 1000: {
        //     if (int_value == 0)
        //     {
        //         LED0_ON(spi_no);
        //         rx_led_status = 1;
        //         do
        //         {
        //             CanRxMsg RxMsg;
        //             if (can_read_rxbuf(spi_no, &RxMsg))
        //             {
        //                 // LOGI("CAN0: Id=0x%x,IDE=%d,DLC=%d,Data=[%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x]", RxMsg.Id, RxMsg.IDE, RxMsg.DLC, RxMsg.Data[0], RxMsg.Data[1], RxMsg.Data[2], RxMsg.Data[3], RxMsg.Data[4], RxMsg.Data[5], RxMsg.Data[6], RxMsg.Data[7]);
        //                 //  if (can_ops[0] != NULL && can_ops[0]->parse)
        //                 //  {
        //                 //      can_status[0]->rx_cnt++;
        //                 //      can_ops[0]->parse(&RxMsg);
        //                 //  }
        //             }
        //             ql_gpio_get_level(GPIO_22, &int_value);
        //         } while (int_value == 0);
        //         LED0_OFF(spi_no);
        //     }
        //     break;
        // }
        // break;
        // // case 1001:
        // //     if (rx_led_status == 1)
        // //     {
        // //         LED0_OFF(spi_no);
        // //         rx_led_status = 2;
        // //     }
        // //     else if(rx_led_status == 2)
        // //     {
        // //         rx_led_status = 0;
        // //     }
        // //     if (tx_led_status == 1)
        // //     {
        // //         LED1_OFF(spi_no);
        // //         tx_led_status = 2;
        // //     }
        // //     else if(rx_led_status == 2)
        // //     {
        // //         rx_led_status = 0;
        // //     }
        // //     osiTimerStart(can_led_timer, 100);
        // //     break;
        // // case 0: {

        // // }
        // // break;
        // }
            sum_channel0 += gWordVar[AC_CURRENT_REG_ADDR + 0];
            sum_channel1 += gWordVar[AC_CURRENT_REG_ADDR + 1];
            sum_channel2 += gWordVar[AC_CURRENT_REG_ADDR + 2];

        count++;
        if (count >=5)
        {

            channel0 = sum_channel0 / count;
            channel1 = sum_channel1 / count;
            channel2 = sum_channel2 / count;

            sum_channel0 = 0;
            sum_channel1 = 0;
            sum_channel2 = 0;
            count = 0;
        }

        CanTxMsg TxPack = {
                .Id = 0x201,
                .IDE = CAN_ID_STD,
                .DLC = 8,
                .Data[0] = channel0 & 0xFF ,
                .Data[1] = (channel0 >> 8) & 0xFF,
                .Data[2] = channel1 & 0xFF,
                .Data[3] = (channel1 >> 8) & 0xFF,
                .Data[4] = channel2 & 0xFF,
                .Data[5] = (channel2 >> 8) & 0xFF,
                };
            LED1_ON(spi_no);
            tx_led_status = 1;
            can_tx_pack(spi_no, &TxPack);
            can_status[0]->bus_status = get_mcp_status(spi_no);
        ql_rtos_task_sleep_ms(100);
    }
}
// gWordVar[AC_CURRENT_REG_ADDR + 0] & 0xFF
// gWordVar[AC_CURRENT_REG_ADDR + 0] << 8 & 0xFF
// gWordVar[AC_CURRENT_REG_ADDR + 1] & 0xFF
// gWordVar[AC_CURRENT_REG_ADDR + 1] << 8 & 0xFF
// gWordVar[AC_CURRENT_REG_ADDR + 2] & 0xFF
// gWordVar[AC_CURRENT_REG_ADDR + 2] << 8 & 0xFF

void can_task_init(void)
{
    QlOSStatus err = 0;
    ql_task_t can_task_ref;
    err = ql_rtos_task_create(&can_task_ref, 4*1024, APP_PRIORITY_BELOW_NORMAL, "can_current_task", can_task, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        LOGI("creat can_current_task fail err = %d", err);
    }
    LOGI("creat can_current_task sucess!");
}
