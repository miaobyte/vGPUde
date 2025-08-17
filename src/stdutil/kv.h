#ifndef  KV_H
#define  KV_H

#include <stddef.h>
#include <stdint.h>


type KV struct {
	total       uint64
	chartypecnt uint8
	obj
}

func (kv KV) sizeofPtr() uint8 {
	e := math.Log2(float64(kv.total)) / 8
	return uint8(e)
}

//type metanode struct{
//	dataptr Ebyte
//	Ebyte[C] childs
//}

func (kv KV) sizeofmetaNode() (size uint32) {
	return uint32(1+kv.chartypecnt) * uint32(kv.sizeofPtr())
}

//type datanode struct {
//	bitmap   []bit
//	min, max Ebyte
//	objlist  []obj
//}

type obj struct {
	size                  Ebyte
	pre_nouse, next_nouse *obj
}

func (kv KV) sizeofdataNode(level int) (size uint32) {
	return uint32(2+kv.chartypecnt) * uint32(kv.sizeofPtr())
}


void* kv_malloc(const int64_t total, size_t size){

}
void kv_free(void *ptr);

#endif // 
