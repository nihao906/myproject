#ifndef SAVEDATA_H
#define SAVEDATA_H

void load_from_nvram(const char *key, void *data, size_t size, void *last_data);
void save_to_nvram(const char *key, void *data, size_t size, void *last_data);
void nvram_timer_callback(void *arg);

#endif
