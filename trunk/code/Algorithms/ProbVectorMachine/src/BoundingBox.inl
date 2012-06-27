#ifndef __BOUNDING_BOX_INL__
#define __BOUNDING_BOX_INL__
//------------------------------------------------------------------------
#include "BoundingBox.h"
#include "Tuple.inl"
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline void BoundingBox<T, U>::operator = (const BoundingBox<T, U> &src)
{
	from(src);
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline bool BoundingBox<T, U>::operator < (BoundingBox<T, U> &src)
{
	if (l_cr < src.l_cr) return true;
	if (src.l_cr < l_cr) return false;
	return u_cr < src.u_cr;
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline bool BoundingBox<T, U>::operator == (const BoundingBox<T, U> &src)
{
	return l_cr == src.l_cr && u_cr == src.u_cr;
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline int BoundingBox<T, U>::point_inside_simple(Tuple<T, U> &point)
{
	if (l_cr.x <= point.x && 
			l_cr.y <= point.y &&
			point.x <= u_cr.x &&
			point.y <= u_cr.y) 
		return __BB_SIDE_IN;
	return __BB_SIDE_OUT;
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline int BoundingBox<T, U>::point_inside_accurate(Tuple<T, U> &point)
{
	if (point_inside_simple(point) == __BB_SIDE_OUT) return __BB_SIDE_OUT;
	if (point.x == l_cr.x || point.x == u_cr.x ||
			point.y == l_cr.y || point.y == u_cr.y)
		return __BB_SIDE_ON;
	return __BB_SIDE_IN;
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline void BoundingBox<T, U>::from(const BoundingBox<T, U> &src)
{
	l_cr = src.l_cr; u_cr = src.u_cr;
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline void BoundingBox<T, U>::include(const Tuple<T, U> &point)
{
	if (point.x < l_cr.x) l_cr.x = point.x;
	if (point.y < l_cr.y) l_cr.y = point.y;
	if (u_cr.x < point.x) u_cr.x = point.x;
	if (u_cr.y < point.y) u_cr.y = point.y;
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline void BoundingBox<T, U>::include(const BoundingBox<T, U> &src)
{
	include(src.l_cr); include(src.u_cr);
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline void BoundingBox<T, U>::init(const Tuple<T, U> &point)
{
	l_cr = point; u_cr = point;
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline void BoundingBox<T, U>::clip_with(const BoundingBox<T, U> &src)
{
	if (l_cr.x < src.l_cr.x) l_cr.x = src.l_cr.x;
	if (l_cr.y < src.l_cr.y) l_cr.y = src.l_cr.y;
	if (u_cr.x > src.u_cr.x) u_cr.x = src.u_cr.x;
	if (u_cr.y > src.u_cr.y) u_cr.y = src.u_cr.y;
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline bool BoundingBox<T, U>::intersects(const BoundingBox<T, U> &src)
{
	if ((l_cr.x > src.u_cr.x || src.l_cr.x > u_cr.x) ||
			(l_cr.y > src.u_cr.y || src.l_cr.y > u_cr.y))
		return false;

	return true;
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
#endif//__BOUNDING_BOX_INL__
