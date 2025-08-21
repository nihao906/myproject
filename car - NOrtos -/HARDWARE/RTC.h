#ifndef __RTC__H_
#define __RTC__H_
#include "stm32f4xx.h"                  // Device header
void RTC_init(void);
void RTC_setdatetime(void);
extern uint8_t RTC_flag;
#endif
