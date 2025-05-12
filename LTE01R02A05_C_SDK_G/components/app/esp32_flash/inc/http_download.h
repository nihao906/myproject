#ifndef __HTTP_DOWNLOAD_H
#define __HTTP_DOWNLOAD_H

#include "esp32_flash.h"

#define MAX_HTTP_CLIENT_REQUEST_QUEUE_SIZE 8

int check_data_call(void);
int http_dowload(char *event_param1, char *file_name);

#endif