
#include <stdint.h>

#include "./t2n_priv.h"
#include "netif/etharp.h"
#include "ModbusS.h"
// #include "lwip/raw.h"
// #include "app_at_cmd_envelope.h"
#include "md5.h"
#ifndef LWIP_ICMP
#define LWIP_ICMP
#endif // LWIP_ICMP
#ifndef LWIP_TCP
#define LWIP_TCP
#endif // LWIP_TCP
#ifndef LWIP_UDP
#define LWIP_UDP
#endif // LWIP_UDP
int NetStatus;
// #include "com_poll.h"

#include "cfg.h"

#include "queue.h"
#include "t2n.h"

#include "ql_log.h"
#define T2N_LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "t2n", msg, ##__VA_ARGS__)
#define T2N_LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "t2n", msg, ##__VA_ARGS__)
#define T2N_LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "t2n", msg, ##__VA_ARGS__)
#define T2N_LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "t2n", msg, ##__VA_ARGS__)

// extern struct ip_globals ip_data;

extern void udp_ota_shell_init(void);

// extern LSAPI_Device_t *rtk_uart;
// extern LSAPI_Device_t *uart3;
extern int8_t gps_gga_req;

uint8_t t2n_hwaddr[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x06};
int GetCompileDateTime(char *szDateTime);
err_t t2n_send_to(struct pbuf *pkt_buf, const struct ip_addr *dest);
uint32_t utc_now(void);
void drp_on_data(u8_t ep, uint16_t id, uint8_t *data, uint16_t length);

extern void cmd_parse(uint8_t ep, uint8_t *data, uint16_t length);
uint32_t pass_update_time = 0;
// extern meter_cfg_t meter_cfg;
struct netif *t2n_if = NULL;

#define LDEBUG 1

// u8_t my_ip[4];
char imsi[18];
char imei[18];
char ccid[24];

const uint8_t hw_type[4] = {'E', 'C', '6', 'U'};
const uint8_t sw_type[4] = {'P', 'S', '0', '1'};
// const uint8_t sw_type[4] = {'C', 'P', '0', '0'};
// const uint8_t sw_type[4] = {'P', 'V', '1', '1'};
uint8_t drp_buf[1460];

dsc_cfg_t dsc_cfg = {
    .dtu_uid = "860000000000",
    .pass = "12345678"};

uint8_t my_class[4] = {0x01, 0x01, 0x81, 0x86};

// static char buffer[2048];
// static sockaddr_struct address= {0};
static struct t2n_context tun_args;
struct t2n_context *cntx_p = &tun_args;
extern u16_t gWordVar[];
err_t tun_start_login(struct t2n_context *cntx_p);
err_t tun_netif_init(struct netif *netif);

// static err_t t2n_output(struct netif *netif, struct pbuf *pb, struct ip_addr *ipaddr);
static err_t t2n_if_output(struct netif *netif, struct pbuf *pb, const ip4_addr_t *ipaddr);

// int t2n_send_heartbeat(u8_t cluster);

extern void ModBusUDPSlave_init(void);
extern ip4_addr_t current_iphdr_src;
/** Destination IP address of current_header */
extern ip4_addr_t current_iphdr_dest;
int net_dog_count;

uint32_t readUint32LE(uint8_t *buf, int offset)
{
    uint32_t ret =
        buf[offset] | (buf[offset + 1] << 8) | (buf[offset + 2] << 16) | (buf[offset + 3] << 24);
    return ret;
}
uint32_t readUint32BE(uint8_t *buf, int offset)
{
    uint32_t ret = buf[offset + 3] | (buf[offset + 2] << 8) | (buf[offset + 1] << 16) | (buf[offset + 0] << 24);
    return ret;
}
int32_t readInt32LE(uint8_t *buf, int offset)
{
    int32_t ret =
        buf[offset] | (buf[offset + 1] << 8) | (buf[offset + 2] << 16) | (buf[offset + 3] << 24);
    return ret;
}
int32_t readInt32BE(uint8_t *buf, int offset)
{
    int32_t ret =
        buf[offset + 3] | (buf[offset + 2] << 8) | (buf[offset + 1] << 16) | (buf[offset + 0] << 24);
    return ret;
}
uint16_t readUInt16LE(uint8_t *buf, int offset)
{
    uint16_t ret = buf[offset] | (buf[offset + 1] << 8);
    return ret;
}
uint16_t readUInt16BE(uint8_t *buf, int offset)
{
    uint16_t ret = buf[offset + 1] | (buf[offset] << 8);
    return ret;
}
int16_t readInt16LE(uint8_t *buf, int offset)
{
    int16_t ret = buf[offset] | (buf[offset + 1] << 8);
    return ret;
}
int16_t readInt16BE(uint8_t *buf, int offset)
{
    int16_t ret = buf[offset + 1] | (buf[offset] << 8);
    return ret;
}

void writeUInt16BE(uint8_t *buf, int offset, uint16_t data)
{
    buf[offset] = data >> 8;
    buf[offset + 1] = data & 0xff;
}
void writeUInt16LE(uint8_t *buf, int offset, uint16_t data)
{
    buf[offset] = data & 0xff;
    buf[offset + 1] = data >> 8;
}
void writeUInt32BE(uint8_t *buf, int offset, uint32_t data)
{
    buf[offset + 3] = (data >> 24) & 0xff;
    buf[offset + 2] = (data >> 16) & 0xff;
    buf[offset + 1] = (data >> 8) & 0xff;
    buf[offset + 0] = data & 0xff;
}
void writeUInt32LE(uint8_t *buf, int offset, uint32_t data)
{
    buf[offset] = data & 0xff;
    buf[offset + 1] = (data >> 8) & 0xff;
    buf[offset + 2] = (data >> 16) & 0xff;
    buf[offset + 3] = (data >> 24) & 0xff;
}

uint8_t readUInt8(uint8_t *buf, int offset)
{
    uint8_t ret = buf[offset];
    return ret;
}
int8_t readInt8(uint8_t *buf, int offset)
{
    int8_t ret = buf[offset];
    return ret;
}

void writeUInt8(uint8_t *buf, int offset, uint8_t data)
{
    buf[offset] = data;
}

