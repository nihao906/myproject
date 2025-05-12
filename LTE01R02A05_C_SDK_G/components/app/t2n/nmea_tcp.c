#include <stdint.h>
#include <string.h>

#include "lwip/err.h"
#include "lwip/tcp.h"
#include "ql_log.h"

extern struct netif *t2n_if;
#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "modbus_tcp", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "modbus_tcp", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "modbus_tcp", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "modbus_tcp", msg, ##__VA_ARGS__)
struct tcp_pcb *gps1_acce_newpacb[1]; // 只能容纳1个客户端
struct tcp_pcb *gps2_acce_newpacb[1]; // 只能容纳1个客户端
#define NMEA_TCP_SEND_BUF_LEN 512
// static char nmea1_tcp_sendBuf[NMEA_TCP_SEND_BUF_LEN] = {0}; // 发送缓冲区 // gga+rmc[512]
// static char nmea2_tcp_sendBuf[NMEA_TCP_SEND_BUF_LEN] = {0}; // 发送缓冲区 // gga+rmc[512]
// int nmea1_idx = 0;
// int nmea2_idx = 0;
typedef struct
{
    int index;
    char data[NMEA_TCP_SEND_BUF_LEN];
} tcp_send_buf_t;

tcp_send_buf_t nmea_tcp_send_buf[2];
/**
 * @brief Gps Tcp数据发送处理
 *
 * @param newpcb
 * @param input_line
 * @return int
 */
int NmeaTcp_send(uint8_t gps_id, char *input_line)
{
    struct tcp_pcb *newpcb = NULL;
    tcp_send_buf_t *tcp_send_buf = NULL;
    if (gps_id == 0)
    {
        newpcb = gps1_acce_newpacb[0];
        tcp_send_buf = &nmea_tcp_send_buf[0];
    }
    else if (gps_id == 1)
    {
        newpcb = gps2_acce_newpacb[0];
        tcp_send_buf = &nmea_tcp_send_buf[1];
    }
    else
    {
        return ERR_ARG;
    }
    if (newpcb == NULL || input_line == NULL || tcp_send_buf == NULL)
    {
        return ERR_ARG;
    }
    int input_len = strlen(input_line);
    if (tcp_send_buf->index == 0)
    {
        strncpy(tcp_send_buf->data, input_line, NMEA_TCP_SEND_BUF_LEN - 2); // 入参已经确保为字符串
        tcp_send_buf->index = input_len;
        tcp_send_buf->data[tcp_send_buf->index++] = '\r';
        tcp_send_buf->data[tcp_send_buf->index++] = '\n';
    }
    else
    {
        strncpy(&tcp_send_buf->data[tcp_send_buf->index], input_line, NMEA_TCP_SEND_BUF_LEN - tcp_send_buf->index - 2); // 入参已经确保为字符串
        tcp_send_buf->index += input_len;
        tcp_send_buf->data[tcp_send_buf->index++] = '\r';
        tcp_send_buf->data[tcp_send_buf->index++] = '\n';
        tcp_write(newpcb, tcp_send_buf->data, tcp_send_buf->index, 1);
        tcp_send_buf->index = 0;
        tcp_output(newpcb);
    }

    // if (*nmea_idx == 0)
    // { // 当前只判断第一字节是否为0和长度是否为0
    //   // memset(nmea_tcp_sendBuf, 0x00, NMEA_TCP_SEND_BUF_LEN);
    //   strncpy(nmea_tcp_sendBuf, input_line, input_len); // 入参已经确保为字符串
    //   nmea_tcp_sendBuf[input_len] = '\r';
    //   nmea_tcp_sendBuf[input_len + 1] = '\n';
    //   *nmea_idx += input_len + 2;
    // }
    // else
    // {
    //   int buf_len = strlen(nmea_tcp_sendBuf);
    //   if (input_len > (NMEA_TCP_SEND_BUF_LEN - buf_len))
    //   { // 查询剩余空间是否充足
    //     nmea_tcp_sendBuf[buf_len] = '\r';
    //     nmea_tcp_sendBuf[buf_len + 1] = '\n';
    //     buf_len = buf_len + 2;
    //     log_printf(LDEBUG, LOG_TAG "A:NmeaTcp_send buf:%s,len:%d\n", nmea_tcp_sendBuf, buf_len);
    //     tcp_write(newpcb, nmea_tcp_sendBuf, buf_len, 1);
    //     tcp_output(newpcb);
    //   }
    //   else
    //   {
    //     strcpy(nmea_tcp_sendBuf + buf_len, input_line);
    //     buf_len = strlen(nmea_tcp_sendBuf);
    //     nmea_tcp_sendBuf[buf_len] = '\r';
    //     nmea_tcp_sendBuf[buf_len + 1] = '\n';
    //     buf_len = buf_len + 2;
    //     log_printf(LDEBUG, LOG_TAG "B:NmeaTcp_send buf:%s,len:%d\n", nmea_tcp_sendBuf, buf_len);
    //     tcp_write(newpcb, nmea_tcp_sendBuf, buf_len, 1);
    //     tcp_output(newpcb);
    //   }
    // memset(nmea_tcp_sendBuf, 0x00, NMEA_TCP_SEND_BUF_LEN);
    // }
    return ERR_OK;
}

