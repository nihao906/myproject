#ifndef __SDMMC_H
#define __SDMMC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sdmmc_init(void);
int sdmmc_write_bin_file(char *file_path, char *sdmmc_path);
int sdmmc_read_bin_file(char *file_path);
int sdmmc_get_file_size(char *file_path);
void sdmmc_create_file_dir(char *filename);
void waiting_sdmmc_mount(void);

void get_dirname(char *filename, char *dirname);

#endif