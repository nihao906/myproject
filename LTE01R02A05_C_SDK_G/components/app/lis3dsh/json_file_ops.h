#ifndef __JSON_FILE_OPS_H
#define __JSON_FILE_OPS_H

#include "cJSON.h"

#define SHAKE_CFG_FILE_PATH "SD:/shake/shake_cfg.json"

void save_shake_cfg(float feq_min, float feq_max, float a_th, const char *filePath);
int load_shake_cfg(float *feq_min_q, float *feq_max_q, float *a_th_q, const char *filePath);

#endif