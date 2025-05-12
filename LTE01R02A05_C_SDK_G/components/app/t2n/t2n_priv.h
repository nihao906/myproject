#ifndef _TUN_EAT_H
#define _TUN_EAT_H

// #include "lwip/def.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/ip4_addr.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/memp.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "queue.h"

typedef struct report_data
{
    uint8_t flg;
    uint8_t rsv;
    uint8_t try_times;
    uint8_t endpoint;
    uint32_t last_send_time;
    uint16_t pack_id;
    uint16_t data_len;
    uint8_t data[1024];
} report_data_t;
extern report_data_t *report_queue_get(void);

extern uint8_t drp_buf[];
int drp_write(uint8_t endpoint, uint8_t flag, uint16_t tran_id, void *data, int length);

#define IP_PROTO_UDP 17
#define IP_PROTO_TCP 6

#define T2N_PROTO_MASK 0x70
#define T2N_PROTO_HDP 0x00
#define T2N_PROTO_DRP 0x10
#define T2N_PROTO_UTP 0x20
#define T2N_PROTO_ICMP 0x30
#define T2N_PROTO_UDP 0x40
#define T2N_PROTO_TCP 0x50
#define T2N_PROTO_IPV4 0x60
#define T2N_PROTO_ETH 0x70

#define T2N_TTL_MASK 0x07

#define T2N_TTL 0x07

#define HTONS(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))

struct drp
{
    unsigned short data_sum;
    unsigned char hdr_sum;
    unsigned char endpoint;
    unsigned short pack_id;
    unsigned short tcf_len;
    unsigned char data[4];
};

struct hbp
{
    unsigned char hdr_sum;
    unsigned char status;
    unsigned short pack_id;
};

struct utp
{
    unsigned char hdr_sum;   // 头部8字节校验码
    unsigned char dest[3];   // 目的地址
    unsigned short data_sum; // 除了头部外的校验玛
    unsigned char src_ep;    // 源端点号
    unsigned char dest_ep;   // 目的端点号
    unsigned short pack_id;  // 包序号
    unsigned short tcf_len;  // 传输控制和数据长度
};

struct tun
{
    unsigned char hdr_sum;
    unsigned char dest[3];
};
union prot
{
    struct hbp hbp; // 心跳包
    struct drp drp; // 数据上报协议
    struct tun tun; // 隧道透传
    struct utp utp; // 设备到设备传输协议
};

struct t2n_hdr
{
    unsigned char flags;  // 控制位
    unsigned char src[3]; // 源地址
#define drp_data_len prot.drp.data_len
#define drp_data_sum prot.drp.data_sum
#define drp_endpoint prot.drp.endpoint
#define drp_hdr_sum prot.drp.hdr_sum
#define drp_pack_id prot.drp.pack_id
#define drp_data prot.drp.data

#define utp_data_len prot.drp.data_len
#define utp_data_sum prot.drp.data_sum
#define utp_src_ep prot.drp.endpoint
#define utp_dst_ep prot.drp.hdr_sum
#define utp_pack_id prot.drp.pack_id
#define drp_data prot.drp.data
    union prot prot; // 协议载荷，由flags控制
};

// #define drp_data_len prot.drp.data_len
// #define drp_data_sum prot.drp.data_sum
// #define drp_endpoint prot.drp.endpoint
// #define drp_hdr_sum prot.drp.hdr_sum
// #define drp_pack_id prot.drp.pack_id

#define DRP_ACK 0x8      // 传输控制位 1 ACK包，0 数据包
#define TCF_ACK 0x8      // 传输控制位 1 ACK包，0 数据包
#define TCF_NEED_ACK 0x4 // 传输控制位 1 需要服务器ACK 0不需要

#define DRP_LEN_GET(t2n_hdr) (HTONS(t2n_hdr->prot.drp.tcf_len) & 0xfff)
#define DRP_LEN_SET(t2n_hdr, len) \
    (t2n_hdr->prot.drp.tcf_len = (t2n_hdr->prot.drp.tcf_len & 0x00f0) | HTONS((len) & 0xfff))
#define DRP_TCF_GET(t2n_hdr) ((t2n_hdr->prot.drp.tcf_len >> 4) & 0x0f)
#define DRP_TCF_SET(t2n_hdr, tcf) \
    (t2n_hdr->prot.drp.tcf_len = (t2n_hdr->prot.drp.tcf_len & 0xff0f) | (((tcf) << 4) & 0xf0))

