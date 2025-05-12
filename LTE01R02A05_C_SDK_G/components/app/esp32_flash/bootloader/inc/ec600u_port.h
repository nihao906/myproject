#ifndef _EC600U_H
#define _EC600U_H

#include <stdint.h>
#include <stdlib.h>
#include "esp_loader.h"

int  loader_port_ec600u_init(void);
void loader_port_ec600u_deinit(void);
esp_loader_error_t  loader_port_write(const uint8_t *data, uint16_t size, uint32_t timeout);
esp_loader_error_t  loader_port_read(uint8_t *data, uint16_t size, uint32_t timeout);
void loader_port_enter_bootloader(void);
void loader_port_reset_target(void);
void loader_port_delay_ms(uint32_t ms);
void loader_port_start_timer(uint32_t ms);
uint32_t loader_port_remaining_time(void);
void loader_port_debug_print(const char *str);
esp_loader_error_t loader_port_change_transmission_rate(uint32_t baudrate);

void loader_port_ec600u_init_gpio(void);
void loader_port_ec600u_deinit_gpio(void);

#endif