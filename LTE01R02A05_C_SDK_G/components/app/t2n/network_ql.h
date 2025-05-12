#ifndef __NETWORK_LS_H__
#define __NETWORK_LS_H__

#include "stdint.h"
// #include "lwip_t2n/netif.h"
// #include "lsapi_sock.h"
#define SOCK_RAW_REV_DATA_IND (LS_API_EV_BASE + 1)
#define USB_NET_REV_DATA_IND (LS_API_EV_BASE + 3)

#define FALSE 0
#define TRUE 1
typedef struct {
  char host[2][32];
  uint16_t port[2];
  uint8_t dtu_type[4];
  char dtu_uid[16];
  char pass[16];
  char APN[16];
  char User[16];
  char Pwd[16];
} dsc_cfg_t;

// struct net_connection_ctx {
//   // uint8_t my_ipaddr[4];
//   // struct ip_addr netmask;
//   // struct ip_addr gateway;
//   // struct ip_addr server_ip;
//   // struct ip_addr remote_ip;
//   // struct udp_pcb *udp_pcb;
//   LSAPI_SOCK_TcpipSocketAddr_t LocalAddr;
//   LSAPI_SOCK_TcpipSocketAddr_t server_addr;
//   char imei[16];
//   int udp_sock_fd;
//   int ser_sel;
//   // uint16_t server_port;
//   // uint16_t remote_port;
//   uint16_t tran_id;
//   uint32_t last_gps_tick;
//   uint32_t last_report_tick;
//   uint32_t rpack_cnt;
//   uint32_t tpack_cnt;
//   uint32_t hb_delay;

//   uint32_t time_out_cnt;
//   uint32_t time_delay_cnt;
//   uint32_t tx_time_cnt;
//   uint32_t rx_time_cnt;
//   int8_t last_status;
//   int8_t status;
//   int8_t next_status;
//   uint8_t gsm_state;
//   uint8_t gprs_state;
//   uint8_t dns_state;
//   uint8_t socket_err_cnt;
//   uint8_t gsm_err_cnt;
//   uint8_t gprs_err_cnt;
//   uint8_t dns_err_cnt1;
//   uint8_t dns_err_cnt2;
//   uint8_t login_err_cnt1;
//   uint8_t login_err_cnt2;
//   int8_t login_status;
//   uint8_t TxIcoCnt;
//   uint8_t RxIcoCnt;
//   uint16_t dns_id;
//   int is_login;
//   uint32_t last_tx_time;
//   uint32_t last_rx_time;
//   uint32_t time_offset;
//   uint32_t unacked;
//   uint32_t last_drp_send;
//   // QUEUE drp_queue_head;
// };

// extern int t2n_input(struct pbuf *pkt_buf,uint8_t *addr, uint16_t port);
// extern void t2n_poll(void);
extern int t2n_sendto_serve(void* data, int length);
void t2n_thread(void* param);
 
// int usb_send_packet(struct pbuf *p_buf);
int ls_usb_eth_init(void);
#endif  // __NETWORK_LS_H__
