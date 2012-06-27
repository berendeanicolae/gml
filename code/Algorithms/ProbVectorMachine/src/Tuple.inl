#ifndef __TUPLE_INL__
#define __TUPLE_INL__
//---------------------------------------------------------------------------
#include "Tuple.h"
//---------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline void Tuple<T, U>::operator = (const Tuple<T, U> &source)
{
  x = source.x, y = source.y;
}
//---------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline Tuple<T, U>::Tuple()
{
  memset(this, 0, sizeof(Tuple<T, U>));
}
//---------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline bool Tuple<T, U>::operator < (Tuple<T, U> &source)
{
	if (x < source.x)
		return true;
	else if (x > source.x)
		return false;

	return y < source.y;
}
//---------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline bool Tuple<T, U>::operator == (const Tuple<T, U> &source)
{
	return x == source.x && y == source.y;
}
//---------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline bool Tuple<T, U>::operator <= (Tuple<T, U> &source)
{
	return !(source.operator < (*this));
}
//---------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline void Tuple<T, U>::operator += (const Tuple<T, U> &src)
{
	x += src.x, y += src.y;
}
//---------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline void Tuple<T, U>::operator -= (const Tuple<T, U> &src)
{
	x -= src.x, y -= src.y;
}
//---------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline bool Tuple<T, U>::operator > (Tuple<T, U> &source)
{
	if (x > source.x)
		return true;
	else if (source.x > x)
		return false;

	return y > source.y;
}
//---------------------------------------------------------------------------
#endif//__TUPLE_INL__
