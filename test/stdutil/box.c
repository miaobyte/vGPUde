#include <stdint.h>

#include "stdutil/box.h"



void test_boxinit(void* ptr, void* data){
    box_init(ptr,1024*1024, data,1024*1024*16);
}
 
int main(int argc, char *argv[]) {

    uint8_t buddy[1024*1024];

    uint8_t data[1024*1024*16];
    test_boxinit(buddy, data);
    return 0;
}