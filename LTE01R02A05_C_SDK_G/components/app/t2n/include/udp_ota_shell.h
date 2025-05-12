#ifndef __UDP_OTA_SHELL_H
#define __UDP_OTA_SHELL_H

enum log_level
{
  LDEBUG = 0,
  LINFO = 1,
  LWARN = 2,
  LERROR = 3,
  LRESP = 99
};

int esp32_ota_log(enum log_level level, char *format, ...);
void udp_ota_shell_init(void);

#endif