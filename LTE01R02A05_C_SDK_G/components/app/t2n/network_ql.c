
#include "network_ql.h"

#include <stdint.h>
#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_nw.h"
#include "ql_api_sim.h"
#include "ql_api_usbnet.h"
#include "ql_api_datacall.h"
#include "osi_api.h"
#include "errno.h"
#include "t2n_priv.h"
#include "ModbusS.h"
#include "ql_log.h"
#include "udp_ota_shell.h"
#include "ql_gpio.h"

#define T2N_LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "t2n_nw", msg, ##__VA_ARGS__)
#define T2N_LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "t2n_nw", msg, ##__VA_ARGS__)
#define T2N_LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "t2n_nw", msg, ##__VA_ARGS__)
#define T2N_LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "t2n_nw", msg, ##__VA_ARGS__)

// static struct net_connection_ctx net_cntx;
// static struct net_connection_ctx *cntx_p = &net_cntx;
// #include "t2n_priv.h"
extern ql_task_t usb_acm_slip_task;

extern struct t2n_context *cntx_p;
osiThread_t *t2n_thread_handle = NULL;
osiTimer_t *t2n_poll_timer = NULL;
extern char imei[16];
extern char ccid[24];
char *defalut_server = "rtk.mcgps.cn";
// char *defalut_server = "106.12.253.52";
const int defalut_port = 6051;
// char *defalut_server = "git.mcxa.cn";
// const int defalut_port = 6051;
// extern sys_data_t sys_data; // 系统数据

static uint8_t g_sim_id = 0;

#define QL_SIM_NUM 2
#define LINFO 1
extern dsc_cfg_t dsc_cfg;
typedef struct
{
    bool state;
    ql_sem_t act_sem;
    ql_sem_t deact_sem;
} datacall_context_s;

datacall_context_s datacall_ctx[QL_SIM_NUM][PROFILE_IDX_NUM] = {0};
extern int t2n_send_heartbeat(uint8_t flags);
extern int t2n_input(struct pbuf *pkt_buf);
extern void t2n_poll_queue_process(uint32_t now);

#define LDEBUG 1
static void hostname_notify_cb(const char *name, const uint32_t ttl, const ip_addr_t *ipaddr, void *callback_arg)
{
    LWIP_UNUSED_ARG(name);
    struct t2n_context *cntx_p = (struct t2n_context *)callback_arg;
    //   struct tun_connection_args * cntx_p = (struct tun_connection_args *)arg;

    if (ipaddr != NULL)
    {
        /* Address resolved, send request */
        // LWIP_DEBUGF(SNTP_DEBUG_STATE, ("sntp_dns_found: Server address resolved, sending request\n"));
        T2N_LOGI("hostname_notify_cb host=%s,ip=%s\n", name, inet_ntoa(ipaddr->u_addr.ip4));
        cntx_p->server_ip = *ipaddr;
        cntx_p->dns_state = 1;
    }
    else
    {
        /* DNS resolving failed -> try another server */
        cntx_p->dns_state = 2;
        T2N_LOGW("hostname_notify_cb: [%s] Failed to resolve server address resolved, trying next server\n", name);
        // LWIP_DEBUGF(SNTP_DEBUG_WARN_STATE, ("sntp_dns_found: Failed to resolve server address resolved, trying next server\n"));
        // sntp_try_next_server(NULL);
    }
}
void ql_nw_ind_callback(uint8_t sim_id, unsigned int ind_type, void *ind_msg_buf)
{
    // esp32_ota_log(LINFO, "ql_nw_ind_callback sim_id = %d, ind_type=%d", sim_id, ind_type);
    T2N_LOGI("ql_nw_ind_callback sim_id = %d, ind_type=%d", sim_id, ind_type);
 
    if (QUEC_NW_DATA_REG_STATUS_IND == ind_type)
    {
        ql_nw_common_reg_status_info_s *data_reg_status = (ql_nw_common_reg_status_info_s *)ind_msg_buf;
        T2N_LOGI("Sim%d data reg status changed, current status is %d", sim_id, data_reg_status->state);
        if ((QL_NW_REG_STATE_HOME_NETWORK == data_reg_status->state) || (QL_NW_REG_STATE_ROAMING == data_reg_status->state))
        {
            // ql_rtos_semaphore_release(ql_data_reg_sem[sim_id]);
            ql_datacall_errcode_e err;
            err = ql_start_data_call(sim_id, 1, QL_PDP_TYPE_IPV4V6, "cmnet", NULL, NULL, 0);
            cntx_p->sim_id = sim_id;
            cntx_p->profile_idx = 1;
            if (err == QL_DATACALL_SUCCESS)
            {
                if (cntx_p->status == 0)
                {
                    cntx_p->status = 1;
                }
            }
        }
    }
    else if (QUEC_NW_CELL_INFO_IND == ind_type)
    {
        ql_nw_cell_info_s *cell_info = (ql_nw_cell_info_s *)ind_msg_buf;
        for (int i = 0; i < cell_info->lte_info_num; i++)
        {
            T2N_LOGI("Sim%d nw_cell_info ind %d rssi=%d,RX_dBm=%d,cid=%d", sim_id, cell_info->lte_info[i].rssi, cell_info->lte_info[i].RX_dBm, cell_info->lte_info[i].cid);
        }
    }
    else if (QUEC_NW_SIGNAL_QUALITY_IND == ind_type)
    {
        ql_nw_signal_strength_info_s *signal_info = (ql_nw_signal_strength_info_s *)ind_msg_buf;
        T2N_LOGI("Sim%d signal: rssi:%d, bitErrorRate:%d, rsrp:%d, rsrq:%d",
                 sim_id,
                 signal_info->rssi, signal_info->bitErrorRate,
                 signal_info->rsrp, signal_info->rsrq);
        gWordVar[REG_CSQ] = signal_info->rssi;
    }
}