#define DRP_TCF_LEN_SET(t2n_hdr, tcf, length) \
    t2n_hdr->prot.drp.tcf_len = HTONS((((tcf) << 12) & 0xf000) | ((length) & 0x0fff))

#define UTP_LEN_GET(t2n_hdr) (HTONS(t2n_hdr->prot.utp.tcf_len) & 0xfff)
#define UTP_LEN_SET(t2n_hdr, len) \
    (t2n_hdr->prot.utp.tcf_len = (t2n_hdr->prot.utp.tcf_len & 0x00f0) | HTONS((len) & 0xfff))
#define UTP_TCF_GET(t2n_hdr) (t2n_hdr->prot.utp.tcf_len >> 4)
#define UTP_TCF_SET(t2n_hdr, tcf) \
    (t2n_hdr->prot.utp.tcf_len = (t2n_hdr->prot.utp.tcf_len & 0xff0f) | (((tcf) << 4) & 0xf0))
#define UTP_TCF_LEN_SET(t2n_hdr, tcf, length) \
    t2n_hdr->prot.utp.tcf_len = HTONS((((tcf) << 12) & 0xf000) | ((length) & 0x0fff))
// #define UTP_SACK_GET(t2n_hdr)		((t2n_hdr->prot.utp.tcf_len & 0x08) != 0)
// #define UTP_SACK_SET(t2n_hdr,tcf)	(t2n_hdr->prot.utp.tcf_len = (t2n_hdr->prot.utp.tcf_len &
// 0xff0f)|(((tcf)<<4)&0xf0))

#define DRP_CWND 4

#define HEADBREAT_PACK_SIZE 8
#define MIN_DRP_PACK_SIZE 12

#define DRP_HEAR_SIZE 12

#define DRP_STATUS_IDLE 1
#define DRP_STATUS_WACK 2
#define DRP_STATUS_ERROR 2

struct t2n_context
{
    // u8_t my_ipaddr[4];
    u32_t my_addr;
    u32_t subnet_mask;
    // struct ip_addr netmask;
    // struct ip_addr gateway;
    struct ip_addr server_ip;
    // struct ip_addr remote_ip;
    // struct udp_pcb *udp_pcb;
    struct netif tun_if;
    struct udp_pcb *udp_pcb;
    // sockaddr_struct server_addr;
    u16_t server_port;
    uint8_t ser_sel;
    uint8_t sim_id;
    int profile_idx;
    // u16_t remote_port;
    u16_t tran_id;
    u32_t last_gps_tick;
    u32_t last_report_tick;
    u32_t rpack_cnt;
    u32_t tpack_cnt;
    u32_t hb_delay;

    u32_t time_out_cnt;
    u32_t time_delay_cnt;
    u32_t tx_time_cnt;
    u32_t rx_time_cnt;
    s8_t last_status;
    s8_t status;
    s8_t next_status;
    u8_t gsm_state;
    u8_t gprs_state;
    u8_t dns_state;
    u8_t socket_err_cnt;
    u8_t gsm_err_cnt;
    u8_t gprs_err_cnt;
    u8_t dns_err_cnt1;
    u8_t dns_err_cnt2;
    u8_t login_err_cnt1;
    u8_t login_err_cnt2;
    s8_t login_status;
    u8_t TxIcoCnt;
    u8_t RxIcoCnt;
    u16_t dns_id;
    int is_login;
    u32_t unacked;
    int64_t last_tx_time;
    int64_t last_rx_time;
    int64_t last_hb_time;
    int64_t time_offset;
    int64_t last_drp_send;
    QUEUE drp_queue_head;
    QUEUE drp_wack_queue_head;
    // ## new add
    // LSAPI_SOCK_TcpipSocketAddr_t LocalAddr;
    // LSAPI_SOCK_TcpipSocketAddr_t server_addr;
    char imei[16];
    int udp_sock_fd;
    // int ser_sel;
};
typedef struct t2n_context t2n_context_t;
// extern int t2n_input(struct pbuf *pkt_buf);
// extern void t2n_poll(void);

#endif // _TUN_EAT_H
