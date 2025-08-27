#include <stdint.h>
#include <stdlib.h>
#include "stdutil/box.h"



void test_boxinit(void* metaptr, void* data){
    box_init(metaptr,1024*1024, data,1024*1024*16);
}


int main(int argc, char *argv[]) {

    uint8_t *buddy = malloc(1024*1024);

    uint8_t *data = malloc(1024*1024*16);
    test_boxinit(buddy, data);

    void* p5=box_alloc(buddy,5);
    void* p7=box_alloc(buddy,7);

    box_free(buddy,p5);
    box_free(buddy,p7);

    free(buddy);
    free(data);
    return 0;
}