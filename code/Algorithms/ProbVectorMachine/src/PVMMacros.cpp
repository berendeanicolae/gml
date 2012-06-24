#include "PVMMacros.h"
#include <cstdio>

void* __ptr__;

void* pvm_malloc_func (int val) {
	void* __ptr__ = malloc(val); 
	printf("malloc-> ptr: %08x; size: %.02f MB; at file: %s line: %d\n", __ptr__, float(val)/(1024.0*1024.0),__FILE__, __LINE__);
	return __ptr__;
}

void pvm_free_func (void* __ptr__) {
	free(__ptr__); 
	printf("free-> ptr: %08x; at file: %s line: %d\n", __ptr__, __FILE__, __LINE__);
}