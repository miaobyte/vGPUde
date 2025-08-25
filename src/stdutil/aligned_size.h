#ifndef STDUTIL_CEILSIZE_H
#define STDUTIL_CEILSIZE_H

#include <stdint.h>

typedef struct {
    uint64_t aligned_value;  // 对齐后的值 aligned_value=multiple*pow(base,power)
    uint32_t base;         // 基数
    uint32_t power;       //  base的幂次
    uint32_t multiple;    //  multiple=[1,base]
} AlignedSize;

AlignedSize align_to(uint64_t n, uint32_t base);

#endif // STDUTIL_CEILSIZE_H