extern int is_have_sim;
int net_status(void)
{
    // if (cntx_p->login_status == 100)
    // {
    //   return 3;
    // }
    // else if (NetStatus == 1)
    // {
    //   return 2;
    // }
    // return is_have_sim;
    return cntx_p->status;
}

typedef struct drp_queue
{
    QUEUE node;
    uint8_t endpoint;
    uint8_t flag;
    uint8_t try_times;
    uint8_t rsv1;
    uint16_t tran_id;
    uint16_t length;
    uint8_t data[4];
} drp_queue_t;

int drp_write(uint8_t endpoint, uint8_t flag, uint16_t tran_id, void *data, int length)
{
    uint32_t now = osiUpTime();
    if (now - pass_update_time > 30000)
    {
        gWordVar[REG_PASS_NUMBER] = 0;
        pass_update_time = now;
    }
    if (cntx_p->status == 5 || flag > 3)
    {
        drp_queue_t *drp_queue = malloc(sizeof(drp_queue_t) + length - 4);
        T2N_LOGI("drp_write [%s] endpoint=%d,length=%d tran_id=%d buf=%p", __FUNCTION__, endpoint,
                 length, tran_id, drp_queue);
        if (drp_queue != NULL)
        {
            drp_queue->endpoint = endpoint;
            drp_queue->flag = flag;
            drp_queue->try_times = 0;
            drp_queue->tran_id = tran_id;
            drp_queue->length = length;
            memcpy(drp_queue->data, data, length);
            QUEUE_INIT(&drp_queue->node);
            QUEUE_INSERT_TAIL(&cntx_p->drp_queue_head, &drp_queue->node);
            return 0;
        }
    }
    else
    {
        T2N_LOGI("drp_write error status=%d", cntx_p->status);
    }
    return -1;
}

int drp_quene_check(void)
{
}
u8_t hdr_chksum(void *hdr)
{
    u8_t *p = (u8_t *)hdr;
    u32_t sum = 0;
    int i;
    for (i = 0; i < 8; i++)
    {
        sum += *p++;
    }
    while (sum >> 8)
    {
        sum = (sum & 0xff) + (sum >> 8);
    }
    return (~sum) & 0xff;
}
int get_login_status(void)
{
    return (cntx_p->login_status == 100);
}
// extern err_t usb_eth_output(struct netif *netif, struct pbuf *p);

err_t tun_netif_init(struct netif *netif)
{
    if (netif == NULL)
    {
        T2N_LOGE("tun_netif_init netif is null");
        return ERR_ARG;
    }
    netif->name[0] = 'T';
    netif->name[1] = 'N';
    netif->num = 0;
    netif->output = t2n_if_output;
    // netif->linkoutput = usb_eth_output;
    netif->mtu = 1472;
    netif->flags = NETIF_FLAG_LINK_UP;
    return ERR_OK;
}
extern void ModBusTCPSlave_init(void);
extern void udp_ota_shell_init(void);
extern void Lis3dshTcp_init(void);
extern void NmeaTcp_init(void);
void t2n_init(void)
{

    // lwip_init();
    memset(cntx_p, 0, sizeof(*cntx_p));
    cntx_p->last_status = -1;
    load_dsc_cfg(&dsc_cfg);
    memcpy(cntx_p->tun_if.hwaddr, t2n_hwaddr, 6);
    IP4_ADDR(&cntx_p->tun_if.ip_addr.u_addr.ip4, 11, 0, 0, 0);
    IP4_ADDR(&cntx_p->tun_if.netmask.u_addr.ip4, 255, 255, 0, 0);
    IP4_ADDR(&cntx_p->tun_if.gw.u_addr.ip4, 0, 0, 0, 0);
    cntx_p->tun_if.num = 0;
    cntx_p->tun_if.hwaddr_len = NETIF_MAX_HWADDR_LEN;
    netif_add(&cntx_p->tun_if, &cntx_p->tun_if.ip_addr.u_addr.ip4, &cntx_p->tun_if.netmask.u_addr.ip4,
              &cntx_p->tun_if.gw.u_addr.ip4, NULL, tun_netif_init, NULL);
    netif_set_down(&cntx_p->tun_if);
    t2n_if = &cntx_p->tun_if;
    NetStatus = 0;
    ModBusUDPSlave_init();
    ModBusTCPSlave_init();
    NmeaTcp_init();
    // tftpd_init();
    udp_ota_shell_init();
    // Lis3dshTcp_init();
    QUEUE_INIT(&cntx_p->drp_queue_head);
    QUEUE_INIT(&cntx_p->drp_wack_queue_head);
}

extern uint32_t reboot_req;
extern int gps_report_period;
extern int real_report_period;
extern int gps_report_req;

int get_net_status(void)
{
    return cntx_p->status;
}

