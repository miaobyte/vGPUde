#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
{   
    blocklisthead
        uint64 element_size
        uint64 size
        uint64 freelist_next:
    blockdata[]{
            uint64 freelist_pre
            uint64 freelist_next
            [element_size]byte data
        }
}
*/
void  list_init(void *list, const size_t element_size, const size_t size);
void  list_mark_use(void *list);
void  list_mark_ususe(void *list);
static void  list_append(void *list);
static void  list_dellast(void *list);