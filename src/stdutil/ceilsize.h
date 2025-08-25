#ifndef STDUTIL_CEILSIZE_H
#define STDUTIL_CEILSIZE_H

#include <stdint.h>

typedef struct {
    uint64_t ceil_value;  // 向上取整的值
    uint8_t power;        // 16的幂次
    uint8_t multiple;     // 倍数 (1-15)
} CeilSize;

CeilSize ceil_to_16_power(uint64_t n); 

#endif // STDUTIL_CEILSIZE_H