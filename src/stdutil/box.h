#ifndef STDUTIL_BOX_H
#define STDUTIL_BOX_H

#include <stddef.h>
#include <stdint.h>

int box_init(void *meta,size_t buddysize, void *boxstart, size_t box_size);
void *box_alloc(void *meta, size_t size);
void box_free(void *meta, void *ptr);

#endif // STDUTIL_BOX_H