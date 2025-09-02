
#if NODE_COUNT <= 128
    #define NODE_INT int8_t
#elif NODE_COUNT <= 32768
    #define NODE_INT int16_t
#elif NODE_COUNT <= 2147483648
    #define NODE_INT int32_t
#else
    #define NODE_INT int64_t
#endif