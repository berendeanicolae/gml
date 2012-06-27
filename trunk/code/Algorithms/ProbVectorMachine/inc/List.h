#ifndef __LIST_H__
#define __LIST_H__
//------------------------------------------------------------------------
template <typename T>
class List
{
public:
	T** v;
	int count;
	int capacity;

	List();
	List(List<T>& source);
	~List();

	T* push_back(T* el);
	T* push_new();
	T* insert(int pos);
	void pop_back();
	void erase(int pos);
	bool erase(T* el);

	void switch_els(int pos_0, int pos_1);

	T* operator [] (int pos);
	bool operator == (const List<T> &src);
	List<T>& operator = (const List<T> &src);

	T*	at(int pos);
	T* front();
	T* back();

	T& at_ref(int pos);

	void reserve(int ncapacity);
	void clear();
	int size();
	void free_mem();
	void resize(int ncount);

	void from(const List<T> &source, int idx_start, int idx_stop);
	void from(const List<T> &source);
	void append(const List<T> &source, int idx_start, int idx_stop);

	void q_sort(int (__cdecl *compare)(const void *el0, const void *el1));
	void q_sort();
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
#endif//__LIST_H__