void ql_datacall_ind_callback(uint8_t sim_id, unsigned int ind_type, int profile_idx, bool result, void *ctx)
{
    T2N_LOGI("ql_datacall_ind_callback nSim = %d, profile_idx=%d, ind_type=0x%x, result=%d", sim_id, profile_idx, ind_type, result);
    if ((profile_idx < PROFILE_IDX_MIN) || (profile_idx > PROFILE_IDX_MAX))
    {
        return;
    }

    switch (ind_type)
    {
    case QUEC_DATACALL_ACT_RSP_IND: // only received in asyn mode
    {
        if (result)
        {
            ql_data_call_info_s info;
            char ip4_addr_str[16] = {0};
            int ret = ql_get_data_call_info(sim_id, profile_idx, &info);
            if (ret != 0)
            {
                T2N_LOGI("ql_get_data_call_info ret: %d", ret);
                ql_stop_data_call(sim_id, profile_idx);
            }
            T2N_LOGI("info->QL_SMARTCARD_PDP_CID: %d", info.profile_idx);
            T2N_LOGI("info->ip_version: %d", info.ip_version);

            T2N_LOGI("info->v4.state: %d", info.v4.state);
            inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
            T2N_LOGI("info.v4.addr.ip: %s\r\n", ip4_addr_str);

            inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
            T2N_LOGI("info.v4.addr.ip: %s\r\n", ip4_addr_str);

            inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
            T2N_LOGI("info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);

            inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
            T2N_LOGI("info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);
            struct netif *gp_if = netif_get_by_cid((cntx_p->sim_id << 8) | (profile_idx & 0xff));
            // struct netif *gp_if = netif_get_by_cid_type((g_sim_id << 8) | (profile_idx & 0xff), NETIF_LINK_MODE_NAT_WAN);
            if (gp_if != NULL)
            {
                T2N_LOGI("netif_set_default %c%c%d addr=%s\r\n", gp_if->name[0], gp_if->name[1], gp_if->num, ip4addr_ntoa(&gp_if->ip_addr.u_addr.ip4));
                netif_set_default(gp_if);
            }
            // ql_usbnet_state_e status;
            // ql_usbnet_errcode_e error = ql_usbnet_get_status(&status);
            // if(error == QL_USBNET_SUCCESS){
            //     T2N_LOGI("usbnet status=%d",status);
            //     if(status == QL_USBNET_STATE_CONNECT){
            //         ql_usbnet_start(g_sim_id, 1, NULL);
            //     }
            // }
            ql_usbnet_start(cntx_p->sim_id, 1, NULL);
            if (usb_acm_slip_task != NULL)
            {
                ql_event_t event;
                event.id = QUEC_DATACALL_ACT_RSP_IND;
                event.param1 = cntx_p->sim_id;
                event.param2 = profile_idx;
                ql_rtos_event_send(usb_acm_slip_task, &event);
            }
            if (cntx_p->status == 1)
            {
                cntx_p->status = 2;
            }
            else if (cntx_p->status == 5)
            {
                t2n_send_heartbeat(0);
            }
        }
        // datacall_ctx[sim_id][profile_idx - 1].state = (result == true) ? QL_PDP_ACTIVED : QL_PDP_DEACTIVED;
        // datacall_ctx[1][profile_idx - 1].state = (result == true) ? QL_PDP_ACTIVED : QL_PDP_DEACTIVED;
        ql_usbnet_start(sim_id, profile_idx, NULL);
        break;
    }
    case QUEC_DATACALL_DEACT_RSP_IND: // only received in asyn mode
    {
        if (result)
        {
        }
        // datacall_ctx[sim_id][profile_idx - 1].state = (result == true) ? QL_PDP_DEACTIVED : QL_PDP_ACTIVED;
        // datacall_ctx[1][profile_idx - 1].state = (result == true) ? QL_PDP_DEACTIVED : QL_PDP_ACTIVED;
        // ql_usbnet_start(sim_id, profile_idx, NULL);
        break;
    }
    case QUEC_DATACALL_PDP_DEACTIVE_IND: // received in both asyn mode and sync mode
    {
        // datacall_ctx[sim_id][profile_idx - 1].state = QL_PDP_DEACTIVED;
        // datacall_ctx[1][profile_idx - 1].state = QL_PDP_DEACTIVED;
        // ql_event_t event = {0};
        // event.id = QUEC_DATACALL_PDP_DEACTIVE_IND;
        // event.param1 = profile_idx;
        // ql_usbnet_stop();
        break;
    }
    }
}
int profile_idx = 1;
static void ql_usbnet_event_callback(unsigned int event_type, ql_usbnet_errcode_e errcode, void *ctx)
{
    T2N_LOGI("event_type: 0x%x, errcode: 0x%x", event_type, errcode);

    switch (event_type)
    {
    case QUEC_USBNET_START_RSP_IND:
    {
        if (QL_USBNET_SUCCESS == errcode)
        {
            T2N_LOGI("usbnet connect success");
        }
        else
        {
            T2N_LOGI("usbnet connect fail, err: 0x%x", errcode);
        }
        break;
    }
    case QUEC_USBNET_DEACTIVE_IND:
    {
        T2N_LOGI("PDP deactived");
        break;
    }
    case QUEC_USBNET_PORT_CONNECT_IND:
    {
        T2N_LOGI("usb port plug in");
        ql_usbnet_state_e status;
        ql_usbnet_errcode_e error = ql_usbnet_get_status(&status);
        if (error == QL_USBNET_SUCCESS)
        {
            T2N_LOGI("usbnet status=%d", status);
            if (status != QL_USBNET_STATE_CONNECT)
            {
                ql_usbnet_start(cntx_p->sim_id, 1, NULL);
            }
        }
        break;
    }
    case QUEC_USBNET_PORT_DISCONNECT_IND:
    {
        T2N_LOGI("usb port pull out");
        // ql_usbnet_state_e status;
        // ql_usbnet_errcode_e error = ql_usbnet_get_status(&status);
        // if (error == QL_USBNET_SUCCESS)
        // {
        //     T2N_LOGI("usbnet status=%d", status);
        //     if (status == QL_USBNET_STATE_CONNECT)
        //     {
        ql_usbnet_stop();
        // }
        // }
        break;
    }
    }
}

err_t t2n_send_to(struct pbuf *pkt_buf, const struct ip4_addr *dest)
{
    cntx_p->last_tx_time = osiUpTime();
    err_t ret = ERR_CONN;
    if (cntx_p->udp_pcb)
    {
        ret = udp_sendto(cntx_p->udp_pcb, pkt_buf, &cntx_p->server_ip, cntx_p->server_port);
    }
    T2N_LOGI("t2n_send_to ip:%s port:%d len:%d ret:%d", inet_ntoa(cntx_p->server_ip.u_addr.ip4), cntx_p->server_port, pkt_buf->tot_len, ret);
    return ret;
}

static void recv_callback_udp(void *arg, struct udp_pcb *upcb, struct pbuf *pkt_buf,
                              const ip_addr_t *addr, u16_t port)
{
    t2n_input(pkt_buf);
}

void set_status(uint8_t status)
{
    cntx_p->status = status;
}

void t2n_poll(void *parm)
{
    // static int csq_time_count = 0;
    // static int cnt = 0;
    //  static int count2 = 0;
    static int nSim = 0;
    static int profile_idx = 1;
    struct t2n_context *contx_p = (struct t2n_context *)parm;
    int64_t now = osiUpTime();
    // if (now > 1000 * 60 * 60 * 24 * 7) // 连续运行7天强制重启
    // {
    //     reboot_req = 0x55aa00ff;
    // }
    if (cntx_p->status != cntx_p->last_status)
    {
        T2N_LOGI("[%s] status=%d,last_status=%d\n", __FUNCTION__, cntx_p->status,
                 cntx_p->last_status);
        // if (cntx_p->status == 5)
        // {
        //     gWordVar[REG_NET_STATUS] = 2;
        // }
        // else if (cntx_p->status >= 2 && cntx_p->status <= 4)
        // {
        //     gWordVar[REG_NET_STATUS] = 1;
        // }
        // else
        // {
        gWordVar[REG_NET_STATUS] = cntx_p->status;
        // }
    }
    // if(++count2 > 10){
    //     count2 = 0;
    //     LSAPI_MENU_Printf("[%s] now=%d",__FUNCTION__,now);
    // }
    switch (cntx_p->status)
    {
    case 0: // 等待模块注册网络
        if (cntx_p->last_status != 0)
        {
            cntx_p->gprs_err_cnt = 0;
            cntx_p->gsm_state = 0;
            cntx_p->time_out_cnt = 0;
            cntx_p->last_status = cntx_p->status;
            int ret = ql_dev_get_imei(imei, sizeof(imei), 0);
            if (ret == 0)
            {
                T2N_LOGI("IMEI=%s\n", imei);
                strncpy(cntx_p->imei, imei, sizeof(cntx_p->imei));
                int len = (strlen(imei) + 1) / 2;
                for (int i = 0; i < len; i++)
                {
                    gWordVar[REG_IMEI + i] = (imei[i * 2] << 8) | imei[i * 2 + 1];
                }
            }
        }
        else
        {
            ql_nw_reg_status_info_s nw_info;
            int ret = ql_nw_get_reg_status(1, &nw_info);
            if (ret == 0)
            {
                T2N_LOGI("ql_nw_get_reg_status ret: 0x%x, current data reg status=%d", ret, nw_info.data_reg.state);
                if ((QL_NW_REG_STATE_HOME_NETWORK == nw_info.data_reg.state) || (QL_NW_REG_STATE_ROAMING == nw_info.data_reg.state))
                {
                    cntx_p->status = 1;
                }
                if (++cntx_p->time_out_cnt > 300)
                {
                    cntx_p->status = 1;
                }
            }
            // else
            // {
            //     T2N_LOGI("ql_nw_get_reg_status err ret: 0x%x", ret);
            // }
            // int timeout_cnt = 0;
            //  int ret = ql_network_register_wait(nSim, 1);
            //  if (ret == 0)
            //  {

            // }
            // else
            // {
            //     if (++cntx_p->time_out_cnt >= 10)
            //     {
            //         log_printf(LINFO, LOG_TAG "network register timeout\n");
            //         cntx_p->time_out_cnt = 0;
            //     }
            // }
        }
        break;
    case 1: // 数据连接
        if (cntx_p->last_status != 1)
        {
            ql_datacall_errcode_e ret;
            cntx_p->last_status = cntx_p->status;
            cntx_p->gprs_err_cnt = 0;
            cntx_p->gsm_state = 0;
            cntx_p->time_out_cnt = 0;
            ret = ql_dev_get_imei(imei, sizeof(imei), 0);
            if (ret == 0)
            {
                T2N_LOGI("IMEI=%s\n", imei);
                strncpy(cntx_p->imei, imei, sizeof(cntx_p->imei));
                int len = (strlen(imei) + 1) / 2;
                for (int i = 0; i < len; i++)
                {
                    gWordVar[REG_IMEI + i] = (imei[i * 2] << 8) | imei[i * 2 + 1];
                }
            }
            ret = ql_start_data_call(cntx_p->sim_id, cntx_p->profile_idx, QL_PDP_TYPE_IPV4V6, "cmnet", NULL, NULL, 0);
            if (ret != 0)
            {
                T2N_LOGW("ql_start_data_call fail,ret=%d\n", ret);
            }
            // ret = ql_sim_get_iccid(1, ccid, sizeof(ccid));
            // if (ret == 0)
            // {
            //     T2N_LOGI("ccid%d=%s\n", 0, ccid);
            // }
            // else
            // {
            //     T2N_LOGI("ql_sim_get_iccid%d ret=%d\n", 0, ret);
            // }
            // ret = ql_sim_get_iccid(1, ccid, sizeof(ccid));
            //  if (ret == 0)
            //  {
            //      T2N_LOGI("ccid%d=%s\n", 1, ccid);
            //  }
            //  else
            //  {
            //      T2N_LOGI("ql_sim_get_iccid%d ret=%d\n", 0, ret);
            //  }

            // ret = ql_start_data_call(1, profile_idx, QL_PDP_TYPE_IPV4V6, "cmnet", NULL, NULL, 0);

            // if (ret != 0)
            // {
            //     T2N_LOGW("ql_start_data_call fail,ret=%d\n", ret);
            // }
        }
        else
        {
            // if (datacall_ctx[nSim][profile_idx - 1].state == QL_PDP_ACTIVED)
            // {
            //     ql_data_call_info_s info;
            //     char ip4_addr_str[16] = {0};
            //     int ret = ql_get_data_call_info(nSim, profile_idx, &info);
            //     if (ret != 0)
            //     {
            //         T2N_LOGI("ql_get_data_call_info ret: %d", ret);
            //         ql_stop_data_call(nSim, profile_idx);
            //     }
            //     T2N_LOGI("info->QL_SMARTCARD_PDP_CID: %d", info.profile_idx);
            //     T2N_LOGI("info->ip_version: %d", info.ip_version);

            //     T2N_LOGI("info->v4.state: %d", info.v4.state);
            //     inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
            //     T2N_LOGI("info.v4.addr.ip: %s\r\n", ip4_addr_str);

            //     inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
            //     T2N_LOGI("info.v4.addr.ip: %s\r\n", ip4_addr_str);

            //     inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
            //     T2N_LOGI("info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);

            //     inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
            //     T2N_LOGI("info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);

            //     struct netif *gp_if = netif_get_by_cid((nSim << 8) | (profile_idx & 0xff));
            //     if (gp_if != NULL)
            //     {
            //         T2N_LOGI("netif_set_default %c%c\r\n", gp_if->name[0], gp_if->name[1]);
            //         netif_set_default(gp_if);
            //     }

            //     // ql_nat_assign_cfg_s  nat_assign_cfg = {
            //     //     .mode = QL_NAT_ASSIGN_TYPE_USBNET,
            //     //     .ip_addr = info.v4.addr.ip,
            //     //     .gw_addr = info.v4.addr.ip,
            //     //     .dns_addr = info.v4.addr.pri_dns,

            //     // };
            //     // ql_datacall_set_nat_assign(nSim,QL_NAT_ASSIGN_TYPE_USBNET,&nat_assign_cfg);
            //     cntx_p->status = 2;
            // }
            // log_printf(LINFO, LOG_TAG "Start NetIf_Create\n");
            // if (LSAPI_NET_GET_GprsNetIf())
            // {
            //     LSAPI_NET_NetIf_Destory();
            // }
            // LSAPI_NET_NetIf_Create();
            // if (LSAPI_NET_GET_GprsNetIf() == FALSE)
            // {
            //     log_printf(LINFO, LOG_TAG " NetIf_Create failed\n");
            //     // cntx_p->status = 1;
            // }
            // else
            // {
            //     cntx_p->status = 2;
            // }
        }
        break;
    case 2: // 服务器DNS
        if (cntx_p->last_status != 2)
        {

            cntx_p->last_status = cntx_p->status;
            cntx_p->is_login = 0;
            char *host;
            if ((cntx_p->ser_sel < 2) && (strlen(dsc_cfg.host[cntx_p->ser_sel]) > 0) &&
                (dsc_cfg.port[cntx_p->ser_sel] > 0))
            {
                host = dsc_cfg.host[cntx_p->ser_sel];
                cntx_p->server_port = dsc_cfg.port[cntx_p->ser_sel];
            }
            else
            {
                host = defalut_server;
                cntx_p->server_port = defalut_port;
            }
            cntx_p->dns_state = 0;
            int err = dns_gethostbyname_addrtype(((cntx_p->sim_id << 8) | (profile_idx & 0xff)), host, &cntx_p->server_ip, hostname_notify_cb, cntx_p, LWIP_DNS_ADDRTYPE_IPV4);
            if (err >= ERR_OK)
            {
                T2N_LOGI("gethostbyname success host=%s,ip=%s", host, inet_ntoa(cntx_p->server_ip.u_addr.ip4));
                cntx_p->dns_err_cnt1 = 0;
                cntx_p->dns_err_cnt2 = 0;
                cntx_p->dns_state = 1;
            }
            else if (err == ERR_INPROGRESS)
            {
                cntx_p->dns_state = 1;
            }
            else
            {
                // int err = dns_gethostbyname(dsc_cfg.host[cntx_p->ser_sel], &cntx_p->server_ip, hostname_notify_cb, cntx_p);
                // cntx_p->server_port = defalut_port;
                // // cntx_p->server_ip.sin_family = LSAPI_SOCK_TCPIP_AF_INET;
                T2N_LOGI("get host [%s] err:%d\n", host, err);
            }
        }
        else
        {
            if (cntx_p->dns_state == 1)
            {
                cntx_p->dns_err_cnt1 = 0;
                cntx_p->dns_err_cnt2 = 0;
                cntx_p->status = 3;
            }
            else if ((cntx_p->dns_state == 2) || (cntx_p->time_out_cnt >= 50 * 10))
            {
                if (++cntx_p->dns_err_cnt1 < 2)
                {
                    cntx_p->time_delay_cnt = 10 * 10; // 10s
                    cntx_p->next_status = 2;
                    cntx_p->status = 6;
                    cntx_p->last_status = -2;
                }
                else if (cntx_p->dns_err_cnt1 == 2)
                {
                    cntx_p->ser_sel++;
                    if (cntx_p->ser_sel > 2)
                    {
                        cntx_p->ser_sel = 0;
                    }
                    else
                    {
                        cntx_p->dns_err_cnt1 = 0;
                    }
                }
                else
                {
                    cntx_p->dns_err_cnt1 = 0;
                    cntx_p->dns_err_cnt2++;
                    // LSAPI_SOCK_Close(cntx_p->udp_sock_fd);
                    cntx_p->udp_sock_fd = 0;
                    // ls_PdpActive(0);
                    if (cntx_p->dns_err_cnt2 < 3)
                    {
                        cntx_p->time_delay_cnt = 10 * 60; // 60s
                    }
                    else // if(cntx_p->dns_err_cnt2 < 10)
                    {
                        cntx_p->time_delay_cnt = 5 * 60 * 10; // 5 min
                    }
                    // else
                    //{
                    // cntx_p->time_delay_cnt = 10*60 * 30;   //30 min
                    //}
                    cntx_p->next_status = 1;
                    cntx_p->status = 6;
                }
            }
            else
            {
                cntx_p->time_out_cnt++;
            }
        }
        break;
    case 3: // 创建SOCKET
        if (cntx_p->last_status != 3)
        {
            int status = -1;
            cntx_p->last_status = cntx_p->status;
            if (cntx_p->udp_pcb != NULL)
            {
                udp_remove(cntx_p->udp_pcb);
                cntx_p->udp_pcb = NULL;
            }
            struct udp_pcb *udp_pcb = udp_new();
            if (udp_pcb != NULL)
            {
                int err = udp_bind(udp_pcb, NULL, 0);
                if (err != ERR_OK)
                {
                    /* Unable to bind to port  */
                    T2N_LOGE("udp_bind %d\n", err);
                    udp_remove(udp_pcb);
                    cntx_p->last_status = -1;
                }
                udp_recv(udp_pcb, recv_callback_udp, cntx_p);
                cntx_p->udp_pcb = udp_pcb;
                cntx_p->status = 4;
            }
            // done:
            //     if (status == 0) // socket ok
            //     {
            //         cntx_p->socket_err_cnt = 0;
            //         cntx_p->login_err_cnt1 = 0;
            //         cntx_p->status = 4;
            //         T2N_LOGI("udp socket create success\n");
            //     }
            //     else // socket err wait 10s after retry
            //     {
            //         cntx_p->socket_err_cnt++;
            //         T2N_LOGE("udp socket create failed\n");
            //     }
            cntx_p->time_out_cnt = 0;
        }
        else
        {
            if (++cntx_p->time_out_cnt >= 10 * 10)
            {
                if (cntx_p->socket_err_cnt <= 3)
                {
                    cntx_p->last_status = -3;
                }
                else
                {
                    // sys_reboot_req();
                    // eat_soc_close(udp_sock_fd);
                    // LSAPI_MENU_Printf("status error
                    // status=%d,last_status=%d",cntx_p->status,cntx_p->last_status);
                    // cntx_p->last_status = -1;
                }
            }
        }
        break;
    case 4: // 登陆服务器
        if (cntx_p->last_status != 4)
        {
            int ret;
            cntx_p->time_out_cnt = 0;
            cntx_p->last_status = cntx_p->status;
            // cntx_p->my_addr = 0;
            cntx_p->login_status = 0;
            // IP4_ADDR(&cntx_p->t2n_if.ip_addr, 0, 0, 0, 0);
            T2N_LOGD("t2n_send_heartbeat start...\n");
            ret = t2n_send_heartbeat(0);
            if (ret < 0)
            {
                // log_printf(LERROR, LOG_TAG "t2n_send_heartbeat error = %d\n", LSAPI_SOCK_Error());
                cntx_p->status = 6;
                cntx_p->time_delay_cnt = 10 * 10; // 60s
                cntx_p->next_status = 3;
            }
        }
        else
        {
            if (cntx_p->login_status == 100) // 登陆成功
            {
                T2N_LOGI("login success\n");
                cntx_p->status = 5;
                cntx_p->login_err_cnt1 = 0;
                cntx_p->login_err_cnt2 = 0;
                // sys_data.snts.server_state = SYS_STA_SER_LOGIN_SUCCESS; // 修改通知当前的服务器状态
                // set_net_active(EAT_TRUE);
            }
            else if (cntx_p->login_status == 0)
            {
                // sys_data.snts.server_state = SYS_STA_SER_LOGIN_FAILED;          // 修改通知当前的服务器状态
                if (++cntx_p->time_out_cnt > (cntx_p->login_err_cnt1 * 50 + 100)) // 10s 服务器不响应
                {
                    cntx_p->login_err_cnt1++;
                    if (cntx_p->login_err_cnt1 < 5)
                    {
                        cntx_p->last_status = -4;
                    }
                    else
                    {
                        if (++cntx_p->ser_sel > 2)
                        {
                            cntx_p->ser_sel = 0;
                        }
                        cntx_p->login_err_cnt2++;
                        if (cntx_p->login_err_cnt2 < 4)
                        {
                            cntx_p->rx_time_cnt = 0;
                            cntx_p->status = 2;
                        }
                        else
                        {
                            cntx_p->next_status = 2;
                            cntx_p->time_delay_cnt = 60 * 10; // 等待1min 重试
                            T2N_LOGI("wait for 60s try again\n");
                            cntx_p->status = 6;
                            // cntx_p->login_err_cnt2 = 0;
                            cntx_p->login_err_cnt1 = 0;
                        }
                    }
                }
            }
            else // 服务器返回登陆不成功
            {
                T2N_LOGE("login failed code=%d\n", cntx_p->login_status);
                cntx_p->login_err_cnt1++;
                if (cntx_p->login_err_cnt1 < 3)
                {
                    cntx_p->time_delay_cnt = 5 * 10; // 5s
                }
                else if (cntx_p->login_err_cnt1 < 10)
                {
                    cntx_p->time_delay_cnt = 15 * 10; // 15s
                }
                else
                {
                    cntx_p->time_delay_cnt = 60 * 10; // 60s
                }
                cntx_p->next_status = 2;
                cntx_p->status = 6;
            }
        }
        break;
    case 5: // 网络正常工作中
        if (cntx_p->last_status != 5)
        {
            cntx_p->tx_time_cnt = 0;
            cntx_p->rx_time_cnt = 0;
            cntx_p->last_report_tick = 0;
            cntx_p->last_status = cntx_p->status;
        }
        else
        {
            int ret = 0;
            t2n_poll_queue_process(now);
            if ((now - cntx_p->last_tx_time) > 30000)
            {
                if (now - cntx_p->last_rx_time > 25000)
                {
                    ret = t2n_send_heartbeat(1);
                }
                else
                {
                    ret = t2n_send_heartbeat(0);
                }
            }
            if ((int)(now - cntx_p->last_rx_time) > 65000) // 服务器不响应
            {
                cntx_p->rx_time_cnt = 0;
                cntx_p->status = 2;
                cntx_p->login_status = 0;
            }
            else if ((now - cntx_p->last_rx_time) > 45000)
            {
                if ((now - cntx_p->last_hb_time) > 5000)
                {
                    t2n_send_heartbeat(1);
                }
            }
        }
        break;
    case 6:
        if (cntx_p->last_status != 6)
        {
            cntx_p->last_status = cntx_p->status;
            cntx_p->time_out_cnt = 0;
        }
        else
        {
            if (++cntx_p->time_out_cnt > cntx_p->time_delay_cnt)
            {
                cntx_p->status = cntx_p->next_status;
            }
        }
        break;
    default:
        cntx_p->status = 0;
        break;
    }
    // sys_check_timeouts();
    if (cntx_p->TxIcoCnt > 0)
    {
        if (--cntx_p->TxIcoCnt == 0)
        {
            // SetTxIco(0);
        }
    }
    if (cntx_p->RxIcoCnt > 0)
    {
        if (--cntx_p->RxIcoCnt == 0)
        {
            // SetRxIco(0);
        }
    }
    // if (++csq_time_count >= 20)
    // {
    //     int csq;
    //     csq_time_count = 0;
    //     if (LSAPI_NET_GetCsq(&csq) == 0)
    //     {
    //         gWordVar[REG_CSQ] = csq;
    //     }
    // }
    osiTimerStart(t2n_poll_timer, 100);
}

extern void t2n_init(void);
int sim_sel = 3;

static void t2n_task(void *param)
{
    t2n_thread_handle = osiThreadCurrent();
    int ret = 0;
    int i;
    int new_sim_id;
    uint8_t sim_id = ql_sim_get_operate_id();
    int is_change_sim = 0;
    if (sim_sel == 3) // 自动切换
    {
        for (i = 0; i < 10; i++)
        {
            ql_sim_status_e sim_status;
            ret = ql_sim_get_card_status(0, &sim_status);
            if (ret == 0)
            {
                if (sim_status == QL_SIM_STATUS_READY)
                {
                    break;
                }
                else
                {
                    T2N_LOGE("ql_sim_get_card_status ret=%d,status=%d\n", ret, sim_status);
                }
            }
            ql_rtos_task_sleep_s(1);
        }
        if (i == 10)
        {
            is_change_sim = 1;
        }
    }
    else
    {
        if (sim_sel & (1 << sim_id) == 0)
        {
            is_change_sim = 1;
        }
    }

    T2N_LOGI("get sim_id is %d\n", sim_id);

    if (is_change_sim)
    {
        new_sim_id = sim_id ? 0 : 1;
exit:
        T2N_LOGE("sim_set_operate_id=%d\n", new_sim_id);
        ret = ql_sim_set_operate_id(new_sim_id);
        if (ret != 0)
        {
            T2N_LOGE("ql_sim_set_operate_id fall ret=%d\n", ret);
        }
        for (int i = 0; i < 10; i++)
        {
            ql_sim_status_e sim_status;
            ql_sim_get_card_status(0, &sim_status);
            if (sim_status == QL_SIM_STATUS_NOSIM)
            {
                T2N_LOGI("card%d status nosim\n", 0);
            }
            else if (sim_status == QL_SIM_STATUS_READY)
            {
                T2N_LOGI("sim card ready\n");
                ret = ql_sim_get_iccid(0, ccid, sizeof(ccid));
                if (ret == 0)
                {
                    T2N_LOGI("ccid=%s\n", ccid);
                    break;
                }
                else
                {
                    T2N_LOGE("ql_sim_get_iccid fall ret=%d\n", 0, ret);
                }
            }
            else
            {
                T2N_LOGI("card%d status=%d\n", 0, sim_status);
            }

            ql_rtos_task_sleep_s(1);
        }
    }

    ql_nat_subnet_config_s config;
    ql_datacall_get_subnet(0, 1, &config);
    if (strcmp(config.subnet_addr, "192.168.5.0") != 0)
    {
        strcpy(config.subnet_addr, "192.168.5.0");
        strcpy(config.subnet_mask, "255.255.255.0");
        ret = ql_datacall_set_subnet(0, 1, &config);
        if (ret != 0)
        {
            T2N_LOGI("ql_datacall_set_subnet err, ret=0x%x", ret);
        }
        ret = ql_datacall_set_subnet(1, 1, &config);
        if (ret != 0)
        {
            T2N_LOGI("ql_datacall_set_subnet err, ret=0x%x", ret);
        }
    }

    uint32_t saved_list = 0;
    uint32_t target_list = 0x10001; // enable sim0 profile 1 and sim1 profile 1 nat function
    ret = ql_datacall_get_nat(&saved_list);
    T2N_LOGI("get datacall saved nat ret: 0x%x, %d", ret, saved_list);
    if (0 != ret)
    {
    }
    if (saved_list != target_list)
    {
        ret = ql_datacall_set_nat(target_list);

        if (0 != ret)
        {
            T2N_LOGE("ql_datacall_set_nat err, ret=0x%x", ret);
        }
    }

    ret = ql_nw_register_cb(ql_nw_ind_callback);
    ql_datacall_register_cb(0, 1, ql_datacall_ind_callback, cntx_p);
    ql_set_data_call_asyn_mode(0, 1, 1);
    // ql_datacall_register_cb(1, 1, ql_datacall_ind_callback, cntx_p);
    // ql_set_data_call_asyn_mode(1, 1, 1);

    ql_usbnet_register_cb(ql_usbnet_event_callback, cntx_p);

    // ql_usbnet_type_e saved_type = QL_USBNET_NONE;
    // ret = ql_usbnet_get_type(&saved_type);
    // if (saved_type != QL_USBNET_ECM)
    // {
    //     ret = ql_usbnet_set_type(QL_USBNET_ECM);
    //     T2N_LOGI("ql_usbnet_set_type err, ret=0x%x", ret);
    // }

    T2N_LOGI("t2n_thread start...\n");
    t2n_init();

    // int timeout_cnt = 0;

    t2n_poll_timer = osiTimerCreate(t2n_thread_handle, t2n_poll, cntx_p); // 创建定时器
    if (NULL == t2n_poll_timer)
    {
        T2N_LOGE("t2n_poll_timer create failed\n");
        osiThreadExit();
    }
    ret = osiTimerStart(t2n_poll_timer, 100);
    if (ret != TRUE)
    {
        T2N_LOGE("t2n_poll_timer start failed\n");
    }
    ql_LvlMode hi = 0;
    for (;;)
    {
        osiEvent_t event;
        osiEventWait(t2n_thread_handle, &event); // 等待事件发生

        T2N_LOGE("hi = %d\n", hi);
        if(ql_gpio_get_level(GPIO_28,&hi) == QL_GPIO_SUCCESS)
        {
            new_sim_id = 1;
            T2N_LOGE("key button change sim_id st\n");
            // goto exit;
        }
        // if (event.id != 0)
        // {
        //     log_printf(LDEBUG, LOG_TAG "waitevenid = 0x%x\n", event.id);
        // }
        // switch (event.id)
        // {
        // case LSAPI_SOCK_TCPIP_REV_DATA_IND:
        //     if (cntx_p->udp_sock_fd > 0)
        //     {
        //         LSAPI_SOCK_TcpipSocketAddr_t udp_fromaddr;
        //         for (;;)
        //         {
        //             // uint8_t *udp_recvbuf = t2n_buf_alloc(1500);
        //             struct pbuf *pkt_buf = pbuf_alloc(PBUF_TRANSPORT, 1500, PBUF_RAM);
        //             if (pkt_buf != NULL)
        //             {
        //                 // log_printf(LDEBUG, LOG_TAG "recvfrom enter\n");
        //                 int len =
        //                     LSAPI_SOCK_Recvfrom(cntx_p->udp_sock_fd, pkt_buf->payload, 1500, 0, &udp_fromaddr);
        //                 if (len > 0)
        //                 {
        //                     char *IpAddr = LSAPI_SOCK_inet_ntoa(udp_fromaddr.sin_addr);
        //                     log_printf(LDEBUG, LOG_TAG "udp recv %d bytes from %s:%d\n", len, IpAddr,
        //                                LSAPI_SOCK_ntohs(udp_fromaddr.sin_port));
        //                     pkt_buf->tot_len = pkt_buf->len = len;
        //                     t2n_input(pkt_buf);
        //                 }
        //                 else
        //                 {
        //                     pbuf_free(pkt_buf);
        //                     if (len < 0)
        //                     {
        //                         log_printf(LERROR, LOG_TAG "recv failed\n");
        //                     }
        //                     break;
        //                 }
        //             }
        //             else
        //             {
        //                 log_printf(LERROR, LOG_TAG "udp recv error no Memory\n");
        //                 break;
        //             }
        //         }
        //     }
        //     else
        //     {
        //         log_printf(LERROR, LOG_TAG "invalid sockfd\n");
        //     }
        //     break;
        // case RAW_ICMP_REV_DATA_IND:
        //     if (raw_icmp_sock_fd >= 0)
        //     {
        //         raw_sock_recv(raw_icmp_sock_fd, LSAPI_SOCK_TCPIP_IPPROTO_ICMP);
        //     }
        //     break;
        // case RAW_TCP_REV_DATA_IND:
        //     if (raw_tcp_sock_fd >= 0)
        //     {
        //         raw_sock_recv(raw_tcp_sock_fd, LSAPI_SOCK_TCPIP_IPPROTO_TCP);
        //     }
        //     break;
        // case RAW_UDP_REV_DATA_IND:
        //     if (raw_udp_sock_fd >= 0)
        //     {
        //         raw_sock_recv(raw_tcp_sock_fd, LSAPI_SOCK_TCPIP_IPPROTO_UDP);
        //     }
        //     break;
        // case USB_NET_REV_DATA_IND: {
        //     struct pbuf *pkt_buf = (struct pbuf *)event.param1;
        //     if (pkt_buf)
        //     {
        //         ethernet_input(pkt_buf, &cntx_p->tun_if);
        //     }
        //     break;
        // }
        // }
    } // for (;;)
    T2N_LOGI("t2n_thread end...\n");
    // LSAPI_OSI_ThreadExit();
}

uint32_t t2n_clock(void)
{
    return osiUpTime();
}

void SetTxIco(int v)
{
    // v=0 不显示 =1 显示
    if (v == 1)
    {
        // sys_data.snts.server_state = SYS_STA_SER_UPLOAD_DATA; // 修改通知当前的服务器状态
    }
    else
    {
        // sys_data.snts.server_state = SYS_STA_SER_UPLOAD_COMPLETE_DATA; // 修改通知当前的服务器状态
    }
}
void SetRxIco(int v)
{
    if (v == 1)
    {
        // sys_data.snts.server_state = SYS_STA_SER_DOWNLOAD_DATA; // 修改通知当前的服务器状态
    }
    else
    {
        // sys_data.snts.server_state = SYS_STA_SER_DOWNLOAD_COMPLETE_DATA; // 修改通知当前的服务器状态
    }
}

// void ql_t2n_app_init(void)
// {
//     QlOSStatus err = QL_SUCCESS;
//     ql_task_t charge_task = NULL;
//     err = ql_rtos_task_create(&charge_task, 8192, APP_PRIORITY_NORMAL, "t2n", t2n_thread, NULL, 1);
// 	if( err != QL_SUCCESS )
//     {
//         T2N_NW_LOGE("t2n task created failed");
//     }
// }

void t2n_task_init(void)
{
    QlOSStatus err = 0;
    ql_task_t task_ref;
    err = ql_rtos_task_create(&task_ref, 4096, APP_PRIORITY_NORMAL, "t2n", t2n_task, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        T2N_LOGE("creat t2n_task fail err = %d", err);
    }
    T2N_LOGI("creat t2n_task sucess!");
}