/**
 * @brief Gps Tcp数据接收处理
 *
 * @param arg
 * @param newpcb
 * @param p
 * @param err
 * @return err_t
 */
static err_t Gps1NmeaTcp_recv(void *arg, struct tcp_pcb *newpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
        uint8_t *rxbuf;
        int rxlen;
        tcp_recved(newpcb, p->tot_len);
        rxbuf = (uint8_t *)p->payload;
        rxlen = p->len;
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        LOGW("gps1 remote end is closing the connection\n");
        for (uint8_t i = 0; i < 1; i++)
        { // 清空所有客户端
            gps1_acce_newpacb[i] = 0;
        }
        tcp_err(newpcb, NULL);
        tcp_recv(newpcb, NULL);
        tcp_poll(newpcb, NULL, 120);
        err_t err_p = tcp_close(newpcb);
        if (ERR_OK != err_p)
        {
            LOGE("gps1 close error. err:%d\n", err_p);
        }
        return err_p;
    }
    return ERR_OK;
}
static err_t Gps2NmeaTcp_recv(void *arg, struct tcp_pcb *newpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
        uint8_t *rxbuf;
        int rxlen;
        tcp_recved(newpcb, p->tot_len);
        rxbuf = (uint8_t *)p->payload;
        rxlen = p->len;
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        LOGW("gps2 remote end is closing the connection\n");
        for (uint8_t i = 0; i < 1; i++)
        { // 清空所有客户端
            gps2_acce_newpacb[i] = 0;
        }
        tcp_err(newpcb, NULL);
        tcp_recv(newpcb, NULL);
        tcp_poll(newpcb, NULL, 120);
        err_t err_p = tcp_close(newpcb);
        if (ERR_OK != err_p)
        {
            LOGE("gps2 close error. err:%d\n", err_p);
        }
        return err_p;
    }
    return ERR_OK;
}
/**
 * @brief Gps Tcp client连接错误处理
 *
 * @param arg
 * @param err
 */
static void NmeaTcp_conn_err(void *arg, err_t err)
{
    LOGE("connection error, err:%d\n", err);
}

static err_t NmeaTcp_poll(void *arg, struct tcp_pcb *newpcb)
{
    if (newpcb)
    {
        LOGI("close pcb\n");
        tcp_close(newpcb);
        newpcb = NULL;
    }
    return ERR_OK;
}

/**
 * @brief Gps1 Tcp client连接处理
 *
 * @param arg
 * @param newpcb
 * @param err
 * @return err_t
 */
static err_t Gps1NmeaTcp_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_err(newpcb, NmeaTcp_conn_err);
    tcp_recv(newpcb, Gps1NmeaTcp_recv);
    tcp_poll(newpcb, NmeaTcp_poll, 120);
    gps1_acce_newpacb[0] = newpcb;
    return ERR_OK;
}

/**
 * @brief Gps2 Tcp client连接处理
 *
 * @param arg
 * @param newpcb
 * @param err
 * @return err_t
 */
static err_t Gps2NmeaTcp_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_err(newpcb, NmeaTcp_conn_err);
    tcp_recv(newpcb, Gps2NmeaTcp_recv);
    tcp_poll(newpcb, NmeaTcp_poll, 120);
    gps2_acce_newpacb[0] = newpcb;
    return ERR_OK;
}

/**
 * @brief Gps2 Tcp server初始化处理
 *
 */
void NmeaTcp_init(void)
{
    struct tcp_pcb *pcb_gps1;
    pcb_gps1 = tcp_new();
    tcp_bind(pcb_gps1, NULL, 6061);
    tcp_bind_netif(pcb_gps1, t2n_if);
    pcb_gps1 = tcp_listen(pcb_gps1);
    tcp_accept(pcb_gps1, Gps1NmeaTcp_accept);
    LOGD("Gps1 Nmea Tcp server listen port:%d\n", 6061);

    struct tcp_pcb *pcb_gps2;
    pcb_gps2 = tcp_new();
    tcp_bind(pcb_gps2, NULL, 6062);
    tcp_bind_netif(pcb_gps2, t2n_if);
    pcb_gps2 = tcp_listen(pcb_gps2);
    tcp_accept(pcb_gps2, Gps2NmeaTcp_accept);
    LOGD("Gps2 Nmea Tcp server listen port:%d\n", 6062);
}
