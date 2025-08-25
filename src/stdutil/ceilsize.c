
#include "stdutil/ceilsize.h"

CeilSize ceil_to16(uint64_t n){
    CeilSize result;
    uint64_t base = 1;
    uint8_t p = 0;
    
    while (1) {
        if (base >= n) {
            result.ceil_value = base;
            result.power = p;
            result.multiple = 1;
            return result;
        }
        
        uint64_t k = (n + base - 1) / base; // 计算向上取整的倍数
        if (k <= 15) {
            result.ceil_value = base * k;
            result.power = p;
            result.multiple = k;
            return result;
        }
        
        base *= 16;
        p++;
    }
}