#ifndef __BOUNDING_BOX_H__
#define __BOUNDING_BOX_H__
//------------------------------------------------------------------------
#include "Tuple.h"
//------------------------------------------------------------------------
#define __BB_SIDE_IN -1
#define __BB_SIDE_ON	0
#define __BB_SIDE_OUT 1
//------------------------------------------------------------------------
template <typename T, typename U>
class BoundingBox
{
public:
	Tuple<T, U> l_cr, u_cr;

	BoundingBox(){};
	BoundingBox(const BoundingBox<T, U> &src){from(src);};

  void operator = (const BoundingBox<T, U> &src);
	bool operator < (BoundingBox<T, U> &src);
	bool operator == (const BoundingBox<T, U> &src);
	int point_inside_simple(Tuple<T, U> &point);
	int point_inside_accurate(Tuple<T, U> &point);

	void from(const BoundingBox<T, U> &src);

	void include(const Tuple<T, U> &point);
	void include(const BoundingBox<T, U> &src);

	void init(const Tuple<T, U> &point);

	void clip_with(const BoundingBox<T, U> &src);

	bool intersects(const BoundingBox<T, U> &src);
};
//------------------------------------------------------------------------
#endif//__BOUNDING_BOX_H__