int drp_queue_check(void)
{
    //    err_t err;
    //    struct t2n_hdr *t2nhdr;
    //    struct pbuf *p_buf;
    //    struct t2n_hdr *t2n_hdr;
    //    if(QUEUE_EMPTY(&drp_queue_head)){
    //        return 0;
    //    }
    //    p_buf = pbuf_alloc(PBUF_TRANSPORT, sizeof(*t2n_hdr), PBUF_RAM);
    //    if(p_buf == NULL)
    //    {
    //        return ERR_MEM;
    //    }
    //    t2n_trace("[%s]", __FUNCTION__);
    //    t2n_hdr = (struct t2n_hdr *)p_buf->payload;
    //
    //
    //    if(pbuf_header(p_buf,DRP_HEAR_SIZE) > 0)
    //    {
    //        return ERR_BUF-100;
    //    }
    //    t2n_trace("[%s]", __FUNCTION__);
    //    t2nhdr = (struct t2n_hdr *)p_buf->payload;
    ////    QUEUE* q;
    ////
    ////    QUEUE_FOREACH(q,&drp_queue_head){
    ////        drp_queue_t *drp_node =  QUEUE_DATA(q, drp_queue_t, node);
    ////
    ////    }
    //    while(!QUEUE_EMPTY(&drp_queue_head)){
    //        QUEUE* q = QUEUE_HEAD(&drp_queue_head);
    //
    //    }
    //
    //    QUEUE_REMOVE(q);
    //    memcpy(report->data,drp_node->data,drp_node->length);
    //    report->endpoint = drp_node->endpoint;
    //    report->pack_id = ++cntx_p->tran_id;
    //    report->data_len = drp_node->length;
    //    report->flg = drp_node->flag;
    //    free(drp_node);
    //
    //
    //    t2nhdr = (struct t2n_hdr *)p_buf->payload;
    //    t2nhdr->flags = T2N_PROTO_DRP | (T2N_TTL&T2N_TTL_MASK);
    //    t2nhdr->src[0] = ip4_addr2(&cntx_p->tun_if.ip_addr);
    //    t2nhdr->src[1] = ip4_addr3(&cntx_p->tun_if.ip_addr);
    //    t2nhdr->src[2] = ip4_addr4(&cntx_p->tun_if.ip_addr);
    //    cntx_p->tran_id++;
    //    t2nhdr->prot.drp.pack_id = HTONS(cntx_p->tran_id);
    //    //t2nhdr->prot.drp.data_len = HTONS(p_buf->tot_len-sizeof(struct t2n_hdr));
    //
    //    DRP_LEN_SET(t2nhdr,p_buf->tot_len-DRP_HEAR_SIZE);
    //    DRP_TCF_SET(t2nhdr,tcf);
    //    t2nhdr->prot.drp.endpoint = endpoint;
    //    pbuf_header(p_buf,-8);
    //    t2nhdr->prot.drp.data_sum = inet_chksum_pbuf(p_buf);
    //    pbuf_header(p_buf,8);
    //    t2nhdr->prot.drp.hdr_sum = 0;
    //    t2nhdr->prot.drp.hdr_sum = hdr_chksum(t2nhdr);
    //    err = t2n_send_to(p_buf,NULL);
    //    cntx_p->drp_status = DRP_STATUS_WACK;
    return 0;
}

int fill_str(u8_t *buf, u8_t code, const char *str, int bufsize)
{
    int n;
    buf[0] = code;
    for (n = 2; n < bufsize; n++)
    {
        if (*str == '\0')
        {
            break;
        }
        buf[n] = *str++;
    }
    buf[1] = n - 2;
    return n;
}

int fill_tlv(u8_t *buf, u8_t type, const void *data, int length)
{
    int n;
    u8_t *p = (u8_t *)data;
    u8_t *dest;
    buf[0] = type;
    if (type > 200)
    {
        buf[1] = length & 0xff;
        buf[2] = (length >> 8);
        dest = buf + 3;
    }
    else
    {
        buf[1] = length & 0xff;
        dest = buf + 2;
    }
    for (n = 0; n < length; n++)
    {
        *dest++ = *p++;
    }
    return dest - buf;
}
err_t t2n_send_drp(struct pbuf *p_buf, u8_t endpoint, u8_t tcf, u16_t packet_id)
{
    err_t err;
    struct t2n_hdr *t2nhdr;
    if (endpoint == 0)
    {
        return ERR_VAL;
    }
    if (pbuf_header(p_buf, DRP_HEAR_SIZE) > 0)
    {
        return ERR_BUF - 100;
    }
    t2nhdr = (struct t2n_hdr *)p_buf->payload;
    t2nhdr->flags = T2N_PROTO_DRP | (T2N_TTL & T2N_TTL_MASK);
    t2nhdr->src[0] = ip4_addr2(&cntx_p->tun_if.ip_addr.u_addr.ip4);
    t2nhdr->src[1] = ip4_addr3(&cntx_p->tun_if.ip_addr.u_addr.ip4);
    t2nhdr->src[2] = ip4_addr4(&cntx_p->tun_if.ip_addr.u_addr.ip4);
    // cntx_p->tran_id++;
    t2nhdr->prot.drp.pack_id = HTONS(packet_id);
    // t2nhdr->prot.drp.data_len = HTONS(p_buf->tot_len-sizeof(struct t2n_hdr));
    DRP_LEN_SET(t2nhdr, p_buf->tot_len - DRP_HEAR_SIZE);
    DRP_TCF_SET(t2nhdr, tcf);
    t2nhdr->prot.drp.endpoint = endpoint;
    pbuf_header(p_buf, -8);
    t2nhdr->prot.drp.data_sum = inet_chksum_pbuf(p_buf);
    pbuf_header(p_buf, 8);
    t2nhdr->prot.drp.hdr_sum = 0;
    t2nhdr->prot.drp.hdr_sum = hdr_chksum(t2nhdr);
    err = t2n_send_to(p_buf, NULL);
    return err;
}
err_t drp_send_to(struct pbuf *p_buf, u8_t endpoint, u16_t length, u8_t tcf, u16_t packet_id)
{
    err_t err;
    struct t2n_hdr *t2nhdr;
    if (endpoint == 0)
    {
        return ERR_VAL;
    }
    if (pbuf_header(p_buf, DRP_HEAR_SIZE) > 0)
    {
        return ERR_BUF - 100;
    }
    if (p_buf->tot_len != (length + DRP_HEAR_SIZE))
    {
        if (p_buf->tot_len == p_buf->len)
        {
            p_buf->tot_len = p_buf->len = DRP_HEAR_SIZE + length;
        }
        else
        {
            // FIX tot_len != len
        }
    }
    t2nhdr = (struct t2n_hdr *)p_buf->payload;
    t2nhdr->flags = T2N_PROTO_DRP | (T2N_TTL & T2N_TTL_MASK);
    t2nhdr->src[0] = ip4_addr2(&cntx_p->tun_if.ip_addr.u_addr.ip4);
    t2nhdr->src[1] = ip4_addr3(&cntx_p->tun_if.ip_addr.u_addr.ip4);
    t2nhdr->src[2] = ip4_addr4(&cntx_p->tun_if.ip_addr.u_addr.ip4);
    cntx_p->tran_id++;
    t2nhdr->prot.drp.pack_id = HTONS(cntx_p->tran_id);
    // t2nhdr->prot.drp.data_len = HTONS(p_buf->tot_len-sizeof(struct t2n_hdr));

    DRP_LEN_SET(t2nhdr, p_buf->tot_len - DRP_HEAR_SIZE);
    DRP_TCF_SET(t2nhdr, tcf);
    t2nhdr->prot.drp.endpoint = endpoint;
    pbuf_header(p_buf, -8);
    t2nhdr->prot.drp.data_sum = inet_chksum_pbuf(p_buf);
    pbuf_header(p_buf, 8);
    t2nhdr->prot.drp.hdr_sum = 0;
    t2nhdr->prot.drp.hdr_sum = hdr_chksum(t2nhdr);
    err = t2n_send_to(p_buf, NULL);
    return err;
}

