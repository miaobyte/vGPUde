#ifndef __BTTES_H__
#define __BTTES_H__


#include <stddef.h>
#include <stdint.h>


typedef struct {
    uint8_t *data;
    size_t len;
} bytes_t;

#define BYTES_LITERAL(str) ((bytes_t){ .data = (uint8_t *)(str), .len = sizeof(str) - 1 })

#define BYTES_BUFFER(name,length) \
    uint8_t name##_buffer[length]; \
    bytes_t name = { .data = name##_buffer, .len = length }

#define BYTES_FUNC(func_name) void func_name(const bytes_t key)

#endif // __BTTES_H