#ifndef NFTL_UTIL_H
#define NFTL_UTIL_H

// System includes
#include <assert.h>
#include "osi_api.h"
#include "osi_log.h"

#define NFTL_YES_TRACE

// Logging functions
#ifdef NFTL_YES_TRACE
#define NFTL_TRACE(fmt, ...) \
    OSI_PRINTFI("nftl_trace:%s(%d): " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define NFTL_TRACE_ASSERT(test, fmt, ...)                                                     \
    do                                                                                        \
    {                                                                                         \
        if (!(test))                                                                          \
        {                                                                                     \
            nftl_dump(NULL);                                                                  \
            OSI_PRINTFI("nftl_trace_assert:%s(%d): " fmt, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                                     \
        OSI_ASSERT(test, "");                                                                 \
    } while (0)
#else
#define NFTL_TRACE(fmt, ...)
#define NFTL_TRACE_ASSERT(test, msg)
#endif

#ifndef NFTL_NO_DEBUG
#define NFTL_DEBUG(fmt, ...) \
    OSI_PRINTFD("nftl_debug:%s(%d): " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define NFTL_DEBUG(fmt, ...)
#endif

#ifndef NFTL_NO_WARN
#define NFTL_WARN(fmt, ...) \
    OSI_PRINTFW("nftl_warn:%s(%d): " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define NFTL_WARN(fmt, ...)
#endif

#ifndef NFTL_NO_ERROR
#define NFTL_ERROR(fmt, ...) \
    OSI_PRINTFE("nftl_error:%s(%d): " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define NFTL_ERROR(fmt, ...)
#endif

// Runtime assertions
#ifndef NFTL_NO_ASSERT
#define NFTL_ASSERT(test) OSI_ASSERT(test, "")
#else
#define NFTL_ASSERT(test)
#endif

// Convert between 32-bit little-endian and native order
static inline uint32_t nftl_fromle32(uint32_t a)
{
#if !defined(NFTL_NO_INTRINSICS) && ((defined(BYTE_ORDER) && defined(ORDER_LITTLE_ENDIAN) && BYTE_ORDER == ORDER_LITTLE_ENDIAN) ||         \
                                     (defined(__BYTE_ORDER) && defined(__ORDER_LITTLE_ENDIAN) && __BYTE_ORDER == __ORDER_LITTLE_ENDIAN) || \
                                     (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__))
    return a;
#elif !defined(NFTL_NO_INTRINSICS) && ((defined(BYTE_ORDER) && defined(ORDER_BIG_ENDIAN) && BYTE_ORDER == ORDER_BIG_ENDIAN) ||         \
                                       (defined(__BYTE_ORDER) && defined(__ORDER_BIG_ENDIAN) && __BYTE_ORDER == __ORDER_BIG_ENDIAN) || \
                                       (defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))
    return __builtin_bswap32(a);
#else
    return (((uint8_t *)&a)[0] << 0) |
           (((uint8_t *)&a)[1] << 8) |
           (((uint8_t *)&a)[2] << 16) |
           (((uint8_t *)&a)[3] << 24);
#endif
}

static inline uint32_t nftl_tole32(uint32_t a)
{
    return nftl_fromle32(a);
}

// Convert between 16-bit little-endian and native order
static inline uint16_t nftl_fromle16(uint16_t a)
{
#if !defined(NFTL_NO_INTRINSICS) && ((defined(BYTE_ORDER) && defined(ORDER_LITTLE_ENDIAN) && BYTE_ORDER == ORDER_LITTLE_ENDIAN) ||         \
                                     (defined(__BYTE_ORDER) && defined(__ORDER_LITTLE_ENDIAN) && __BYTE_ORDER == __ORDER_LITTLE_ENDIAN) || \
                                     (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__))
    return a;
#elif !defined(NFTL_NO_INTRINSICS) && ((defined(BYTE_ORDER) && defined(ORDER_BIG_ENDIAN) && BYTE_ORDER == ORDER_BIG_ENDIAN) ||         \
                                       (defined(__BYTE_ORDER) && defined(__ORDER_BIG_ENDIAN) && __BYTE_ORDER == __ORDER_BIG_ENDIAN) || \
                                       (defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))
    return __builtin_bswap16(a);
#else
    return (((uint8_t *)&a)[0] << 0) |
           (((uint8_t *)&a)[1] << 8);
#endif
}

static inline uint16_t nftl_tole16(uint16_t a)
{
    return nftl_fromle16(a);
}

// Min/max functions for unsigned 32-bit numbers
static inline uint32_t nftl_max(uint32_t a, uint32_t b)
{
    return (a > b) ? a : b;
}

static inline uint32_t nftl_min(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
}

// Align to nearest multiple of a size
static inline uint32_t nftl_aligndown(uint32_t a, uint32_t alignment)
{
    return a - (a % alignment);
}

static inline unsigned long long nftl_alignup(unsigned long long a, uint32_t alignment)
{
    return nftl_aligndown(a + alignment - 1, alignment);
}

// Find the sequence comparison of a and b, this is the distance
// between a and b ignoring overflow
static inline int nftl_scmp(uint32_t a, uint32_t b)
{
    return (int)(unsigned)(a - b);
}

// calc CRC32
uint32_t nftl_crc32(uint32_t init, const void *buffer, size_t size);

#endif /* NFTL_UTIL_H */