int t2n_send_heartbeat(uint8_t flags)
{
    int err;
    struct pbuf *p_buf;
    struct t2n_hdr *t2n_hdr;

    p_buf = pbuf_alloc(PBUF_TRANSPORT, HEADBREAT_PACK_SIZE, PBUF_RAM);
    if (p_buf == NULL)
    {
        return ERR_MEM;
    }
    t2n_hdr = (struct t2n_hdr *)p_buf->payload;
    t2n_hdr->flags = T2N_PROTO_HDP | (T2N_TTL & T2N_TTL_MASK);
    if (cntx_p->status != 5)
    {
        t2n_hdr->src[0] = 0;
        t2n_hdr->src[1] = 0;
        t2n_hdr->src[2] = 0;
    }
    else
    {
        t2n_hdr->src[0] = ip4_addr2(&cntx_p->tun_if.ip_addr.u_addr.ip4);
        t2n_hdr->src[1] = ip4_addr3(&cntx_p->tun_if.ip_addr.u_addr.ip4);
        t2n_hdr->src[2] = ip4_addr4(&cntx_p->tun_if.ip_addr.u_addr.ip4);
    }

    t2n_hdr->prot.hbp.pack_id = HTONS(cntx_p->tran_id);
    cntx_p->tran_id++;
    t2n_hdr->prot.hbp.status = flags;
    t2n_hdr->prot.hbp.hdr_sum = 0;
    t2n_hdr->prot.hbp.hdr_sum = hdr_chksum(t2n_hdr);
    err = t2n_send_to(p_buf, NULL);
    if (p_buf)
    {
        pbuf_free(p_buf);
    }
    cntx_p->last_hb_time = osiUpTime();
    return err;
}

