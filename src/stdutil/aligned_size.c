#include "stdutil/aligned_size.h"
#include "stdutil/logutil.h"

static uint64_t int_pow(uint64_t base, uint32_t exp) {
    uint64_t result = 1;
    for (uint32_t i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}
uint32_t int_log(uint64_t n, uint32_t base) {
    uint32_t log = 0;
    while (n >= base) {
        n /= base;
        log++;
    }
    return log;
}

AlignedSize align_to(uint64_t n, uint32_t base) {
    if(n<0 ) {
        // Invalid input, return zeroed structure
        LOG("Invalid arg n ", n);
        AlignedSize result = {0, 0, 0, 0};
        return result;
    }
    if(base<1) {
        // Invalid input, return zeroed structure
        LOG("Invalid arg base ", base);
        AlignedSize result = {0, 0, 0, 0};
        return result;
    }
    AlignedSize result = {
        .base = base,
        .aligned_value = 0,
        .power = 0,
        .multiple = 0
    };

    if (n < base) {
        result.power = 1;
        result.multiple = 1;
        result.aligned_value = base;
        return result;
    }

    result.power = int_log(n, base);
    uint64_t minbase = int_pow(base, result.power);

    result.multiple = (n + minbase - 1) / minbase;
    if (result.multiple >= base) {
        result.multiple = 1;
        result.power++;
        minbase = int_pow(base, result.power);
    }

    result.aligned_value = minbase * result.multiple;
    return result;
}