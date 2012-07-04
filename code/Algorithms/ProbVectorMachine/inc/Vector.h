#ifndef __VECTOR_H__
#define __VECTOR_H__
//------------------------------------------------------------------------
/*
	Template pentru vectori
*/
//------------------------------------------------------------------------
namespace IMP
{

template <typename T>
class Vector
{
public:
	T*	v;
	int count;
	int capacity;

	Vector();
	~Vector();

	void push_back(const T& bc);
	void insert(const T& bc, int pos);
	void pop_back();
	void erase(int pos);
	void erase(int start_pos, int stop_pos);

	T& operator [] (int pos);
	bool operator == (const Vector<T> &other);
	Vector<T>& operator = (const Vector<T> &bv);

	T& at(int pos);
	T& front();
	T& back();
	T* begin();

	void assign(T* classic_vector, int number_of_elements);
	void fill_memory_location(T* classic_vector, int no_elem);
	void assign_val(const T &val, int start_idx, int stop_idx);
	void assign_val(const T &val);
	void reserve(int ncap);
	void clear();

	int size();

	void free_mem();

	void resize(int ncount);
	void zero_all();
	void from(const Vector<T> &source, int idx_start, int idx_stop);
	void from(const Vector<T> &source);
	void append(const Vector<T> &source, int idx_start, int idx_stop);
	void append(const Vector<T> &source);

	void q_sort(int (__cdecl *comparison_function) (const void *, const void *));
	void q_sort();

	int get_position_ordered(const T &el);

	//pe functiile de operatii de mai jos ar
	//trebui facuta specializare pentru tipuri numerice
	//pt a face implementare de operatii simultane
	T sum();
	T sum(int start_idx, int stop_idx);
	T min_val(int &pos);
	T max_val(int &pos);
	T dotProd(const Vector<T> &src);
	T dotProd(const Vector<T> &src, int limit);
	void add(const T &val);
	void add(const T &val, int start_idx, int stop_idx);
	void substract(const T &val);
	void substract(const Vector<T> &src);
	void sum(const Vector<T> &src);
	void sum(const Vector<T> &src, T &val);
	void sum(const Vector<T> &src, T &val, int limit);
	void sum_circular(const Vector<T> &src, int offset);
	void multiply(const T &val);
	T norm_l_1();
};
//------------------------------------------------------------------------
};
#endif//__VECTOR_H__