void drp_input(struct pbuf *pkt_buf)
{
    int hdr_len = 4;
    u32_t addr;
    struct t2n_hdr *rxdrp = (struct t2n_hdr *)pkt_buf->payload;
    addr = (ip4_addr1(&cntx_p->tun_if.ip_addr.u_addr.ip4) << 24) + (rxdrp->src[0] << 16) + (rxdrp->src[1] << 8) +
           rxdrp->src[2];
    if (addr == htonl(cntx_p->tun_if.ip_addr.u_addr.ip4.addr) || (addr & 0xffffff) == 0xffffff)
    {

        u8_t endpoint = rxdrp->prot.drp.endpoint;
        u16_t pack_id = HTONS(rxdrp->prot.drp.pack_id);
        u16_t length = DRP_LEN_GET(rxdrp);
        u8_t tcf = DRP_TCF_GET(rxdrp);
        u8_t *rxdata = rxdrp->drp_data;
        cntx_p->tran_id = pack_id;
        T2N_LOGI("endpoint=%d,length=%d,tcf=%d\r\n", endpoint, length, tcf);
        if (endpoint == 0)
        {
            uint8_t tag = *rxdata++;
            int len = *rxdata++;
            if (len > length - 2)
            {
                len = length - 2;
            }
            if (tag == 1 && (tcf & TCF_ACK) == 0)
            {
                err_t err;
                struct t2n_hdr *txdrp;
                u8_t str_buf[32];
                struct pbuf *p_buf;
                u8_t *txdata;
                cntx_p->time_offset = (rxdata[0] << 24) + (rxdata[1] << 16) + (rxdata[2] << 8) + rxdata[3] -
                                      t2n_clock() / 1000;

                p_buf = pbuf_alloc(PBUF_TRANSPORT, 512, PBUF_RAM);
                if (p_buf == NULL)
                {
                    return;
                }
                txdrp = (struct t2n_hdr *)p_buf->payload;
                txdrp->flags = T2N_PROTO_DRP | (T2N_TTL & T2N_TTL_MASK);
                txdrp->src[0] = ip4_addr2(&cntx_p->tun_if.ip_addr.u_addr.ip4);
                txdrp->src[1] = ip4_addr3(&cntx_p->tun_if.ip_addr.u_addr.ip4);
                txdrp->src[2] = ip4_addr4(&cntx_p->tun_if.ip_addr.u_addr.ip4);
                // drphdr = (struct drp_hdr *)(tunhdr+1);
                txdrp->prot.drp.pack_id = rxdrp->prot.drp.pack_id;
                txdrp->prot.drp.endpoint = 0;
                txdata = txdrp->drp_data;
                {
                    int len;
                    u8_t salt[6];
                    MD5_CTX md5_ctx;
                    MD5Init(&md5_ctx);
                    salt[0] = cntx_p->tran_id >> 8;
                    salt[1] = cntx_p->tran_id & 0xff;
                    salt[2] = rxdata[0];
                    salt[3] = rxdata[1];
                    salt[4] = rxdata[2];
                    salt[5] = rxdata[3];
                    MD5Update(&md5_ctx, salt, 6);
                    len = strlen(dsc_cfg.pass);
                    MD5Update(&md5_ctx, (uint8_t *)dsc_cfg.pass, len);
                    MD5Final(str_buf, &md5_ctx);
                }
                if (imei[0] == '8' && imei[1] == '6')
                {
                    txdata += fill_str(txdata, 0x01, imei, 32);
                }
                else
                {
                    txdata += fill_str(txdata, 0x01, &ccid[2], 32);
                }

                txdata += fill_tlv(txdata, 0x02, str_buf, 16);
                txdata += fill_tlv(txdata, 0x03, hw_type, 4);
                txdata += fill_tlv(txdata, 0x04, sw_type, 4);
                length = txdata - txdrp->prot.drp.data;
                p_buf->len = p_buf->tot_len = length + 12;
                DRP_LEN_SET(txdrp, length);
                DRP_TCF_SET(txdrp, DRP_ACK);

                pbuf_header(p_buf, -8);
                txdrp->prot.drp.data_sum = inet_chksum_pbuf(p_buf);
                pbuf_header(p_buf, 8);
                txdrp->prot.drp.hdr_sum = 0;
                txdrp->prot.drp.hdr_sum = hdr_chksum(txdrp);
                err = t2n_send_to(p_buf, NULL);
                pbuf_free(p_buf);
                cntx_p->tran_id++;
            }
            else if (tag == 2)
            {
                int pos = 0;
                char status = 0;
                struct ip4_addr ip = {0};
                struct ip4_addr netmask = {0xffffffff};
                struct ip4_addr gw = {0};
                u8_t my_mask[4];
                u8_t my_gw[4];
                u8_t my_ip[4];
                u16_t subnet = 0;
                //
                if (pack_id != cntx_p->tran_id)
                {
                    T2N_LOGI("[%s] bad tran expected=%d,actual =%d\r\n", __FUNCTION__,
                             pack_id, cntx_p->tran_id);
                    goto end;
                }
                while (pos < len)
                {
                    u8_t length = rxdata[pos + 1];
                    if (length == 0)
                    {
                        break;
                    }
                    if (rxdata[pos] == 1)
                    {
                        status = (char)rxdata[pos + 2];
                    }
                    else if (rxdata[pos] == 2)
                    {
                        int i;
                        for (i = 0; i < 4; i++)
                        {
                            my_ip[i] = rxdata[pos + 2 + i];
                        }
                        IP4_ADDR(&ip, my_ip[0], my_ip[1], my_ip[2], my_ip[3]);
                    }
                    else if (rxdata[pos] == 3)
                    {
                        uint32_t mask;
                        int subnet_mask = rxdata[pos + 2 + 0];
                        int t2n_mask = rxdata[pos + 2 + 1];

                        mask = 0xffffffff << (32 - t2n_mask);
                        my_mask[0] = 0xff;
                        my_mask[1] = mask >> 16 & 0xff;
                        my_mask[2] = mask >> 8 & 0xff;
                        my_mask[3] = mask & 0xff;
                        IP4_ADDR(&netmask, my_mask[0], my_mask[1], my_mask[2], my_mask[3]);
                        if (subnet_mask > 8)
                        {
                            subnet_mask = 8;
                        }
                        cntx_p->subnet_mask = 0xffffffff >> subnet_mask;
                    }
                    else if (rxdata[pos] == 4)
                    {
                        int i;
                        for (i = 0; i < 4; i++)
                        {
                            my_gw[i] = rxdata[pos + 2 + i];
                        }
                        IP4_ADDR(&gw, my_gw[0], my_gw[1], my_gw[2], my_gw[3]);
                        // cntx_p->my_subnet = (my_mask[0] << 24) | (my_mask[1] << 16) | (my_mask[2] << 8) |
                        // my_mask[3];
                    }
                    pos += length + 2;
                }
                if (status == 100)
                {
                    T2N_LOGI("login suncess,IP=%d.%d.%d.%d mask=%d.%d.%d.%d\r\n", my_ip[0],
                             my_ip[1], my_ip[2], my_ip[3], my_mask[0], my_mask[1], my_mask[2], my_mask[3]);
                    // netif_set_addr(&cntx_p->tun_if,&ip,&netmask,&gw);
                    cntx_p->tun_if.ip_addr.type = IPADDR_TYPE_V4;
                    cntx_p->tun_if.ip_addr.u_addr.ip4.addr = ip.addr;
                    cntx_p->tun_if.netmask.type = IPADDR_TYPE_V4;
                    cntx_p->tun_if.netmask.u_addr.ip4.addr = netmask.addr;
                    cntx_p->tun_if.gw.type = IPADDR_TYPE_V4;
                    cntx_p->tun_if.gw.u_addr.ip4.addr = gw.addr;
                    netif_set_up(&cntx_p->tun_if);
                    cntx_p->is_login = 1;
                }
                else if (status == -1)
                {
                    T2N_LOGW("登陆失败,账号未注册\r\n");
                }
                else if (status == -2)
                {
                    T2N_LOGW("登陆失败,密码错误\r\n");
                }
                else if (status == -3)
                {
                    T2N_LOGW("登陆失败,账号已停用\r\n");
                }
                else if (status == -4)
                {
                    T2N_LOGW("登陆失败,设备类型不匹配\r\n");
                }
                else if (status == -9)
                {
                    T2N_LOGW("登陆失败,服务器内部错误\r\n");
                }
                else
                {
                    T2N_LOGW("登陆失败,未知错误 [%d]\r\n", status);
                }
                cntx_p->login_status = status;
                // last_send_time = time(NULL);
            }
        }
        else
        {
            drp_on_data(endpoint, pack_id, rxdata, length);
            if (!QUEUE_EMPTY(&cntx_p->drp_wack_queue_head) /* && cntx_p->unacked < DRP_CWND */)
            {
                QUEUE *q = QUEUE_HEAD(&cntx_p->drp_wack_queue_head);
                while (q != NULL)
                {
                    drp_queue_t *drp_node = QUEUE_DATA(q, drp_queue_t, node);
                    T2N_LOGI("drp_ack ep=%d,id=%d", drp_node->endpoint, drp_node->tran_id);
                    QUEUE *next = QUEUE_NEXT(q);
                    if (endpoint == drp_node->endpoint && pack_id >= drp_node->tran_id || drp_node->tran_id - pack_id >= 65520)
                    {
                        QUEUE_REMOVE(q);
                        free(drp_node);
                    }
                    q = next;
                }
            }
            // int n;
            // uint16_t pack_id = HTONS(rxdtp->prot.drp.pack_id);
            // for(n=0;n<REPORT_QUEUE_SIZE;n++)
            // {
            //     if(report_queue.data[n].pack_id == pack_id)
            //     {
            //         //report_queue.data[n].flg = 0;
            //         memset(&report_queue.data[n],0,16);
            //         report_queue.count--;
            //         break;
            //     }
            // }
        }
    }
    else
    {
        T2N_LOGI("[%s] addr not equ %8x\r\n", __FUNCTION__, addr);
    }
end:
    pbuf_free(pkt_buf);
}

