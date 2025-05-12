#include <unistd.h>
#include <errno.h>

int my_read(int fd, void *buf, size_t count) {
    errno = ENOSYS;  // 函数未实现
    return -1;
}

int my_write(int fd, const void *buf, size_t count) {
    errno = ENOSYS;
    return -1;  // 或 return count 模拟成功
}

off_t my_lseek(int fd, off_t offset, int whence) {
    errno = ENOSYS;
    return -1;
}

int my_close(int fd) {
    errno = ENOSYS;
    return -1;
}
