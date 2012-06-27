#ifndef __TUPLE_H__
#define __TUPLE_H__
//---------------------------------------------------------------------------
template <typename T, typename U>
class Tuple
{
public:
  T x;
  U y;

  Tuple();
  void operator = (const Tuple<T, U> &source);
	bool operator < (Tuple<T, U> &source);
	bool operator == (const Tuple<T, U> &source);
	bool operator <= (Tuple<T, U> &source);

	void operator += (const Tuple<T, U> &src);
	void operator -= (const Tuple<T, U> &src);

	bool operator > (Tuple<T, U> &source);
};
//---------------------------------------------------------------------------
#endif//__TUPLE_H__
