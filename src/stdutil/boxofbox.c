#include "boxofbox.h"

void boxofbox_init(boxofbox_meta* meta, size_t size, size_t block_size,void *boxstart){
    *meta=(boxofbox_meta){
        .total_size=size,
        .block_size=block_size,
        .max=block_size- sizeof(obj),
        .min=1,
        .boxstart=boxstart
    };

    
}
void *boxofbox_alloc(boxofbox_meta* meta, size_t size);
void boxofbox_free(boxofbox_meta* meta, void *ptr);