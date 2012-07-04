#ifndef __VECTOR_INL__
#define __VECTOR_INL__
//------------------------------------------------------------------------
#include "Vector.h"
#include "imp_memory.h"
#include "TemplateFunctions.inl"
//------------------------------------------------------------------------
using namespace IMP;
//------------------------------------------------------------------------
template <typename T>
__imp_inline IMP::Vector<T>::Vector()
{
	memset(this, 0, sizeof(Vector<T>));
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline IMP::Vector<T>::~Vector()
{
	if (capacity)
		imp_free(v);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::push_back(const T& bc)
{
	if (count == capacity)
		reserve((capacity << 1) + 1);

	v[count++] = bc;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::insert(const T& bc, int pos)
{
	imp_assert(pos >= 0 && pos <= count);

	if (pos >= count)
	{
		push_back(bc);
		return;
	}

	if (count == capacity)
		reserve((capacity << 1) + 1);

	imp_memmove(v + pos + 1, v + pos, (count - pos) * sizeof(T));
	v[pos] = bc;

	count++;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T& IMP::Vector<T>::operator [] (int pos)
{
	imp_assert(pos >= 0 && pos < count);

	return v[pos];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::reserve(int ncap)
{
	T *nv;

	if (ncap <= capacity)
		return;

	nv = (T *)imp_malloc(ncap * sizeof(T));
	imp_memset(nv, 0, ncap * sizeof(T));

	if (count)
		imp_memcpy(nv, v, count * sizeof(T));

	if (capacity)
		imp_free(v);

	v = nv;
	capacity = ncap;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::clear()
{
	count = 0;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline int IMP::Vector<T>::size()
{
	return count;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::erase(int pos)
{
	imp_assert(pos >= 0 && pos < count);

	if (pos == count - 1)
	{
		pop_back();
		return;
	}

	imp_memmove(v + pos, v + pos + 1, (count - pos - 1) * sizeof(T));
	count--;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::erase(int start_pos, int stop_pos)
{
	imp_assert(start_pos >= 0 && stop_pos >= start_pos && stop_pos <= count && start_pos < count);

	if (count != stop_pos && start_pos != stop_pos)
		imp_memmove(v + start_pos, v + stop_pos, (count - stop_pos) * sizeof(T));

	count -= stop_pos - start_pos;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::pop_back()
{
	count --;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::free_mem()
{
	if (capacity)
		imp_free(v);

	memset(this, 0, sizeof(Vector<T>));
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::assign_val(const T &val)
{
	int i;
	for (i = 0; i < count; i++) v[i] = val;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::assign_val(const T &val, int start_idx, int stop_idx)
{
	int i;
	imp_assert(0 <= start_idx && start_idx <= stop_idx && stop_idx <= count && start_idx < count);
	for (i = start_idx; i < stop_idx; i++) v[i] = val;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::resize(int ncount)
{
	if (ncount > capacity)
		reserve(ncount);

	count = ncount;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::zero_all()
{
	imp_memset(v, 0, count * sizeof(T));
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline IMP::Vector<T>& IMP::Vector<T>::operator = (const IMP::Vector<T> &bv)
{
	imp_assert(&bv != this);
	resize(bv.count);
	if (count)	//copiem doar daca avem ce
		imp_memcpy(v, bv.v, count * sizeof(T));
	return *this;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::assign(T* classic_vector, int number_of_elements)
{
	reserve(number_of_elements);
	imp_assert(count >= number_of_elements);	
	memcpy(v, classic_vector, number_of_elements * sizeof(T));
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::fill_memory_location(T* classic_vector, int no_elem)
{
	imp_assert(no_elem <= count);
	memcpy(classic_vector, v, no_elem * sizeof(T));
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::from(const IMP::Vector<T> &source, int idx_start, int idx_stop)
{
	imp_assert(0 <= idx_start && idx_start <= idx_stop && idx_stop <= source.count);

	resize(idx_stop - idx_start);
	if (idx_start != idx_stop)
		imp_memcpy(v, source.v + idx_start, (idx_stop - idx_start) * sizeof(T));
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::from(const IMP::Vector<T> &source)
{
	from(source, 0, source.count);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::append(const IMP::Vector<T> &source, int idx_start, int idx_stop)
{
	imp_assert(0 <= idx_start && idx_start <= idx_stop && idx_stop <= source.count);

	int old_count = count;
	int to_add = idx_stop - idx_start;
	resize(old_count + to_add);
	if (to_add)
		imp_memcpy(v + old_count, source.v + idx_start, to_add * sizeof(T));
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::append(const IMP::Vector<T> &source)
{
	append(source, 0, source.count);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T& IMP::Vector<T>::at(int pos)
{
	imp_assert(pos >= 0 && pos < count);

	return v[pos];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T& IMP::Vector<T>::front()
{
	imp_assert(count);

	return *v;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T& IMP::Vector<T>::back()
{
	imp_assert(count);

	return v[count - 1];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T* IMP::Vector<T>::begin()
{
	return v;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::q_sort(int (__cdecl *comparison_function)(const void *, const void *))
{
	qsort((void *)v, (size_t)count, sizeof(T), comparison_function);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::q_sort()
{
	q_sort(type_compare_pointer_func<T>);
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline bool IMP::Vector<T>::operator == (const IMP::Vector<T> &other)
{
	int comp;
	if (count != other.count)
		return false;

	if (!count)
		return true;

	comp = imp_memcmp(v, other.v, count * sizeof(T));
	if (!comp)
		return true;
	return false;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline int IMP::Vector<T>::get_position_ordered(const T &el)
{
	int i_l = 0, i_r = count, i_c;
	imp_assert(count);
	if (v[count - 1] < el) return i_r;
	while(i_r - i_l > 1)
	{
		i_c = (i_r + i_l) >> 1;
		if (el < v[i_c]) i_r = i_c; else i_l = i_c;
	};
	return i_l;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T IMP::Vector<T>::sum()
{
	int i;
	T res;
	imp_assert(count);
	res = v[0];
	for (i = 1; i < count; i++) res += v[i];
	return res;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T IMP::Vector<T>::sum(int start_idx, int stop_idx)
{
	int i;
	T res;
	imp_assert(0 <= start_idx && start_idx < stop_idx && stop_idx <= count);
	res = v[start_idx];
	for (i = start_idx + 1; i < stop_idx; i++) res += v[i];
	return res;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T IMP::Vector<T>::norm_l_1()
{
	int i; T res;
	imp_assert(count);
	res = v[0] < 0 ? -v[0] : v[0];
	for (i = 1; i < count; i++) res += (v[i] < 0 ? -v[i] : v[i]);
	return res;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::multiply(const T &val)
{
	for (int i = 0; i < count; i++) v[i] *= val;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T IMP::Vector<T>::min_val(int &pos)
{
	int i;
	T res;
	imp_assert(count);
	res = v[0], pos = 0;
	for (i = 0; i < count; i++)
		if (v[i] < res) res = v[i], pos = i;

	return res;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T IMP::Vector<T>::max_val(int &pos)
{
	int i;
	T res;
	imp_assert(count);
	res = v[0], pos = 0;
	for (i = 0; i < count; i++)
		if (v[i] > res) res = v[i], pos = i;

	return res;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::add(const T &val)
{
	for (int i = 0; i < count; i++) v[i] += val;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::add(const T &val, int start_idx, int stop_idx)
{
	imp_assert(start_idx >= 0 && start_idx < count && stop_idx >= 0 && stop_idx <= count && start_idx <= stop_idx);
	for (int i = start_idx; i < stop_idx; i++) v[i] += val;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::substract(const T &val)
{
	for (int i = 0; i < count; i++) v[i] -= val;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::substract(const IMP::Vector<T> &src)
{
	int i, last_i = count;
	if (last_i > src.count) last_i = src.count;
	for (i = 0; i < last_i; i++) v[i] -= src.v[i];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::sum(const IMP::Vector<T> &src)
{
	int i, last_i = count;
	if (last_i > src.count) last_i = src.count;
	for (i = 0; i < last_i; i++) v[i] += src.v[i];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::sum(const Vector<T> &src, T &val, int limit)
{
	int i;
	imp_assert(count == src.count && limit > 0 && limit <= count);
	
	for (i = 0; i < limit; i++)
		v[i] += val * src.v[i];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::sum(const Vector<T> &src, T &val)
{
	int i;
	imp_assert(count == src.count);
	
	for (i = 0; i < count; i++)
		v[i] += val * src.v[i];
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T IMP::Vector<T>::dotProd(const Vector<T> &src)
{
	int i;
	T res;
	imp_assert(count == src.count && count >= 1);

	res = v[0] * src.v[0];

	for (i = 1; i < count; i++)
		res += v[i] * src.v[i];

	return res;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline T IMP::Vector<T>::dotProd(const Vector<T> &src, int limit)
{
	int i;
	T res;
	imp_assert(count == src.count && count >= 1 && limit > 0 && limit <= count);

	res = v[0] * src.v[0];

	for (i = 1; i < limit; i++)
		res += v[i] * src.v[i];

	return res;
}
//------------------------------------------------------------------------
template <typename T>
__imp_inline void IMP::Vector<T>::sum_circular(const IMP::Vector<T> &src, int offset)
{
	int i = 0, j = offset, last_i;

	if (j + src.count <= count)
	{
		for (; i < src.count; i++, j++)
			v[j] += src.v[i];
	}
	else
	{
		do
		{
			last_i = src.count;
			if (last_i > i + count - j)
				last_i = i + count - j;

			for (; i < last_i; i++, j++)
				v[j] += src.v[i];

			imp_assert(j <= count);
			j = 0;
		}while(i != src.count);
	}
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
#endif//__VECTOR_INL__
