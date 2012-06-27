#include "imp_memory.h"
//------------------------------------------------------------------------
#ifdef __RESTRICTION_MANAGER_MEMORY_DEBUGGER
//------------------------------------------------------------------------
memory_debugger mem_dbg;
//------------------------------------------------------------------------
memory_debugger::memory_debugger()
{
	memset(this, 0, sizeof(memory_debugger));
}
//------------------------------------------------------------------------
memory_debugger::~memory_debugger()
{
	//if (count || count_v)
		//imp_memory_assert(0);

	if (capacity)
		free(v);

	if (capacity_v)
		free(vv);

	memset(this, 0, sizeof(memory_debugger));
}
//------------------------------------------------------------------------
void memory_debugger::push_back(memory_block &blck)
{
	if (count == capacity)
		reserve((capacity << 1) + 1);

	v[count++] = blck;
}
//------------------------------------------------------------------------
void memory_debugger::reserve(int ncap)
{
	memory_block *nv;

	if (ncap <= capacity)
		return;

	nv = (memory_block *)malloc(ncap * sizeof(memory_block));
	memset(nv, 0, ncap * sizeof(memory_block));

	if (count)
		memcpy(nv, v, count * sizeof(memory_block));

	if (capacity)
		free(v);

	v = nv;
	capacity = ncap;
}
//------------------------------------------------------------------------
void memory_debugger::erase(int pos)
{
	memory_block *nv;

	imp_memory_assert(pos >= 0 && pos < count);

	if (pos == count - 1)
	{
		count --;
		return;
	}

	nv = (memory_block *)malloc((count - pos - 1) * sizeof(memory_block));
	memcpy(nv, v + pos + 1, (count - pos - 1) * sizeof(memory_block));
	memcpy(v + pos, nv, (count - pos - 1) * sizeof(memory_block));

	free(nv);
	count --;
}
//------------------------------------------------------------------------
void memory_debugger::sort()
{
	qsort(v, count, sizeof(memory_block), compare_memory_blocks_ascending);
}
//------------------------------------------------------------------------
int memory_debugger::get_pos(void *start)
{
	int i_l = 0, i_r = count, i_m;

	if (!count)
		return 0;

	while (i_r - i_l > 1)
	{
		i_m = (i_l + i_r) >> 1;

		if (start < v[i_m].start)
			i_r = i_m;
		else
			i_l = i_m;
	}

	if (start >= v[i_l].stop)
		return i_r;

	return i_l;
}
//------------------------------------------------------------------------
void* memory_debugger::__malloc(size_t sz, char file[__FILE_NAME_MAX_LENGTH], int line)
{
	memory_block mem_blk;
	void *ret = malloc(sz);

	imp_memory_assert(ret);

	mem_blk.start = ret;
	mem_blk.stop = (void *)(((size_t)ret) + sz);
	strcpy_s(mem_blk.file, __FILE_NAME_MAX_LENGTH, file);

	mem_blk.line = line;

	insert(mem_blk, get_pos(ret));

	return ret;
}
//------------------------------------------------------------------------
void memory_debugger::__free(void *mem)
{
	int pos = get_pos(mem);

	free(mem);

	if (!count || v[pos].stop <= mem)
		imp_memory_assert(0);

	if (v[pos].start == mem)
	{
		erase(pos);
		return;
	}

	if (v[pos].start < mem)
		v[pos].stop = mem;
	else
		imp_memory_assert(0);
}
//------------------------------------------------------------------------
void memory_debugger::__memset(void *start, char val, size_t sz)
{
	int pos = get_pos(start);
	int pos_v = get_pos_obj(start);

	if (pos_v < count_v && vv[pos_v].start == start)
	{
		if (pos < count)
			imp_memory_assert(start < v[pos].start || start >= v[pos].stop);

		if (pos_v < count_v - 1)
		{
    	imp_memory_assert((size_t)start + sz < (size_t)(vv[pos_v + 1].start));
		}
	}
	else
	{
		if (v[pos].start <= start && v[pos].stop > start)
		{
			if ((size_t)start + sz > (size_t)v[pos].stop)
				imp_memory_assert(0);
		}
		else
			imp_memory_assert(0);
	}

	memset(start, val, sz);
}
//------------------------------------------------------------------------
void memory_debugger::__memcpy(void *dest, void *source, size_t sz)
{
	int pos1 = get_pos(dest), pos2 = get_pos(source);
	int pos_v1 = get_pos_obj(dest), pos_v2 = get_pos_obj(source);

	if (pos_v1 < count_v && vv[pos_v1].start == dest)
	{
		if (pos_v1 < count_v - 1)
			imp_memory_assert((size_t)(vv[pos_v1 + 1].start) > (size_t)dest + sz);
	}
	else
	{
		if (v[pos1].start > dest || (size_t)dest + sz > (size_t)v[pos1].stop)
			imp_memory_assert(0);
	}

	if (pos_v2 < count_v && vv[pos_v2].start == source)
	{
		if (pos_v2 < count_v - 1)
			imp_memory_assert((size_t)(vv[pos_v2 + 1].start) > (size_t)source + sz);
	}
	else
	{
		if (v[pos2].start > source || (size_t)source + sz > (size_t)v[pos2].stop)
			imp_memory_assert(0);
	}

	memcpy(dest, source, sz);
}
//------------------------------------------------------------------------
int compare_memory_blocks_ascending(const void *bl1, const void *bl2)
{
	memory_block *mb1 = (memory_block *)bl1,
							 *mb2 = (memory_block *)bl2;

	if (mb1->start < mb2->start ||
		 (mb1->start == mb2->start && mb1->stop < mb2->stop))
		return -1;
	else if (mb1->start > mb2->start ||
					(mb1->start == mb2->start && mb1->stop > mb2->stop))
		return 1;

	return 0;
}
//------------------------------------------------------------------------
void memory_block::operator = (memory_block &mem_blk)
{
	start = mem_blk.start;
	stop = mem_blk.stop;
	strcpy_s(file, __FILE_NAME_MAX_LENGTH, mem_blk.file);
	line = mem_blk.line;
	idx = mem_blk.idx;
}
//------------------------------------------------------------------------
void object_block::operator = (object_block &obj_blk)
{
	memcpy(this, &obj_blk, sizeof(object_block));
}
//------------------------------------------------------------------------
void memory_debugger::insert(memory_block &blck, int pos)
{
	memory_block *nv;

	imp_memory_assert(pos >= 0 && pos <= count);

	blck.idx = midx;
	midx++;
	if (pos == count)
	{
		push_back(blck);
		return;
	}

	if (count == capacity)
		reserve((capacity << 1) + 1);

	nv = (memory_block *)malloc((count - pos) * sizeof(memory_block));

	memcpy(nv, v + pos, (count - pos) * sizeof(memory_block));
	memcpy(v + pos + 1, nv, (count - pos) * sizeof(memory_block));

	v[pos] = blck;
	count++;

	free(nv);
}
//------------------------------------------------------------------------
int memory_debugger::__memcmp(void *dest, void *source, size_t sz)
{
	int pos1 = get_pos(dest), pos2 = get_pos(source);

	if (v[pos1].start > dest || (size_t)dest + sz > (size_t)v[pos1].stop)
		imp_memory_assert(0);

	if (v[pos2].start > source || (size_t)source + sz > (size_t)v[pos2].stop)
		imp_memory_assert(0);

	return memcmp(dest, source, sz);
}
//------------------------------------------------------------------------
void memory_debugger::__memmove(void *dest, void *src, size_t sz)
{
  int pos1 = get_pos(dest), pos2 = get_pos(src);

  imp_memory_assert(0 <= pos1 && pos1 < count);
  imp_memory_assert(((size_t)v[pos1].start) <= (size_t)dest && ((size_t)dest) + sz <= ((size_t)v[pos1].stop));

  imp_memory_assert(0 <= pos2 && pos2 < count);
  imp_memory_assert(((size_t)v[pos2].start) <= (size_t)src && ((size_t)src) + sz <= ((size_t)v[pos2].stop));

  memmove(dest, src, sz);
}
//------------------------------------------------------------------------
void* memory_debugger::__realloc(void *ptr, size_t sz, char file[__FILE_NAME_MAX_LENGTH], int line)
{
	int pos;
  void *ret;
  memory_block mem_blk;

  if (ptr)
  {
    pos = get_pos(ptr);
    if (!count || v[pos].stop <= ptr)
      imp_memory_assert(0);

    if (v[pos].start == ptr)
      erase(pos);
    else
    {
      if (v[pos].start < ptr)
        v[pos].stop = ptr;
      else
        imp_memory_assert(0);
    }
  }

  ret = realloc(ptr, sz);

	imp_memory_assert(ret);

	mem_blk.start = ret;
	mem_blk.stop = (void *)(((size_t)ret) + sz);
	strcpy_s(mem_blk.file, __FILE_NAME_MAX_LENGTH, file);
	mem_blk.line = line;

	insert(mem_blk, get_pos(ret));

	return ret;
}
//------------------------------------------------------------------------
void memory_debugger::__new(void *mem, char file[__FILE_NAME_MAX_LENGTH], int line)
{
	int pos = get_pos_obj(mem);
	object_block obj;

	obj.start = mem;
	strcpy_s(obj.file, __FILE_NAME_MAX_LENGTH, file);
	obj.line = line;

	insert_obj(obj, pos);
}
//------------------------------------------------------------------------
void memory_debugger::__delete(void *dest)
{
	int pos = get_pos_obj(dest);

	imp_memory_assert(dest == vv[pos].start);

	erase_obj(pos);
}
//------------------------------------------------------------------------
void memory_debugger::push_back_obj(object_block &obj)
{
	if (count_v == capacity_v)
		reserve_obj((capacity_v << 1) + 1);

	vv[count_v++] = obj;
}
//------------------------------------------------------------------------
void memory_debugger::insert_obj(object_block &mem, int pos)
{
	object_block *vvv;

	imp_memory_assert(pos >= 0 && pos <= count_v);
	if (pos == count_v)
	{
		push_back_obj(mem);
		return;
	}

	if (count_v == capacity_v)
		reserve_obj((capacity_v << 1) + 1);

	vvv = (object_block *)malloc((count_v - pos) * sizeof(object_block));

	memcpy(vvv, vv + pos, (count_v - pos) * sizeof(object_block));
	memcpy(vv + pos + 1, vvv, (count_v - pos) * sizeof(object_block));

	vv[pos] = mem;
	count_v++;

	free(vvv);
}
//------------------------------------------------------------------------
void memory_debugger::reserve_obj(int ncap_v)
{
	object_block *nvv;

	if (ncap_v <= capacity_v)
		return;

	nvv = (object_block *)malloc(ncap_v * sizeof(object_block));
	memset(nvv, 0, ncap_v * sizeof(object_block));

	if (count_v)
		memcpy(nvv, vv, count_v * sizeof(object_block));

	if (capacity_v)
		free(vv);

	vv = nvv;
	capacity_v = ncap_v;
}
//------------------------------------------------------------------------
void memory_debugger::erase_obj(int pos_v)
{
	object_block *nv;

	imp_memory_assert(pos_v >= 0 && pos_v < count_v);

	if (pos_v == count_v - 1)
	{
		count_v --;
		return;
	}

	nv = (object_block *)malloc((count_v - pos_v - 1) * sizeof(object_block));
	memcpy(nv, vv + pos_v + 1, (count_v - pos_v - 1) * sizeof(object_block));
	memcpy(vv + pos_v, nv, (count_v - pos_v - 1) * sizeof(object_block));

	free(nv);
	count_v --;
}
//------------------------------------------------------------------------
void memory_debugger::sort_obj()
{
	qsort(vv, count_v, sizeof(object_block), compare_object_block_ascending);
}
//------------------------------------------------------------------------
int memory_debugger::get_pos_obj(void *start)
{
	int i_l = 0, i_r = count_v, i_m;

	if (!count_v)
		return 0;

	while (i_r - i_l > 1)
	{
		i_m = (i_l + i_r) >> 1;

		if ((int)start < (int)(vv[i_m].start))
			i_r = i_m;
		else
			i_l = i_m;
	}

	if ((int)vv[i_l].start > (int)start)
		return i_l;

	if (vv[i_l].start != start)
		return i_r;

	return i_l;
}
//------------------------------------------------------------------------
int compare_pointer_obj_ascending(const void *bl1, const void *bl2)
{
	int pos1 = (int)(*((void **)(bl1))), pos2 = (int)(*((void **)(bl2)));

	if (pos1 < pos2)
		return -1;
	else if (pos1 > pos2)
		return 1;

	return 0;
}
//------------------------------------------------------------------------
int compare_object_block_ascending(const void *bl1, const void *bl2)
{
	int pos1 = (int)(((object_block *)bl1)->start),
			pos2 = (int)(((object_block *)bl2)->start);

	if (pos1 < pos2)
		return -1;
	else if (pos2 < pos1)
		return 1;

	return 0;

}
//------------------------------------------------------------------------
#endif//__RESTRICTION_MANAGER_MEMORY_DEBUGGER
//------------------------------------------------------------------------
