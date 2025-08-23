#include <stdint.h>

// 计算数组中 2 位单元的数量
#define BIT2_COUNT(arr_size) ((arr_size) * 4)

// 获取指定索引的 2 位单元值 (0-3)
#define GET_BIT2(arr, idx) (((arr)[(idx) / 4] >> (((idx) % 4) * 2)) & 0x03)

// 设置指定索引的 2 位单元值 (值必须在 0-3 范围内)
#define SET_BIT2(arr, idx, val) do { \
    uint8_t mask = ~(0x03 << (((idx) % 4) * 2)); \
    (arr)[(idx) / 4] = ((arr)[(idx) / 4] & mask) | (((val) & 0x03) << (((idx) % 4) * 2)); \
} while(0)