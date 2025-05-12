
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>

#include "ModbusS.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "ql_log.h"
extern struct netif *t2n_if;
#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "modbus_tcp", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "modbus_tcp", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "modbus_tcp", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "modbus_tcp", msg, ##__VA_ARGS__)
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
// static err_t ModBusSlave_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
// static err_t ModBusSlave_accept(void *arg, struct tcp_pcb *pcb, err_t err);
// static void ModBusSlave_conn_err(void *arg, err_t err);
// static err_t ModBusSlave_poll(void *arg, struct tcp_pcb *pcb);
/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Called when a data is received on the telnet connection
 * @param  arg	the user argument
 * @param  pcb	the tcp_pcb that has received the data
 * @param  p	the packet buffer
 * @param  err	the error value linked with the received data
 * @retval error value
 */
int ModBusTcpMonitor;
static err_t ModBusSlave_recv(void *arg, struct tcp_pcb *newpcb, struct pbuf *p, err_t err)
{
    /* We perform here any necessary processing on the pbuf */
    if (p != NULL)
    {
        uint8_t *txbuf;
        uint8_t *rxbuf;
        int txlen, rxlen;
        /* We call this function to tell the LwIp that we have processed the data */
        /* This lets the stack advertise a larger window, so more data can be received*/
        tcp_recved(newpcb, p->tot_len);
        rxbuf = (uint8_t *)p->payload;
        rxlen = p->len;
        if (rxlen < 6)
        {
            pbuf_free(p);
            return tcp_close(newpcb);
        }
        txbuf = (uint8_t *)malloc(1024 + 16);
        if (txbuf == NULL)
        {
            return tcp_close(newpcb);
        }
        ModBusTcpMonitor = 0;
        txlen = ModbusSlaveProcess(&txbuf[6], &rxbuf[6], (rxlen - 6), 0);
        LOGI("[%s] ModbusSlaveProcess (ret=%d)", __FUNCTION__, txlen);
        if (txlen > 0)
        {
            int i;
            for (i = 0; i < 4; i++)
            {
                txbuf[i] = rxbuf[i];
            }
            txbuf[4] = (uint8_t)(txlen >> 8);
            txbuf[5] = (uint8_t)(txlen & 0xff);
            tcp_write(newpcb, txbuf, txlen + 6, 1);
            ModBusTcpMonitor = 0;
        }
        free(txbuf);
        /* End of processing, we free the pbuf */
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
        /* We free the allocated memory and we close the connection */
        LOGW("remote end is closing the connection\n");
        tcp_err(newpcb, NULL);
        tcp_recv(newpcb, NULL);
        tcp_poll(newpcb, NULL, 120); // 60 second timeout
        // return tcp_close(newpcb);
        err_t err_p = tcp_close(newpcb);
        if (ERR_OK != err_p)
        {
            LOGE("close error. err:%d\n", err_p);
        }
        return err_p;
    }
    return ERR_OK;
}
/**
 * @brief  This function is called when an error occurs on the connection
 * @param  arg
 * @parm   err
 * @retval None
 */
static void ModBusSlave_conn_err(void *arg, err_t err)
{
    LOGE("connection error, err:%d\n", err);
}
static err_t ModBusSlave_poll(void *arg, struct tcp_pcb *newpcb)
{
    if (newpcb)
    {
        LOGE("close pcb\n");
        tcp_close(newpcb);
        newpcb = NULL;
    }
    return ERR_OK;
}
/**
 * @brief  This function when the Telnet connection is established
 * @param  arg  user supplied argument
 * @param  pcb	 the tcp_pcb which accepted the connection
 * @param  err	 error value
 * @retval ERR_OK
 */
static err_t ModBusSlave_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    /* Tell LwIP to associate this structure with this connection. */
    // tcp_arg(pcb, mem_calloc(sizeof(struct name), 1));
    /* Configure LwIP to use our call back functions. */
    tcp_err(pcb, ModBusSlave_conn_err);
    tcp_recv(pcb, ModBusSlave_recv);
    tcp_poll(pcb, ModBusSlave_poll, 120); // 60 second timeout
    /* Send out the first message */
    // tcp_write(pcb, GREETING, strlen(GREETING), 1);
    return ERR_OK;
}
/**
 * @brief  Initialize the hello application
 * @param  None
 * @retval None
 */
void ModBusTCPSlave_init(void)
{
    struct tcp_pcb *pcb;
    /* Create a new TCP control block  */
    pcb = tcp_new();
    /* Assign to the new pcb a local IP address and a port number */
    /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
    tcp_bind(pcb, NULL, 502);
    // tcp_bind_netif(pcb, t2n_if);
    /* Set the connection to the LISTEN state */
    pcb = tcp_listen(pcb);
    /* Specify the function to be called when a connection is established */
    tcp_accept(pcb, ModBusSlave_accept);
    // LSAPI_Log_Debug("ModBus Tcp server listen port:%d\n", 502);
}
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
