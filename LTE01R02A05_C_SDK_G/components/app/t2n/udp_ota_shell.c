#include "ModbusS.h"
#include "osi_api.h"
#include "ql_api_osi.h"
#include "lwip/ip_addr.h"
#include "lwip/udp.h"
#include "fupdate_config.h"
#include "ql_fs.h"
#include "../esp32_flash/inc/esp32_flash.h"
#include "../http_fota/inc/fota_http_demo.h"
#include "udp_ota_shell.h"
#include "ql_log.h"
#include "ql_power.h"

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "udp_ota_shell", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "udp_ota_shell", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "udp_ota_shell", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "udp_ota_shell", msg, ##__VA_ARGS__)

extern *modbus_slave_thread;
static struct udp_pcb *udp_ota_shell_pcb = NULL;
static struct ip_addr remote_addr;
static u16_t remote_port;

int CFG_LOG_PRINT_LEVEL = LDEBUG;

int esp32_ota_log(enum log_level level, char *format, ...)
{
  if (level < CFG_LOG_PRINT_LEVEL)
  {           // >config level, then printf
    return 0; // no printf
  }
  if (udp_ota_shell_pcb == NULL || remote_port == 0)
  {
    return 0;
  }

  struct pbuf *pkt_txbuf = pbuf_alloc(PBUF_TRANSPORT, 1400, PBUF_RAM);
  // ## init
  char *buf = (char *)pkt_txbuf->payload;
  int offset = 0;
  // ## level
  const char *evel_cr = "";
  if (level == LDEBUG)
  {
    evel_cr = "[debug]";
  }
  else if (level == LINFO)
  {
    evel_cr = "[info]";
  }
  else if (level == LWARN)
  {
    evel_cr = "[warn]";
  }
  else if (level == LERROR)
  {
    evel_cr = "[error]";
  }
  // ## conv
  offset = sprintf(buf, "%s", evel_cr);
  // ## output
  int i = 0;
  va_list vArgList;
  va_start(vArgList, format);
  char *sb = buf;
  offset += vsnprintf(sb + offset, 1400 - offset, format, vArgList);
  va_end(vArgList);
  pkt_txbuf->len = offset;
  pkt_txbuf->tot_len = offset;
  err_t err = udp_sendto(udp_ota_shell_pcb, pkt_txbuf, &remote_addr, remote_port);
  pbuf_free(pkt_txbuf);
  return err;
}

void log_help(void)
{
  esp32_ota_log(LINFO, "hi, frend!");
  // esp32_ota_log(LINFO, "nihao");
  // esp32_ota_log(LINFO, "ec600u_fota:");
  // esp32_ota_log(LINFO, "\tec600u_fota -u [url]");
  // esp32_ota_log(LINFO, "interpretation: Auto fota ec600u, http download -> verify -> reset -> fota.");
  // esp32_ota_log(LINFO, " ");
  // esp32_ota_log(LINFO, " ");
  // esp32_ota_log(LINFO, "esp32_flash:");
  // esp32_ota_log(LINFO, "\tesp32_flash http_download [-b/-p/-a]");
  // esp32_ota_log(LINFO, "\tesp32_flash flash");
  // esp32_ota_log(LINFO, "\tesp32_flash reset");
  // esp32_ota_log(LINFO, "\tesp32_flash rm_file [-b/-p/-a]");
  // esp32_ota_log(LINFO, "Options:");
  // esp32_ota_log(LINFO, "\t-h\tDisplay this help.");
  // esp32_ota_log(LINFO, "\t-b\tSelect object is bootloader.");
  // esp32_ota_log(LINFO, "\t-p\tSelect object is partition-table.");
  // esp32_ota_log(LINFO, "\t-a\tSelect object is app.");
}

// 从字符串中提取选项
int str_get_opt(char *str, char *opt, char *val, int max_len)
{
  LOGI("str_get_opt: %s, %s", str, opt);
  if (str == NULL || opt == NULL)
    return -1;
  char *p = strstr(str, opt);
  if (p == NULL)
    return -2;
  p += strlen(opt);
  while (*p == ' ')
    p++;
  char *q = strchr(p, ' ');
  if (q == NULL)
    q = str + strlen(str); // 尾后
  int len = (int)((uint32_t)q - (uint32_t)p);
  if (q - p + 1 > max_len)
    return -3;
  strncpy(val, p, len);
  val[len] = '\0';
  LOGI("str_get_opt: %s, %s, %s", str, opt, val);

  return 0;
}

void recv_callback_ota_shell_udp(void *arg, struct udp_pcb *upcb, struct pbuf *pkt_buf,
                                 struct ip_addr *addr, u16_t port)
{
  err_t err = ERR_OK;
  char *cmd;
  char *resp;
  int rxlen;

  cmd = (char *)pkt_buf->payload;
  rxlen = pkt_buf->len;
  *(cmd + rxlen) = '\0';
  LOGI("udp recived:%s", cmd);

  if (strstr(cmd, "-h") != NULL)
  {
    log_help();
  }

