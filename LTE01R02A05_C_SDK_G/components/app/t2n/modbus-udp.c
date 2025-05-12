#include "ModbusS.h"
#include "lwip/ip_addr.h"
#include "lwip/udp.h"
#include "ql_log.h"
extern struct netif *t2n_if;

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "modbus_udp", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "modbus_udp", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "modbus_udp", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "modbus_udp", msg, ##__VA_ARGS__)

void recv_callback_modbus_udp(void *arg, struct udp_pcb *upcb, struct pbuf *pkt_buf,
                              const struct ip_addr *addr, u16_t port)
{
    err_t err = ERR_OK;
    // uint16_t id, _id;
    struct pbuf *pkt_tbuf; /* Chain of pbuf's to be sent */
    uint8_t *txbuf;
    uint8_t *rxbuf;
    int txlen, rxlen;
    /* We call this function to tell the LwIp that we have processed the data */
    /* This lets the stack advertise a larger window, so more data can be received*/
    // eat_trace("[%s] ip=%d.%d.%d.%d port=%d",
    // __FUNCTION__,ip4_addr1(addr),ip4_addr2(addr),ip4_addr3(addr),ip4_addr4(addr),port);
    if (pkt_buf->tot_len < 6)
    {
        pbuf_free(pkt_buf);
        // return ERR_VAL;
    }
    rxbuf = (uint8_t *)pkt_buf->payload;
    // id = (rxbuf[0] << 8) + rxbuf[1];
    // _id = (rxbuf[2] << 8) + rxbuf[3];
    //    if((id + _id)&0xffff != 0)
    //    {
    //        pbuf_free(pkt_buf);
    //        return ERR_VAL;
    //    }
    /* PBUF_TRANSPORT - specifies the transport layer */
    pkt_tbuf = pbuf_alloc(PBUF_TRANSPORT, 1024 + 16, PBUF_RAM);
    if (!pkt_tbuf) /*if the packet pbuf == NULL exit and EndTransfertransmission */
    {
        pbuf_free(pkt_buf);
        // return ERR_MEM;
    }
    txbuf = (uint8_t *)pkt_tbuf->payload;
    rxlen = pkt_buf->len;
    txlen = ModbusSlaveProcess(&txbuf[6], &rxbuf[6], (rxlen - 6), 0);
    // eat_trace("[%s] ModbusSlaveProcess (ret=%d)", __FUNCTION__,txlen);
    if (txlen > 0)
    {
        int i;
        for (i = 0; i < 4; i++)
        {
            txbuf[i] = rxbuf[i];
        }
        txbuf[4] = (uint8_t)(txlen >> 8);
        txbuf[5] = (uint8_t)(txlen & 0xff);
        pkt_tbuf->len = txlen + 6;
        pkt_tbuf->tot_len = txlen + 6;
        err = udp_sendto(upcb, pkt_tbuf, addr, port);
    }
    /* free the buffer pbuf */
    pbuf_free(pkt_buf);
    pbuf_free(pkt_tbuf);
}

void ModBusUDPSlave_init(void)
{
    err_t err;
    unsigned int port = 5002;
    struct udp_pcb *UDPpcb;
    /* create a new UDP PCB structure  */
    UDPpcb = udp_new();
    if (!UDPpcb)
    {
        /* Error creating PCB. Out of Memory  */
        return;
    }
    /* Bind this PCB to port 5002  */

    err = udp_bind(UDPpcb, NULL, port);
    if (err != ERR_OK)
    {
        /* Unable to bind to port  */
        return;
    }
    udp_bind_netif(UDPpcb, t2n_if);
    /* TFTP server start  */
    udp_recv(UDPpcb, recv_callback_modbus_udp, NULL);
}
