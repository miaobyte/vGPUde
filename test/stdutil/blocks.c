#include <stdio.h>
#include <assert.h>


#include "stdutil/bytes.h"
#include "stdutil/block.h"

void test_alloc(blocks_t *blocks) {
    for (size_t i = 0; i < 62; i++) {
        block_t block = blocks_alloc(blocks);
        assert(block.id != (size_t)-1);
        printf("Allocated block id: %zu\n", block.id);
    }
}

#include <stdlib.h> // 用于随机数生成
#include <time.h>   // 用于随机数种子
void test_free(blocks_t *blocks) {
    block_t block = blocks_alloc(blocks);
    for(int i=0;i<10;i++){
         block=blocks_alloc(blocks);
    }
    blocks_free(blocks,9);
    blocks_alloc(blocks);
    blocks_free(blocks,9);
    blocks_free(blocks,3);
    block=blocks_alloc(blocks);
}

int main() {
    BYTES_BUFFER(pool,1024*4);
    init_blocks(pool.data, pool.len, 64);
    // test_alloc((blocks_t *)pool.data);
    test_free((blocks_t *)pool.data);
}