void utp_input(struct pbuf *pkt_buf)
{
    u32_t addr;
    // 8_t *buffer = (u8_t *)pkt_buf->payload;
    struct t2n_hdr *rxutp = (struct t2n_hdr *)pkt_buf->payload;
    // Flags = buffer[0];
    addr = (ip4_addr1(&cntx_p->tun_if.ip_addr.u_addr.ip4) << 24) + (rxutp->src[0] << 16) + (rxutp->src[1] << 8) +
           rxutp->src[2];
    if (addr == htonl(cntx_p->tun_if.ip_addr.u_addr.ip4.addr) || (addr & 0xffffff) == 0xffffff)
    {
        uint8_t *rxdata = (uint8_t *)(rxutp + 1);
        int tcf = UTP_TCF_GET(rxutp);
        if (rxutp->prot.utp.dest_ep == 0x02)
        { // modbus utp
            int rxlen;
            int txlen;
            struct pbuf *p_buf;
            struct t2n_hdr *txutp;
            uint8_t *txdata;
            if (tcf != 3)
            {
                goto end;
            }
            p_buf = pbuf_alloc(PBUF_TRANSPORT, 1024 + 32, PBUF_RAM);
            if (p_buf == NULL)
            {
                goto end;
            }
            txutp = (struct t2n_hdr *)p_buf->payload;
            txutp->flags = T2N_PROTO_UTP | (T2N_TTL & T2N_TTL_MASK);
            txutp->src[0] = ip4_addr2(&cntx_p->tun_if.ip_addr.u_addr.ip4);
            txutp->src[1] = ip4_addr3(&cntx_p->tun_if.ip_addr.u_addr.ip4);
            txutp->src[2] = ip4_addr4(&cntx_p->tun_if.ip_addr.u_addr.ip4);
            txutp->prot.utp.dest[0] = rxutp->src[0];
            txutp->prot.utp.dest[1] = rxutp->src[1];
            txutp->prot.utp.dest[2] = rxutp->src[2];
            txutp->prot.utp.pack_id = rxutp->prot.utp.pack_id;
            rxlen = UTP_LEN_GET(rxutp);
            txutp->prot.utp.dest_ep = rxutp->prot.utp.src_ep;
            txutp->prot.utp.src_ep = rxutp->prot.utp.dest_ep;
            txdata = (uint8_t *)(rxutp + 1);
            txlen = ModbusSlaveProcess(txdata, rxdata, rxlen, 0);
            if (txlen > 0)
            {
                err_t err;
                UTP_TCF_LEN_SET(txutp, tcf | TCF_ACK, txlen);
                txutp->drp_data_sum = inet_chksum(&txutp->prot.utp.src_ep, txlen + 8);
                txutp->drp_hdr_sum = 0;
                txutp->drp_hdr_sum = hdr_chksum(txutp);
                p_buf->tot_len = p_buf->len = txlen + 16;
                err = t2n_send_to(p_buf, NULL);
            }
            pbuf_free(p_buf);
        }
        else if (rxutp->prot.utp.dest_ep == 1)
        { // file transport
        }
    }
end:
    pbuf_free(pkt_buf);
}

void hbp_input(struct pbuf *pkt_buf)
{
    struct t2n_hdr *t2nhdr = (struct t2n_hdr *)pkt_buf->payload;
    u16_t tran_id = HTONS(t2nhdr->prot.hbp.pack_id);
    if (t2nhdr->prot.hbp.status == 1 && tran_id != cntx_p->tran_id)
    {
        cntx_p->tran_id = tran_id;
        t2n_send_heartbeat(2);
    }
    pbuf_free(pkt_buf);
}

err_t rebuild_iphdr(struct pbuf *pkt_buf, u8_t proto)
{
    struct t2n_hdr *t2nhdr = (struct t2n_hdr *)pkt_buf->payload;
    struct ip_hdr *iphdr;
    // int hdr_len;
    struct ip4_addr src;
    struct ip4_addr dest;
    u8_t flags = t2nhdr->flags;
    // SetRxIco(1);
    cntx_p->RxIcoCnt = 2;
    IP4_ADDR(&src, 11, t2nhdr->src[0], t2nhdr->src[1], t2nhdr->src[2]);
    IP4_ADDR(&dest, 11, t2nhdr->prot.tun.dest[0], t2nhdr->prot.tun.dest[1], t2nhdr->prot.tun.dest[2]);
    T2N_LOGD("[%s] ip=%d.%d.%d.%d PROTO=%d\r\n",
             __FUNCTION__, ip4_addr1(&dest), ip4_addr2(&dest), ip4_addr3(&dest), ip4_addr4(&dest), flags & T2N_PROTO_MASK);
    pbuf_header(pkt_buf, 20 - 8);
    iphdr = (struct ip_hdr *)pkt_buf->payload;
    memset(iphdr, 0, 20);
    IPH_VHL_SET(iphdr, 4, 5);
    // IPH_TOS_SET(iphdr,0);
    IPH_TTL_SET(iphdr, flags & T2N_TTL_MASK);
    IPH_LEN_SET(iphdr, htons(pkt_buf->tot_len));
    iphdr->src.addr = src.addr;
    iphdr->dest.addr = dest.addr;
    IPH_PROTO_SET(iphdr, proto);
    // IPH_CHKSUM_SET(iphdr, 0);
    IPH_CHKSUM_SET(iphdr, inet_chksum(iphdr, 20));
    // ip_input(pkt_buf,)
    // ip_addr_copy(*ipX_2_ip(&current_iphdr_dest), dest);
    // ip_addr_copy(*ipX_2_ip(&ip_data.current_iphdr_src), src);
    // ip_addr_copy(current_iphdr_dest, dest);
    // ip_addr_copy(current_iphdr_src, src);
    // ip_addr_copy_from_ip4(ip_data.current_iphdr_dest, iphdr->dest);
    // ip_addr_copy_from_ip4(ip_data.current_iphdr_src, iphdr->src);
    // ip_data.current_netif = &self->tun_if;
    // ip_data.current_ip4_header = iphdr;
    // ip_data.current_ip_header_tot_len = IP_HLEN;

    // pbuf_header(pkt_buf,-20);
    return ERR_OK;
}

