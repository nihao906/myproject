
#include <stdint.h>
#include <string.h>

#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "ql_api_osi.h"
#include "ql_log.h"

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "lis3dsh_tcp", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO,  "lis3dsh_tcp", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN,  "lis3dsh_tcp", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "lis3dsh_tcp", msg, ##__VA_ARGS__)

extern struct netif *t2n_if;
extern ql_sem_t raw_data_write_sema;
extern ql_sem_t fft_data_write_sema;

// 链表，记录连接的客户端
struct clientNode
{
  struct tcp_pcb *pcb;
  struct clientNode *next;
};

struct clientNode client_list;                // 客户端链表头结点
struct clientNode *client_r_p = &client_list; // 客户端链表尾指针

void client_list_add(struct tcp_pcb *pcb)
{
  struct clientNode *client = (struct clientNode *)malloc(sizeof(struct clientNode));
  client->pcb = pcb;
  client->next = NULL;
  client_r_p->next = client;
  client_r_p = client;
}

void client_list_delete(struct tcp_pcb *pcb)
{
  struct clientNode *p = &client_list;
  while (p->next)
  {
    if (p->next->pcb == pcb)
    {
      struct clientNode *q = p->next;
      if (q == client_r_p)
      {
        client_r_p = p;
      }
      p->next = q->next;
      free(q);
      return;
    }
    p = p->next;
  }
}

void cilent_list_delete_all(void)
{
  struct clientNode *p = &client_list;
  while (p->next)
  {
    struct clientNode *q = p->next;
    p->next = q->next;
    free(q);
  }
  client_r_p = &client_list;
}

void tcp_write_to_cilents(uint8_t *data, int len)
{
    struct clientNode *p = client_list.next;
    int ret = 0;
    while (p) 
    {
        // LOGI("write to 0x%x", p->pcb);
        ret = tcp_write(p->pcb, data, len, 1);
        if (ret < 0)
        {
            LOGE("write error:0x%x\n", ret);
        }
        else 
        {
            tcp_output(p->pcb);
        }
        // LOGI("write to 0x%x ok!", p->pcb);
        p = p->next;
    }
}


static err_t Lis3dshTcp_recv(void *arg, struct tcp_pcb *newpcb, struct pbuf *p, err_t err)
{
    LOGI("recive");
    if (p != NULL)
    {
        uint8_t *txbuf;
        uint8_t *rxbuf;
        int txlen, rxlen;
      
        tcp_recved(newpcb, p->tot_len);
        rxbuf = (uint8_t *)p->payload;
        rxlen = p->len;

        if (memcmp(rxbuf, "-h", 2) == 0)
        {
            tcp_write(newpcb, "hello", 6, 1);
            tcp_output(newpcb);
        }
       
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
        /* We free the allocated memory and we close the connection */
        LOGI("connect closing");
        // LOGW("remote end is closing the connection\n");
        tcp_err(newpcb, NULL);
        tcp_recv(newpcb, NULL);
        tcp_poll(newpcb, NULL, 2); // 60 second timeout
        client_list_delete(newpcb);
        err_t err_p = tcp_close(newpcb);
        if (ERR_OK != err_p)
        {
            LOGE("close error. err:%d\n", err_p);
        }
        return err_p;
    }
    return ERR_OK;
}

static void Lis3dshTcp_conn_err(void *arg, err_t err)
{
    LOGI("conn_err");
    cilent_list_delete_all();
    // LOGE("connection error, err:%d\n", err);
}

// static err_t Lis3dshTcp_poll(void *arg, struct tcp_pcb *newpcb)
// {
//     // if (newpcb)
//     // {
//     //     LOGE("close pcb\n");
//     //     tcp_close(newpcb);
//     //     newpcb = NULL;
//     // }
//     return ERR_OK;
// }

static err_t Lis3dshTcp_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    LOGI("accept 0x%x", pcb);
    tcp_err(pcb, Lis3dshTcp_conn_err);
    tcp_recv(pcb, Lis3dshTcp_recv);
    // tcp_poll(pcb, Lis3dshTcp_poll, 10); // 60 second timeout
    /* Send out the first message */
    // tcp_write(pcb, GREETING, strlen(GREETING), 1);
    client_list_add(pcb);
    return ERR_OK;
}


// #define N_SAMPLES 1024
// extern float x1[N_SAMPLES * 2];
// extern float x2[N_SAMPLES * 2];
// extern float x3[N_SAMPLES * 2];
// extern int buf_r;
// extern int fft_h;

// static void lisdsh_send_data_task(void)
// {
//     int timeout = 0;
//     int last_buf_r = 0;
//     uint8_t log_buf[256];

//     while (1)
//     {
//         // 发送原始数据
//         tcp_write_to_cilents("send raw: wait seam", strlen("send raw: wait seam") + 1);
//         float x1_v = 0, x2_v = 0, x3_v = 0;
//         int send_raw_flag = 0;
//         if (ql_rtos_semaphore_wait(raw_data_write_sema, 100) == QL_OSI_SUCCESS)
//         {
//             tcp_write_to_cilents("send raw: seam get!!!!!!!!!!!!!!!!!!!!!!!!!!", strlen("send raw: seam get!!!!!!!!!!!!!!!!!!!!!!!!!!") + 1);
//             // if (buf_r != last_buf_r)
//             // {
//             //     last_buf_r = (buf_r - 1 + N_SAMPLES * 2) % (N_SAMPLES * 2);
//             //     x1_v = x1[last_buf_r];
//             //     x2_v = x2[last_buf_r];
//             //     x3_v = x3[last_buf_r];
//             //     send_raw_flag = 1;
//             //     // sprintf(log_buf, "send raw: buf[%d]=%f, %f, %f", last_buf_r, x1_v, x2_v, x3_v);
//             //     // tcp_write_to_cilents(log_buf, strlen(log_buf) + 1);
//             // }
//             ql_rtos_semaphore_release(raw_data_write_sema);
//             // tcp_write_to_cilents("send raw: seam release", strlen("send raw: seam release") + 1);
//         }
//         if (send_raw_flag == 1)
//         {
//             char buf[100];
//             // sprintf(buf, "%f,%f,%f\n", x1_v, x2_v, x3_v);
//             // tcp_write_to_cilents(buf, strlen(buf) + 1);
//         }

//         osiThreadSleep(100);
//     }

// exit:
//   ql_rtos_task_delete(NULL);
// }

void Lis3dshTcp_init(void)
{
    QlOSStatus err = 0;
    struct tcp_pcb *pcb;
    pcb = tcp_new();
    tcp_bind(pcb, NULL, 9998);
    // tcp_bind_netif(pcb, t2n_if);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, Lis3dshTcp_accept);

    // ql_task_t lisdsh_send_data_task_ref;
    // err = ql_rtos_task_create(&lisdsh_send_data_task_ref, 4096, APP_PRIORITY_BELOW_NORMAL, "lis3dsh", lisdsh_send_data_task, NULL, 5);
    // if (err != QL_OSI_SUCCESS)
    // {
    //     LOGI("creat lisdsh_send_data_task fail err = %d", err);
    // }
    // LOGI("creat lisdsh_send_data_task sucess!");
}
