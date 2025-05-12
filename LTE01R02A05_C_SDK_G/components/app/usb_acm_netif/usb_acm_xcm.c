

#include "ql_api_osi.h"
#include "drv_uart.h"
#include "ql_uart.h"
#include "osi_api.h"
#include "netif.h"
// #include "netif/etharp.h"
#include "netif/ethernet.h"
#include "etharp.h"
#include "ql_log.h"
#include "stdlib.h"

#define LOG_TAG "usb_acm_xcm"
#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, msg, ##__VA_ARGS__)

#define SYNC0 'Y'
#define SYNC1 'H'
#define SYNC2 'C'
#define SYNC3 'M'

const uint8_t eth_broadcast_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const uint8_t eth_mac[] = {0x00, 0x0E, 0x20, 0x24, 022, 0x04};
static ip4_nat_entry_t *nat_entry = NULL;
static ql_sem_t sem_send;
static ql_mutex_t mutex_send;
ql_task_t usb_acm_slip_task = NULL;
int usb_fifo_full = 0;

static struct netif eth_net_if;

static char hex[1514 * 3 + 2];

static char *bin2hex(uint8_t *bin, int lenght)
{

    if (lenght > sizeof(hex) / 3)
    {
        lenght = sizeof(hex) / 3;
    }
    int i;
    for (i = 0; i < lenght; i++)
    {
        sprintf(hex + i * 3, "%02X ", bin[i]);
    }
    hex[i * 3] = '\0';
    return hex;
}

int send_pending = 0;

void acm_uart_notify_cb(unsigned int ind_type, ql_uart_port_number_e port, unsigned int size)
{

    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "UART port %d receive ind type:0x%x, receive data size:%d", port, ind_type, size);
    switch (ind_type)
    {
    case QUEC_UART_RX_OVERFLOW_IND: // rx buffer overflow
    case QUEC_UART_RX_RECV_DATA_IND:
        LOGI("UART port %d receive data size:%d", port, size);
        if (size > 0)
        {
            ql_event_t event;
            event.id = QUEC_UART_RX_RECV_DATA_APP_IND;
            event.param1 = size;
            ql_rtos_event_send(usb_acm_slip_task, &event);
        }
        break;
    case QUEC_UART_TX_FIFO_COMPLETE_IND:
        // LOGI("UART port %d tx complete.\n", port);
        usb_fifo_full = 0;
        ql_rtos_semaphore_release(sem_send);
        break;
    }
}


static uint8_t usb_rx_buf[70];

static err_t acm_net_output(struct netif *netif, struct pbuf *p)
{
    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "acm_net_output pbuf=%p,len=%d,tot_len=%d,data=[%s]\n", p, p->tot_len, p->len, bin2hex(p->payload, p->len));
    int ret = ql_rtos_mutex_lock(mutex_send, 500);
    if (ret != QL_OSI_SUCCESS)
    {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "acm_net_output lock fail\n");
        return ERR_TIMEOUT;
    }
    if (usb_fifo_full)
    {
        if (ql_rtos_semaphore_wait(sem_send, 100) != QL_OSI_SUCCESS)
        {
            QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, "wait sem_send fail\n");
            ret = ERR_BUF;
            goto exit;
        }
    }
    unsigned int write_len = 0;
    if (pbuf_header(p, 6))
    {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "acm_net_output: no space for options header\n");
        ret = ERR_BUF;
        goto exit;
    }
    uint8_t *header = p->payload;
    header[0] = SYNC0;
    header[1] = SYNC1;
    header[2] = SYNC2;
    header[3] = SYNC3;
    header[4] = (p->tot_len - 6) >> 8;
    header[5] = (p->tot_len - 6) & 0xff;
    struct pbuf *pbuf = p;
    int offset = 0;
    while (pbuf != NULL)
    {
        int write_len = pbuf->len - offset;
        int actual_len = ql_uart_write(QL_USB_PORT_MODEM, (uint8_t *)pbuf->payload + offset, write_len);
        // int actual_len = ql_uart_write(QL_USB_PORT_MODEM, (uint8_t *)pbuf->payload, pbuf->len);
        if (actual_len < 0)
        {
            QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, "acm_net_output write fail\n");
            ql_rtos_mutex_unlock(mutex_send);
            ret = ERR_CLSD;
            goto exit;
        }
        if (actual_len < write_len)
        {
            usb_fifo_full = 1;
            offset += actual_len;
            if (ql_rtos_semaphore_wait(sem_send, 100) != QL_OSI_SUCCESS)
            {
                QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, "fifo full\n");
                ret = ERR_TIMEOUT;
                goto exit;
            }
            continue;
        }
        else
        {
            usb_fifo_full = 0;
        }
        offset = 0;
        pbuf = pbuf->next;
    }
    ret = ERR_OK;
    ql_rtos_semaphore_wait(sem_send, 0);
    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "acm_net_output len =%d\n", write_len);
