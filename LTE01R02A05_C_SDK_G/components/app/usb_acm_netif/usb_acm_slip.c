

#include "ql_api_osi.h"
#include "drv_uart.h"
#include "ql_uart.h"
#include "osi_api.h"
#include "netif.h"
#include "netif/ethernet.h"
#include "etharp.h"
#include "ql_log.h"
#include "stdlib.h"

#define LOG_TAG "usb_acm_slip"
#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, msg, ##__VA_ARGS__)

// const uint16_t  eth_type_list[] = {0x0000,0x0800, 0x0806, 0x8035, 0x86DD, 0x8100,0xffff,0xffff};
// const uint8_t eth_broadcast_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const uint8_t eth_mac[] = {0x00, 0x0E, 0x20, 0x24, 022, 0x04};
static ip4_nat_entry_t *nat_entry = NULL;
static ql_sem_t sem_send;
ql_task_t usb_acm_slip_task = NULL;

// static uint8_t acm_net_rxbuf[2048];

static struct netif eth_net_if;

// static char hex[1514 * 3 + 2];

// static char *bin2hex(uint8_t *bin, int lenght)
// {

//     if (lenght > sizeof(hex) / 3)
//     {
//         lenght = sizeof(hex) / 3;
//     }
//     int i;
//     for (i = 0; i < lenght; i++)
//     {
//         sprintf(hex + i * 3, "%02X ", bin[i]);
//     }
//     hex[i * 3] = '\0';
//     return hex;
// }

void acm_uart_notify_cb(unsigned int ind_type, ql_uart_port_number_e port, unsigned int size)
{

    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "UART port %d receive ind type:0x%x, receive data size:%d", port, ind_type, size);
    switch (ind_type)
    {
    case QUEC_UART_RX_OVERFLOW_IND: // rx buffer overflow
    case QUEC_UART_RX_RECV_DATA_IND:
        // LOGI("UART port %d receive ind type:0x%x, receive data size:%d", port, ind_type, size);
        if (size > 0)
        {
            ql_event_t event;
            event.id = QUEC_UART_RX_RECV_DATA_APP_IND;
            event.param1 = size;
            ql_rtos_event_send(usb_acm_slip_task, &event);
        }
        break;
    case QUEC_UART_TX_FIFO_COMPLETE_IND:
        // LOGI("UART port %d tx complete.\n",port);
        ql_rtos_semaphore_release(sem_send);
        break;
    }
}

static uint8_t usb_tx_buf[2048];
static uint8_t usb_rx_buf[2048];

static err_t acm_net_output(struct netif *netif, struct pbuf *p)
{
    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG ,"acm_net_output pbuf=%p,len=%d,tot_len=%d,data=[%s]\n", p, p->tot_len,p->len,bin2hex(p->payload, p->len));
    unsigned int write_len = 0;
    int out_idx = 0;
    if (QL_OSI_SUCCESS != ql_rtos_semaphore_wait(sem_send, 100))
    {
        QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, "sem_send wait fail\n");
    }
    struct pbuf *pbuf = p;
    usb_tx_buf[out_idx++] = 0xC0;
    while (pbuf != NULL)
    {
        uint8_t *data = (uint8_t *)pbuf->payload;
        for (int i = 0; i < pbuf->len; i++)
        {
            uint8_t byte = data[i];
            if (byte == 0xC0)
            {
                usb_tx_buf[out_idx++] = 0xDB;
                usb_tx_buf[out_idx++] = 0xDC;
            }
            else if (byte == 0xDB)
            {
                usb_tx_buf[out_idx++] = 0xDB;
                usb_tx_buf[out_idx++] = 0xDD;
            }
            else
            {
                usb_tx_buf[out_idx++] = byte;
            }
            if (out_idx >= sizeof(usb_tx_buf) - 2)
            {
                write_len += ql_uart_write(QL_USB_PORT_MODEM, usb_tx_buf, out_idx);
                out_idx = 0;
            }
        }
        pbuf = pbuf->next;
    }
    usb_tx_buf[out_idx++] = 0xC0;
    write_len += ql_uart_write(QL_USB_PORT_MODEM, usb_tx_buf, out_idx);
    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "acm_net_output len =%d\n", write_len);
    return 0;
}

err_t acm_netif_init(struct netif *netif)
{
    if (netif == NULL)
    {
        QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, "acm_netif_init netif is NULL\n");
        return ERR_ARG;
    }
    netif->name[0] = 'E';
    netif->name[1] = 'T';
    netif->num = 0;
    netif->link_mode = NETIF_LINK_MODE_NAT_LWIP_LAN;
    netif->output = etharp_output;
    netif->linkoutput = acm_net_output;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_UP | NETIF_FLAG_BROADCAST | NETIF_FLAG_LINK_UP | NETIF_FLAG_ETHARP;
    netif->hwaddr_len = 6;
    memcpy(netif->hwaddr, eth_mac, 6);
    netif->sim_cid = 0x01;
    return ERR_OK;
}

