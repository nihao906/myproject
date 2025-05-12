#ifndef SNPRINTF_FUN_H
#define SNPRINTF_FUN_H

#include <unistd.h>
#include <errno.h>

int my_read(int fd, void *buf, size_t count);

int my_write(int fd, const void *buf, size_t count);

off_t my_lseek(int fd, off_t offset, int whence);

int my_close(int fd);

#endif