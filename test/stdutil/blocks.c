#include <stdio.h>
#include <assert.h>


#include "stdutil/bytes.h"
#include "stdutil/block.h"

void test_alloc(blocks_meta *blocks) {
    block_t* block ;
    for (size_t i = 0; i < 62; i++) {
        block= blocks_alloc(blocks);
        assert(block->id != (size_t)-1);
        printf("Allocated block id: %zu\n", block->id);
    }
}

#include <stdlib.h> // 用于随机数生成
#include <time.h>   // 用于随机数种子
void test_free(blocks_meta *blocks) {
    block_t* block = blocks_alloc(blocks);
    for(int i=0;i<10;i++){
         block=blocks_alloc(blocks);
         LOG("alloc block %zu,used %d ，next.freeid %zu",block.id,block.used,block.free_next_id);
    }
    blocks_free(blocks,9);
    blocks_alloc(blocks);
    blocks_free(blocks,9);
    blocks_free(blocks,3);
    blocks_alloc(blocks);
    blocks_free(blocks,5);
    blocks_free(blocks,4);
    blocks_alloc(blocks);
}

int main() {

    BYTES_BUFFER(pool,1024*4);
    blocks_meta blocks;
    init_blocks(pool.data,pool.len, 64, &blocks);
    // test_alloc((blocks_meta *)pool.data);
    test_free(&blocks);
}