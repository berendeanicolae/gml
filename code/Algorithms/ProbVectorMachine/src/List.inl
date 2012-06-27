#ifndef __LIST_INL__
#define __LIST_INL__
//------------------------------------------------------------------------
#include "List.h"
#include "imp_memory.h"
#include "TemplateFunctions.inl"
//------------------------------------------------------------------------
template <typename T>
__imp_inline List<T>::~List()
{
	free_mem();
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline List<T>::List()
{
	memset(this, 0, sizeof(List<T>));
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline List<T>::List(List<T> &source)
{
	memset(this, 0, sizeof(List<T>));
	operator = (source);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T* List<T>::push_back(T* el)
{
	if (count == capacity)
		reserve((capacity << 1) + 1);

	v[count] = el;
	count++;
	return el;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T* List<T>::push_new()
{
	if (count == capacity)
		reserve((capacity << 1) + 1);

	imp_new(v[count], new T());
	count++;

	return v[count - 1];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T* List<T>::insert(int pos)
{
	T **nv;

	if (count == capacity)
		reserve((capacity << 1) + 1);

	if (pos == count)
		return PushNew();

	nv = (T**)imp_malloc((count - pos) * sizeof(T*));
	imp_memcpy(nv, v + pos, (count - pos) * sizeof(T*));
	imp_memcpy(v + pos + 1, nv, (count - pos) * sizeof(T*));
	imp_free(nv);

	imp_new(v[pos], new T());
	count++;

	return v[pos];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::pop_back()
{
	imp_assert(count > 0);
	imp_delete(v[count - 1]);
	count--;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::erase(int pos)
{
	T **nv;
	imp_assert(0 <= pos && pos < count);

	if (pos == count - 1)
	{
		pop_back();
		return;
	}

	nv = (T**)imp_malloc((count - pos - 1) * sizeof(T*));
	imp_memcpy(nv, v + pos + 1, (count - pos - 1) * sizeof(T*));
	imp_delete(v[pos]);
	imp_memcpy(v + pos, nv, (count - pos - 1) * sizeof(T*));
	imp_free(nv);

	count--;
	return;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline bool List<T>::erase(T* el)
{
	int i;

	for (i = 0; i < count; i++)
		if (v[i] == el)
		{
			Erase(i);
			return true;
		}

	return false;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T* List<T>::operator [] (int pos)
{
	imp_assert(0 <= pos && pos < count);
	return v[pos];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T* List<T>::at(int pos)
{
	return operator [](pos);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T& List<T>::at_ref(int pos)
{
	imp_assert(0 <= pos && pos < count);
	return *(v[pos]);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T* List<T>::front()
{
	return v[0];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T* List<T>::back()
{
	imp_assert(count);

	return v[count - 1];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::reserve(int ncapacity)
{
	T **nv;
	if (ncapacity <= capacity)
		return;

	nv = (T**)imp_malloc(ncapacity * sizeof(T*));
	if (count)
		imp_memcpy(nv, v, count * sizeof(T*));

	if (capacity)
		imp_free(v);

	v = nv;
	capacity = ncapacity;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::clear()
{
	int i;

	for (i = 0; i < count; i++)
		imp_delete(v[i]);

	count = 0;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline int List<T>::size()
{
	return count;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::free_mem()
{
	clear();
	if (v)
	{
		imp_free(v);
		v = 0;
		capacity = 0;
	}
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::resize(int ncount)
{
	int i;
	if (ncount <= count)
	{
		for (i = ncount; i < count; i++)
			imp_delete(v[i]);
	}
	else
	{
		reserve(ncount);

		for (i = count; i < ncount; i++)
			imp_new(v[i], new T());
	}

	count = ncount;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::from(const List<T> &source, int idx_start, int idx_stop)
{
	int i;

	imp_assert(0 <= idx_start && idx_start < idx_stop && idx_stop <= source.count);

	clear();
	resize(idx_stop - idx_start);

	for (i = 0; i < count; i++)
		*(v[i]) = *(source.v[idx_start + i]);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::from(const List<T> &source)
{
	from(source, 0, source.count);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::append(const List<T> &source, int idx_start, int idx_stop)
{
	int i, cidx;

	imp_assert(0 <= idx_start && idx_start < idx_stop && idx_stop <= source.count);
	resize(count + (idx_stop - idx_start));

	for (i = idx_stop, cidx = count; i < idx_start; i++, cidx++)
		*(v[cidx]) = *(source.v[i]);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline List<T>& List<T>::operator = (const List<T> &source)
{
	if (source.count)
		from(source, 0, source.count);
	else
		clear();

	return *this;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::q_sort(int (__cdecl *compare)(const void *el0, const void *el1))
{
	qsort(v, count, sizeof(T*), compare);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::q_sort()
{
	q_sort(type_compare_pointer_dual_func<T>);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline bool List<T>::operator == (const List<T> &src)
{
	int i;
	if (count != src.count) return false;
	for (i = 0; i < count; i++)
		if ((*(v[i])) != (*(src.v[i])))
			return false;

	return true;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void List<T>::switch_els(int pos_0, int pos_1)
{
	T* temp;
	imp_assert(0 <= pos_0 && pos_0 < count && 0 <= pos_1 && pos_1 < count);
	temp = v[pos_0];
	v[pos_0] = v[pos_1];
	v[pos_1] = temp;
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
#endif//__LIST_INL__