void usb_acm_net_thread(void *param)
{
    static struct pbuf *rx_pkt_buf = NULL;
    uint8_t *out_buf = NULL;
    int buf_idx = 0;
    int out_idx = 0;
    int end_pos = 0;
    int read_len = 0;
    ql_event_t event;
    // ql_rtos_task_sleep_s(10);

    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG ,"enter uart1_task_thread!\n");
    int ret = 0;
    int run_loop = 1;
    ql_rtos_semaphore_create(&sem_send, 0);
    ip4_addr_t ipaddr, netmask, gw;
    IP4_ADDR(&ipaddr, 192, 168, 10, 1);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, 192, 168, 10, 0);
    memset(&eth_net_if, 0, sizeof(struct netif));
    netif_add(&eth_net_if, &ipaddr, &netmask, &gw, NULL, acm_netif_init, ethernet_input);
    netif_set_down(&eth_net_if);
    // LOGI("eth_net_if ip4 addr=%s",ip4addr_ntoa(netif_ip4_addr(&eth_net_if)));

    ret = ql_uart_open(QL_USB_PORT_MODEM);
    if (ret != QL_UART_SUCCESS)
    {
        LOGE("ql_uart_open fail ret = %d\n", ret);
        goto err1;
    }
    LOGI("ql_uart_open success\n");
    ret = ql_uart_register_cb(QL_USB_PORT_MODEM, acm_uart_notify_cb);
    LOGE("ql_uart_register_cb ret = %d\n", ret);
    ql_rtos_semaphore_release(sem_send);
    netif_set_up(&eth_net_if);

    run_loop = 1;
    while (run_loop)
    {
    start:
        ql_event_try_wait(&event);
        // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "event id = %d\n", event.id);
        int uart_recv_size = event.param1;
        if (event.id == QUEC_UART_RX_RECV_DATA_APP_IND)
        {
        read:
            read_len = ql_uart_read(QL_USB_PORT_MODEM, &usb_rx_buf[end_pos], sizeof(usb_rx_buf) - end_pos);
            if (read_len <= 0)
            {
                goto start;
            }
            LOGI("uart5 read len = %d\n", read_len);
            uart_recv_size -= read_len;
            end_pos += read_len;
        sync:
            if (rx_pkt_buf == NULL)
            {
                while (buf_idx < end_pos)
                {
                    if (usb_rx_buf[buf_idx++] == 0xC0)
                    {
                        rx_pkt_buf = pbuf_alloc(PBUF_LINK, 1514, PBUF_POOL);
                        if (rx_pkt_buf == NULL)
                        {
                            QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "pbuf_alloc fail\n");
                            end_pos = 0; // pbuf满，丢弃
                            buf_idx = 0;
                            goto start;
                        }
                        // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG ,"pbuf_alloc addr=%p,next=%p\n", rx_pkt_buf, rx_pkt_buf->next);
                        rx_pkt_buf->len = 0;
                        rx_pkt_buf->tot_len = 0;
                        out_idx = 0;
                        out_buf = rx_pkt_buf->payload;
                        break;
                    }
                }
            }
            while (buf_idx < end_pos)
            {
                uint8_t ch = usb_rx_buf[buf_idx++];
                if (ch == 0xDB)
                {
                    if (buf_idx == end_pos - 1) // 最后一个字节为转意字，结束循环，下次再处理
                    {
                        usb_rx_buf[0] = 0xDB;
                        end_pos = 1;
                        buf_idx = 0;
                        goto start;
                    }
                    ch = usb_rx_buf[buf_idx++];
                    if (ch == 0xDC)
                    {
                        out_buf[out_idx++] = 0xC0;
                    }
                    else if (ch == 0xDD)
                    {
                        out_buf[out_idx++] = 0xDB;
                    }
                    else // 状态错误，重新同步
                    {
                        pbuf_free(rx_pkt_buf);
                        rx_pkt_buf = NULL;
                        goto sync;
                    }
                }
                else if (ch == 0xc0) // 收到完整包
                {
                    rx_pkt_buf->len = out_idx;
                    rx_pkt_buf->tot_len = out_idx;
                    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG ,"ethernet_input len = %d data=[%s]\n", rx_pkt_buf->len, bin2hex(rx_pkt_buf->payload, rx_pkt_buf->len));
                    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "ethernet_input len = %d\n", rx_pkt_buf->len);
                    if (out_idx == 0)
                    {
                        continue;
                    }
                    ethernet_input(rx_pkt_buf, &eth_net_if);
                    rx_pkt_buf = NULL;
                    if (buf_idx < end_pos)
                    {
                        goto sync;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    out_buf[out_idx++] = ch;
                }
            }
        done:
            buf_idx = 0;
            end_pos = 0;
            if (uart_recv_size > 0)
            {
                goto read;
            }
        }
        else if ((event.id == QUEC_DATACALL_ACT_RSP_IND))
        {
            if (nat_entry != NULL)
            {
                remove_static_route(nat_entry);
                nat_entry = NULL;
            }
            struct netif *wan_if = netif_get_by_cid_type(0x01, NETIF_LINK_MODE_NAT_WAN);
            if (wan_if != NULL)
            {
                IP4_ADDR(&ipaddr, 0, 0, 0, 0);
                IP4_ADDR(&netmask, 255, 255, 255, 255);
                nat_entry = add_static_route(ipaddr, netmask, &eth_net_if, wan_if);
            }
        }
    }
err1:
    osiThreadExit();
}

void usb_acm_slip_init(void)
{
    QlOSStatus err = 0;

    err = ql_rtos_task_create(&usb_acm_slip_task, 8192, APP_PRIORITY_REALTIME, "usb_net_task", usb_acm_net_thread, NULL, 4);
    if (err != QL_OSI_SUCCESS)
    {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "creat usb_net task fail err = %d", err);
    }
    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "creat usb_net task sucess");
}
