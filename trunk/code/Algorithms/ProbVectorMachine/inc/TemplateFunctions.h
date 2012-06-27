#ifndef __IMAGE_PROCESSING_TEMPLATE_FUNCTIONS_H__
#define __IMAGE_PROCESSING_TEMPLATE_FUNCTIONS_H__
//---------------------------------------------------------------------------
#include "TemplateStructures.h"
//---------------------------------------------------------------------------
template <typename T>
int type_compare_func(T &e1, T &e2);
//---------------------------------------------------------------------------
template <typename T>
int type_compare_pointer_func(const void *e1, const void *e2);
//---------------------------------------------------------------------------
template <typename T>
int type_compare_pointer_dual_func(const void *e1, const void *e2);
//---------------------------------------------------------------------------
template <typename T, typename U>
int type_compare_tuple_norm_sqr(Tuple<T, U> &e1, Tuple<T, U> &e2);
//---------------------------------------------------------------------------
template <typename T, typename U>
int type_compare_tuple_pointer_norm_sqr_func(const void *e1, const void *e2);
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/*typedef type_compare_func<int> int_compare_func;
typedef type_compare_func<double> double_compare_func;
typedef type_compare_func<__int64> int64_compare_func;
typedef type_compare_func<point_i32_t> point_i32_compare_func;
typedef type_compare_pointer_func<point_i32_t> point_i32_compare_pointer_func;*/
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif//__IMAGE_PROCESSING_TEMPLATE_FUNCTIONS_H__