int t2n_input(struct pbuf *pkt_buf)
{
    // struct pbuf *pkt_buf = container_of(data, struct pbuf, payload);
    // T2N_LOGI("[%s]data = %p,pkt_buf=%p\r\n", __FUNCTION__, data,pkt_buf);
    // return;
    u8_t flags = *(u8_t *)pkt_buf->payload;
    // pkt_buf->tot_len = length;
    // pkt_buf->len = length;
    //  cntx_p->RxIcoCnt = 2;
    T2N_LOGI("[%s] revice length(len=%d)\r\n", __FUNCTION__, pkt_buf->tot_len);

    if (hdr_chksum(pkt_buf->payload) != 0)
    {
        T2N_LOGI("[%s] bad hdr checksum length=%d\r\n", __FUNCTION__,
                 pkt_buf->tot_len);
        goto err;
    }
    cntx_p->last_rx_time = osiUpTime();
    cntx_p->rpack_cnt++;
    net_dog_count = 0;
    // rx_packet_count++;
    // NetMonitor = 0;
    switch (flags & T2N_PROTO_MASK)
    {
    case T2N_PROTO_TCP:
        if (rebuild_iphdr(pkt_buf, IP_PROTO_TCP) == ERR_OK)
        {
            ip_input(pkt_buf, &cntx_p->tun_if);
        }
        break;
    case T2N_PROTO_UDP:
        if (rebuild_iphdr(pkt_buf, IP_PROTO_UDP) == ERR_OK)
        {
            ip_input(pkt_buf, &cntx_p->tun_if);
        }
        break;
    case T2N_PROTO_ICMP:
        if (rebuild_iphdr(pkt_buf, IP_PROTO_ICMP) == ERR_OK)
        {
            ip_input(pkt_buf, &cntx_p->tun_if);
        }
        break;
    case T2N_PROTO_IPV4:
        ip_input(pkt_buf, &cntx_p->tun_if);
        break;
    case T2N_PROTO_ETH:
        break;
    case T2N_PROTO_DRP:
        drp_input(pkt_buf);
        break;
    case T2N_PROTO_UTP:
        utp_input(pkt_buf);
        break;
    default:
        hbp_input(pkt_buf);
        break;
    }
    return 0;

err:
    pbuf_free(pkt_buf);
    return ERR_MEM;
}

err_t t2n_output(struct pbuf *pb, const struct ip4_addr *ipaddr)
{
    err_t err;
    struct t2n_hdr *t2nhdr;
    // u8_t *payload;

    struct pbuf *p;
    struct ip_hdr *iphdr = pb->payload;
    int hdr_len = 8;
    u8_t proto = IPH_PROTO(iphdr);
    struct ip4_addr src;
    struct ip4_addr dest;
    src.addr = iphdr->src.addr;
    dest.addr = iphdr->dest.addr;
    if (pbuf_header(pb, -IP_HLEN) > 0)
    {
        err = ERR_BUF;
        goto err_out2;
    }
    // payload = pb->payload;
    if (pbuf_header(pb, 14 + IP_HLEN + UDP_HLEN + hdr_len) > 0)
    {
        p = pbuf_alloc(PBUF_TRANSPORT, hdr_len, PBUF_RAM);
        if (p == NULL)
        {
            err = ERR_MEM;
            goto err_out2;
        }
        pbuf_chain(p, pb);
    }
    else
    {
        pbuf_header(pb, -(14 + IP_HLEN + UDP_HLEN));
        p = pb;
    }
    t2nhdr = (struct t2n_hdr *)p->payload;
    t2nhdr->flags = (T2N_TTL & T2N_TTL_MASK);
    t2nhdr->src[2] = ip4_addr4(&src);
    t2nhdr->src[1] = ip4_addr3(&src);
    t2nhdr->src[0] = ip4_addr2(&src);
    t2nhdr->prot.tun.dest[2] = ip4_addr4(&dest);
    t2nhdr->prot.tun.dest[1] = ip4_addr3(&dest);
    t2nhdr->prot.tun.dest[0] = ip4_addr2(&dest);
    switch (proto)
    {
    case IP_PROTO_UDP:
    {
        t2nhdr->flags = T2N_PROTO_UDP | (T2N_TTL & T2N_TTL_MASK);
    }
    break;
    case IP_PROTO_TCP:
    {
        t2nhdr->flags = T2N_PROTO_TCP | (T2N_TTL & T2N_TTL_MASK);
    }
    break;
    case IP_PROTO_ICMP:
    {
        t2nhdr->flags = T2N_PROTO_ICMP | (T2N_TTL & T2N_TTL_MASK);
    }
    break;
    default:
        err = ERR_VAL;
        goto err_out1;
    }
    t2nhdr->prot.tun.hdr_sum = 0;
    t2nhdr->prot.tun.hdr_sum = hdr_chksum(t2nhdr);
    err = t2n_send_to(p, ipaddr);
err_out1:
    if (p != pb)
    {
        pbuf_free(p);
    }
err_out2:
    return err;
}