exit:
    ql_rtos_mutex_unlock(mutex_send);
    return ret;
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
    int end_pos = 0;
    int read_len = 0;
    int buf_index = 0;
    ql_event_t event;
    // ql_rtos_task_sleep_s(10);

    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG ,"enter uart1_task_thread!\n");
    int ret = 0;
    int run_loop = 1;
    ql_rtos_semaphore_create(&sem_send, 0);
    ql_rtos_mutex_create(&mutex_send);
    ip4_addr_t ipaddr, netmask, gw;
    IP4_ADDR(&ipaddr, 192, 168, 11, 1);
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
    ret = ql_uart_set_event_mask(QL_USB_PORT_MODEM, QUEC_UART_TX_FIFO_COMPLETE_IND | QUEC_UART_RX_RECV_DATA_IND | QUEC_UART_RX_OVERFLOW_IND);
    LOGI("ql_uart_set_event_mask ret = %d\n", ret);
    ret = ql_uart_register_cb(QL_USB_PORT_MODEM, acm_uart_notify_cb);
    LOGI("ql_uart_register_cb ret = %d\n", ret);
    ql_rtos_semaphore_release(sem_send);
    netif_set_up(&eth_net_if);
    run_loop = 1;
    while (run_loop)
    {
    next:
        ql_event_try_wait(&event);
        // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "event id = %d\n", event.id);
        int uart_recv_size = event.param1;
        if (event.id == QUEC_UART_RX_RECV_DATA_APP_IND)
        {
            while (1)
            {
                while (rx_pkt_buf == NULL)
                {
                    read_len = ql_uart_read(QL_USB_PORT_MODEM, &usb_rx_buf[end_pos], 64);
                    if (read_len <= 0)
                    {
                        goto next;
                    }
                    end_pos += read_len;
                    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "ql_uart_read len=%d,buf:[%s]\n", read_len, bin2hex(rx_buf, read_len));
                    while (buf_index < end_pos - 6)
                    {
                        // for (int pos = 0; pos < end_pos - 6; pos++)
                        {
                            if (usb_rx_buf[buf_index] == SYNC0 && usb_rx_buf[buf_index + 1] == SYNC1 && usb_rx_buf[buf_index + 2] == SYNC2 && usb_rx_buf[buf_index + 3] == SYNC3)
                            {
                                int pack_len = (usb_rx_buf[buf_index + 4] << 8) | usb_rx_buf[buf_index + 5];
                                if (pack_len <= 1514 && pack_len >= 40)
                                {
                                    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "async ok pos=%d,len=%d\n", buf_index, pack_len);
                                    rx_pkt_buf = pbuf_alloc(PBUF_LINK, pack_len, PBUF_POOL);
                                    if (rx_pkt_buf == NULL)
                                    {
                                        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "pbuf_alloc fail\n");
                                        // flush recv buffer
                                        int len;
                                        do
                                        {
                                            len = ql_uart_read(QL_USB_PORT_MODEM, usb_rx_buf, 64);
                                        } while (len < 64);
                                        goto next;
                                    }
                                    buf_index += 6;
                                    int len = end_pos - buf_index;
                                    if (len > pack_len)
                                    {
                                        len = pack_len;
                                    }
                                    if (len > 0)
                                    {
                                        memcpy(rx_pkt_buf->payload, &usb_rx_buf[buf_index], len);
                                        buf_index += len;
                                        rx_pkt_buf->len = len;
                                        if (len >= pack_len)
                                        {
                                            // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "ethernet_input len=%d,buf:[%s]\n", pack_len, bin2hex(rx_pkt_buf->payload, rx_pkt_buf->len));
                                            if (memcmp(rx_pkt_buf->payload, eth_mac, 6) == 0 || memcmp(rx_pkt_buf->payload, eth_broadcast_mac, 6) == 0)
                                            {
                                                ethernet_input(rx_pkt_buf, &eth_net_if);
                                            }
                                            else
                                            {
                                                pbuf_free(rx_pkt_buf);
                                                QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "not for me\n");
                                            }
                                            rx_pkt_buf = NULL;
                                        }
                                    }
                                    break;
                                }
                            }
                            else
                            {
                                buf_index++;
                            }
                        }
                        if (buf_index < end_pos)
                        {
                            end_pos = end_pos - buf_index;
                            memmove(usb_rx_buf, &usb_rx_buf[buf_index], end_pos);
                        }
                        else
                        {
                            end_pos = 0;
                        }
                    }
                    if (rx_pkt_buf != NULL)
                    {
                        read_len = ql_uart_read(QL_USB_PORT_MODEM, (unsigned char *)rx_pkt_buf->payload + rx_pkt_buf->len, rx_pkt_buf->tot_len - rx_pkt_buf->len);
                        if (read_len <= 0)
                        {
                            break;
                        }
                        rx_pkt_buf->len += read_len;
                        if (rx_pkt_buf->len >= rx_pkt_buf->tot_len)
                        {
                            // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "ethernet_input len=%d,buf:[%s]\n", rx_pkt_buf->len, bin2hex(rx_pkt_buf->payload, rx_pkt_buf->len));
                            if (memcmp(rx_pkt_buf->payload, eth_mac, 6) == 0 || memcmp(rx_pkt_buf->payload, eth_broadcast_mac, 6) == 0)
                            {
                                ethernet_input(rx_pkt_buf, &eth_net_if);
                            }
                            else
                            {
                                pbuf_free(rx_pkt_buf);
                                QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "not for me\n");
                            }
                            rx_pkt_buf = NULL;
                            end_pos = 0;
                        }
                    }
                }
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
    err1:
        osiThreadExit();
    }
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
