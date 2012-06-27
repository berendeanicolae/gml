#ifndef __RESTRICTION_MANAGER_MEMORY_H__
#define __RESTRICTION_MANAGER_MEMORY_H__
//---------------------------------------------------------------------------
#include "imp_assert.h"
#include <string.h>
#include <stdlib.h>
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef __RESTRICTION_MANAGER_MEMORY_DEBUGGER
//---------------------------------------------------------------------------
#define __FILE_NAME_MAX_LENGTH		_MAX_PATH
#define imp_memory_assert					imp_assert
//#define imp_memory_assert(expr)		void(0)
//---------------------------------------------------------------------------
typedef struct memory_block
{
	void *start;
	void *stop;
	char file[__FILE_NAME_MAX_LENGTH];
	int line;
	int idx;

	void operator = (memory_block &mem_blk);
}memory_block;
int compare_memory_blocks_ascending(const void *bl1, const void *bl2);
int compare_pointer_void_ascending(const void *bl1, const void *bl2);
//---------------------------------------------------------------------------
typedef struct object_block
{
	void *start;
	char file[__FILE_NAME_MAX_LENGTH];
	int line;

	void operator = (object_block &obj_blk);
}object_block;
//---------------------------------------------------------------------------
int compare_object_block_ascending(const void *bl1, const void *bl2);
//---------------------------------------------------------------------------
struct memory_debugger
{
	int midx;
	memory_block *v;
	object_block *vv;

	int count, capacity;
	int count_v, capacity_v;

	memory_debugger();
	~memory_debugger();

	void push_back(memory_block &blck);
	void insert(memory_block &blck, int pos);
	void reserve(int ncap);
	void erase(int pos);
	void sort();
	int  get_pos(void *start);

	void push_back_obj(object_block &obj);
	void insert_obj(object_block &mem, int pos);
	void reserve_obj(int ncap_v);
	void erase_obj(int pos_v);
	void sort_obj();
	int get_pos_obj(void *start);

	void*	__malloc(size_t sz, char file[__FILE_NAME_MAX_LENGTH], int line);
	void	__free(void *mem);
	void	__memset(void *start, char val, size_t sz);
	void	__memcpy(void *dest, void *source, size_t sz);
	int		__memcmp(void *dest, void *source, size_t sz);
	void	__memmove(void *dest, void *src, size_t sz);
	void* __realloc(void *ptr, size_t sz, char file[__FILE_NAME_MAX_LENGTH], int line);
	void	__new(void *mem, char file[__FILE_NAME_MAX_LENGTH], int line);
	void	__delete(void *dest);
};
//---------------------------------------------------------------------------
extern memory_debugger mem_dbg;
//---------------------------------------------------------------------------
#define imp_malloc(size)							(mem_dbg.__malloc(size, __FILE__, __LINE__))
#define imp_free											mem_dbg.__free
#define imp_memset										mem_dbg.__memset
#define imp_memcpy										mem_dbg.__memcpy
#define imp_memcmp										mem_dbg.__memcmp
#define imp_memmove                   mem_dbg.__memmove
#define imp_new(obj, constructor)			(obj = constructor, mem_dbg.__new((void *)obj, __FILE__, __LINE__))
#define imp_delete(obj)								(delete(obj), mem_dbg.__delete((void *)obj))
#define	imp_realloc(ptr, sz)					mem_dbg.__realloc(ptr, sz, __FILE__, __LINE__)
#else//__RESTRICTION_MANAGER_MEMORY_DEBUGGER
//---------------------------------------------------------------------------
#define imp_malloc										malloc
#define imp_free											free
#define imp_memset										memset
#define imp_memcpy										memcpy
#define imp_memcmp										memcmp
#define imp_memmove                   memmove
#define imp_new(obj, constructor)			obj = constructor
#define imp_delete(x)									if((x)) {delete((x));(x) = NULL;}
//---------------------------------------------------------------------------
#endif//__RESTRICTION_MANAGER_MEMORY_DEBUGGER
//---------------------------------------------------------------------------
//#define imp_zero_mem(dest, sz)						imp_memset(dest, 0, sz)
#define imp_zero_mem_nsf(dest, sz)				memset(dest, 0, sz)
#define imp_memcpy_nsf										memcpy
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif//__RESTRICTION_MANAGER_MEMORY_H__
