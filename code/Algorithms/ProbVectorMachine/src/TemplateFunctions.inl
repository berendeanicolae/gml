#ifndef __IMAGE_PROCESSING_TEMPLATE_FUNCTIONS_INL__
#define __IMAGE_PROCESSING_TEMPLATE_FUNCTIONS_INL__
//---------------------------------------------------------------------------
#include "TemplateFunctions.h"
#include "TemplateStructures.inl"
//---------------------------------------------------------------------------
template <typename T>
__imp_inline int type_compare_func(T &e1, T &e2)
{
	if (e1 < e2)
		return -1;
	else if (e2 < e1)
		return 1;
	return 0;
}
//---------------------------------------------------------------------------
template <typename T>
__imp_inline int type_compare_pointer_func(const void *e1, const void *e2)
{
	return type_compare_func<T>(*((T *)e1), *((T *)e2));
}
//---------------------------------------------------------------------------
template <typename T>
__imp_inline int type_compare_pointer_dual_func(const void *e1, const void *e2)
{
	return type_compare_func<T>(**((T **)e1), **((T **)e2));
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline int type_compare_tuple_norm_sqr(Tuple<T, U> &e1, Tuple<T, U> &e2)
{
	double norm1 = (double)e1.x * (double)e1.x + (double)e1.y * (double)e1.y;
	double norm2 = (double)e2.x * (double)e2.x + (double)e2.y * (double)e2.y;

	if (norm1 < norm2)
		return -1;
	else if (norm2 < norm1)
		return 1;
	return 0;
}
//------------------------------------------------------------------------
template <typename T, typename U>
__imp_inline int type_compare_tuple_pointer_norm_sqr_func(const void *e1, const void *e2)
{
	return type_compare_tuple_norm_sqr<T, U>(*((Tuple<T, U> *)e1), *((Tuple<T, U> *)e2));
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif//__IMAGE_PROCESSING_TEMPLATE_FUNCTIONS_INL__