  if (strncmp(cmd, "esp32_flash flash", strlen("esp32_flash flash")) == 0) // 烧录程序
  {
    if (modbus_slave_thread)
    {
      osiEvent_t event;
      event.id = ESP32_FLASH_COMMAND_ESP32_FLASH;
      osiEventSend(modbus_slave_thread, &event);
      LOGI("esp32_flash flash");
    }
  }
  else if (strncmp(cmd, "esp32_flash http_download", strlen("esp32_flash http_download")) == 0) // http下载程序
  {
    LOGI("esp32_flash http_download");
    if (modbus_slave_thread)
    {
      osiEvent_t event;
      char url[256];
      if (str_get_opt(cmd, "-b", url, sizeof(url)) == 0)
      {
        char *str = malloc(strlen(url) + 1);
        if (str != NULL)
        {
          strcpy(str, url);
        }
        event.param1 = str;
        event.param2 = ESP32_FLASH_SDMMC_BOOTLOADER_FILENAME;
        event.id = ESP32_FLASH_COMMAND_HTTP_DOWNLOAD;
        osiEventSend(modbus_slave_thread, &event);
      }
      else if (str_get_opt(cmd, "-p", url, sizeof(url)) == 0)
      {
        char *str = malloc(strlen(url) + 1);
        if (str != NULL)
        {
          strcpy(str, url);
        }
        event.param1 = str;
        event.param2 = ESP32_FLASH_SDMMC_PARTITION_TABLE_FILENAME;
        event.id = ESP32_FLASH_COMMAND_HTTP_DOWNLOAD;
        osiEventSend(modbus_slave_thread, &event);
      }
      else if (str_get_opt(cmd, "-a", url, sizeof(url)) == 0)
      {
        char *str = malloc(strlen(url) + 1);
        if (str != NULL)
        {
          strcpy(str, url);
        }
        event.param1 = str;
        event.param2 = ESP32_FLASH_SDMMC_APP_FILENAME;
        event.id = ESP32_FLASH_COMMAND_HTTP_DOWNLOAD;
        osiEventSend(modbus_slave_thread, &event);
      }
      else
      {
        log_help();
        esp32_ota_log(LERROR, "cmd err");
      }
    }
  }
  else if (strncmp(cmd, "esp32_flash rm_file", strlen("esp32_flash rm_file")) == 0)
  {
    if (strstr(cmd, "-b"))
    {
      int ret = 0;
      ret = ql_remove(ESP32_FLASH_SDMMC_BOOTLOADER_FILENAME);
      if (ret)
      {
        esp32_ota_log(LERROR, "esp32_flash rm_file %s is failed, ret = 0x%x", ESP32_FLASH_SDMMC_BOOTLOADER_FILENAME, ret);
      }
      else
      {
        esp32_ota_log(LINFO, "esp32_flash rm_file %s successful", ESP32_FLASH_SDMMC_BOOTLOADER_FILENAME);
      }
    }
    else if (strstr(cmd, "-p"))
    {
      int ret = 0;
      ret = ql_remove(ESP32_FLASH_SDMMC_PARTITION_TABLE_FILENAME);
      if (ret)
      {
        esp32_ota_log(LERROR, "esp32_flash rm_file %s is failed, ret = 0x%x", ESP32_FLASH_SDMMC_PARTITION_TABLE_FILENAME, ret);
      }
      else
      {
        esp32_ota_log(LINFO, "esp32_flash rm_file %s successful", ESP32_FLASH_SDMMC_BOOTLOADER_FILENAME);
      }
    }
    else if (strnstr(cmd, "close", strlen("close")))
    {
      udp_ota_shell_pcb = NULL;
      remote_port = 0;
    }
    else
    {
      log_help();
      // esp32_ota_log(LERROR, "cmd err");
    }

    LOGI("esp32_flash rm_file");
  }
  else if (strncmp(cmd, "esp32_flash reset", strlen("esp32_flash reset")) == 0)
  {
    if (modbus_slave_thread)
    {
      osiEvent_t event;
      event.id = ESP32_FLASH_COMMAND_ESP32_RESET;
      osiEventSend(modbus_slave_thread, &event);
      LOGI("esp32_flash reset");
    }
  }
  else if (strncmp(cmd, "ec600u_fota", strlen("ec600u_fota")) == 0)
  {
    LOGI("ec600u_fota");
    char url[256];
    if (str_get_opt(cmd, "-u", url, sizeof(url)) == 0)
    {
      ql_fota_http_app_init(url);
    }
    else
    {
      // esp32_ota_log(LERROR, "cmd error");
      log_help();
      LOGI("cmd error");
    }
  }
  else if (strncmp(cmd, "ec600u_reboot", strlen("ec600u_reboot")) == 0)
  {
    LOGI("OK reboot");
    ql_rtos_task_sleep_s(1);
    ql_power_reset(RESET_NORMAL);
  }
  else if( strncmp(cmd, "download_voice_file", strlen("download_voice_file")) == 0)
  {
    LOGI("download_voice_file");
    char url[256];
    if (str_get_opt(cmd, "-u", url, sizeof(url)) == 0)
    {
      ql_fota_http_app_init(url);
    }
  }
  else
  {
    log_help();
    // esp32_ota_log(LERROR, "cmd error");
    LOGI("cmd error");
  }
  /* free the buffer pbuf */
  pbuf_free(pkt_buf);
  udp_ota_shell_pcb = upcb;
  remote_port = port;
  remote_addr = *addr;
}

void udp_ota_shell_init(void)
{
  LOGI("udp_ota_shell_init");
  err_t err;
  unsigned int port = 9999;
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
  /* TFTP server start  */
  udp_recv(UDPpcb, recv_callback_ota_shell_udp, NULL);
}