static err_t t2n_if_output(struct netif *netif, struct pbuf *pb, const ip4_addr_t *ipaddr)
{
    err_t err;
    (void)ipaddr;
    // struct t2n_hdr *t2nhdr;
    // struct pbuf *p;
    // struct ip_hdr *iphdr = pb->payload;
    // int hdr_len = 8;
    //  u8_t proto = IPH_PROTO(iphdr);
    T2N_LOGI("t2n_output ip=%08x,if.addr=%08x,subnet_mask=%08x", ipaddr->addr, cntx_p->tun_if.ip_addr.u_addr.ip4.addr, cntx_p->subnet_mask);
    // if ((ipaddr->addr & cntx_p->subnet_mask) == (cntx_p->tun_if.ip_addr.u_addr.ip4.addr & cntx_p->subnet_mask))
    // {
    //     // netif->linkoutput(netif,pb);
    //     etharp_output(netif, pb, ipaddr);
    // }
    // else
    // {
    t2n_output(pb, ipaddr);
    // }
}
// svoid netif_init(void) {}
u32_t sys_now(void)
{
    // uint32_t now = time_offset + eat_get_duration_ms(0)/1000;
    return osiUpTime();
}
// u32_t get_myip(char *ip){
//     if(ip != NULL){
//         ip[0] = ip4_addr1(&cntx_p->tun_if.ip_addr.u_addr.ip4);
//         ip[1] = ip4_addr2(&cntx_p->tun_if.ip_addr.u_addr.ip4);
//         ip[2] = ip4_addr3(&cntx_p->tun_if.ip_addr.u_addr.ip4);
//         ip[3] = ip4_addr4(&cntx_p->tun_if.ip_addr.u_addr.ip4);
//     }
//     return cntx_p->tun_if.ip_addr.u_addr.ip4;
// }
uint32_t utc_now(void)
{
    uint32_t now;
    now = cntx_p->time_offset + osiUpTime() / 1000;
    return now;
}
// #define PP_HTONS(x) ((((x)&0x00ffUL) << 8) | (((x)&0xff00UL) >> 8))
// #define PP_NTOHS(x) PP_HTONS(x)
// #define PP_HTONL(x)                                                                       \
//     ((((x)&0x000000ffUL) << 24) | (((x)&0x0000ff00UL) << 8) | (((x)&0x00ff0000UL) >> 8) | \
//      (((x)&0xff000000UL) >> 24))
// #define PP_NTOHL(x) PP_HTONL(x)
// u32_t lwip_htonl_use(u32_t a)
// {
//     return PP_HTONL(a);
// }
// u16_t lwip_htons(u16_t a)
// {
//     return PP_HTONS(a);
// }
// u16_t lwip_ntohs(u16_t a)
// {
//     return PP_NTOHS(a);
// }
// u32_t lwip_htonl(u32_t a)
// {
//     return PP_HTONL(a);
// }
// u32_t lwip_ntohl(u32_t a)
// {
//     return PP_NTOHL(a);
// }
struct drp_pcb
{
    uint8_t endpoint;
    void (*recv)(uint8_t *data, uint16_t length, uint16_t packet_id, void *arg);
    void *arg;
    struct drp_pcb *next;
};
extern void rtk_rtcm_intput(uint8_t *data, int length);
void drp_on_data(u8_t ep, uint16_t id, uint8_t *data, uint16_t length)
{
    if (ep == 13)
    {
        int pass = 0;
        *(data + length) = '\0';
        if (sscanf(data, "pass=%d", &pass) == 1)
        {
            gWordVar[REG_PASS_NUMBER] = pass;
            pass_update_time = osiUpTime();
        }
    }
    else if (ep == 10)
    {
        T2N_LOGI("rtcm recv length:%d\n", length);
        rtk_rtcm_intput(data, length);
    }
    else if (ep == 11)
    {
        T2N_LOGI("GGA_REQ length:%d\n", length);
        if (!strncmp((char *)data, "GGA_REQ", 7))
        {
            gps_gga_req = 1;
        }
    }
    else if (ep == 1)
    { // 解析命令并处理
      // cmd_parse(ep, data, length);
    }
}

void t2n_poll_queue_process(uint32_t now)
{
    if (!QUEUE_EMPTY(&cntx_p->drp_queue_head) && (now - cntx_p->last_drp_send > 500) /*&&
        cntx_p->unacked < DRP_CWND*/
    )
    {
        struct pbuf *p_buf = pbuf_alloc(PBUF_TRANSPORT, 1400, PBUF_RAM);
        int offset = 0;
        if (p_buf != NULL)
        {
            u8_t tcf = 3;
            u8_t endpoint;
            struct t2n_hdr *t2nhdr = (struct t2n_hdr *)p_buf->payload;
            uint8_t *pBuf = &t2nhdr->prot.drp.endpoint;
            t2nhdr->flags = T2N_PROTO_DRP | (T2N_TTL & T2N_TTL_MASK);
            t2nhdr->src[0] = ip4_addr2(&cntx_p->tun_if.ip_addr.u_addr.ip4);
            t2nhdr->src[1] = ip4_addr3(&cntx_p->tun_if.ip_addr.u_addr.ip4);
            t2nhdr->src[2] = ip4_addr4(&cntx_p->tun_if.ip_addr.u_addr.ip4);
            do
            {
                QUEUE *q = QUEUE_HEAD(&cntx_p->drp_queue_head);
                drp_queue_t *drp_node = QUEUE_DATA(q, drp_queue_t, node);
                T2N_LOGI("drp_node ep=%d,len=%d,id=%d", drp_node->endpoint, drp_node->length,
                         drp_node->tran_id);
                if ((offset + drp_node->length) > (p_buf->tot_len - 12))
                {
                    break;
                }
                writeUInt8(pBuf, 0, drp_node->endpoint);
                writeUInt16BE(pBuf, 1, drp_node->tran_id);
                writeUInt16BE(pBuf, 3, (drp_node->length & 0xfff) | (tcf << 12));
                memcpy(pBuf + 5, drp_node->data, drp_node->length);
                offset += 5 + drp_node->length;
                pBuf += 5 + drp_node->length;
                QUEUE_REMOVE(q);
                if (drp_node->flag == 0)
                {
                    free(drp_node);
                }
                else
                {
                    free(drp_node);
                    // QUEUE_INIT(&drp_node->node);
                    // QUEUE_INSERT_TAIL(&cntx_p->drp_wack_queue_head, &drp_node->node);
                }
            } while (!QUEUE_EMPTY(&cntx_p->drp_queue_head));

            p_buf->len = p_buf->tot_len = pBuf - (uint8_t *)p_buf->payload;
            pbuf_header(p_buf, -8);
            t2nhdr->prot.drp.data_sum = inet_chksum_pbuf(p_buf);
            pbuf_header(p_buf, 8);
            t2nhdr->prot.drp.hdr_sum = 0;
            t2nhdr->prot.drp.hdr_sum = hdr_chksum(t2nhdr);
            cntx_p->last_drp_send = now;
            t2n_send_to(p_buf, NULL);
            pbuf_free(p_buf);
            cntx_p->unacked++;
        }
    }
}

// void t2n_ip_input(struct pbuf *pkt_buf)
// {
//     struct ip_hdr *iphdr = (struct ip_hdr *)pkt_buf->payload;
//     if ((iphdr->dest.addr & cntx_p->subnet_mask) == (cntx_p->tun_if.ip_addr.u_addr.ip4.addr & cntx_p->subnet_mask))
//     {
//         ip_input(pkt_buf, &cntx_p->tun_if);
//     }
//     // else if ((iphdr->dest.addr & cntx_p->tun_if.netmask.u_addr.ip4.addr) == (cntx_p->tun_if.ip_addr.u_addr.ip4.addr & cntx_p->tun_if.netmask.u_addr.ip4.addr))
//     // {
//     //     t2n_output(pkt_buf, &iphdr->dest);
//     // }
//     else
//     {
//         pbuf_free(pkt_buf);
//     }
// }
