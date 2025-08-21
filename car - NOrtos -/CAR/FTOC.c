#include "stm32f4xx.h"                  // Device header
#include <stdbool.h>

// 功能: 将 float 类型转换为字符串
// 参数:
//   number - 要转换的浮点数
//   buffer - 输出字符串缓冲区
//   decimal_places - 保留小数位数（建议 0~6）
// 示例: float_to_string(3.14159, buf, 2) --> "3.14"
void float_to_string(float number, char *buffer, int decimal_places)
{
    int int_part = (int)number; // 整数部分
    float fraction = number - int_part;

    // 处理负数
    if (number < 0)
    {
        *buffer++ = '-';
        number = -number;
        int_part = (int)number;
        fraction = number - int_part;
    }

    // 整数部分转换
    char temp[20];
    int i = 0;
    do {
        temp[i++] = (int_part % 10) + '0';
        int_part /= 10;
    } while (int_part > 0);

    // 反转整数部分写入 buffer
    while (i > 0) {
        *buffer++ = temp[--i];
    }

    // 小数部分处理
    if (decimal_places > 0)
    {
        *buffer++ = '.';

        for (int j = 0; j < decimal_places; j++)
        {
            fraction *= 10;
            int digit = (int)fraction;
            *buffer++ = digit + '0';
            fraction -= digit;
        }
    }

    *buffer = '\0';
}

void int16_to_str(int16_t num, char *str)
{
    char buf[7]; // int16 范围是 -32768 ~ 32767，共6位 + '\0'
    int i = 0;
    bool isNegative = false;

    // 处理负数
    if (num < 0) {
        isNegative = true;
        num = -num;
    }

    // 特殊情况0
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    // 转换数字部分（逆序）
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (isNegative) {
        buf[i++] = '-';
    }

    // 反转字符串
    for (int j = 0; j < i; j++) {
        str[j] = buf[i - j - 1];
    }
    str[i] = '\0'; // 结束符
}

