#ifndef __IMAGE_PROCESSING_TEMPLATE_STRUCTURES_H__
#define __IMAGE_PROCESSING_TEMPLATE_STRUCTURES_H__
//------------------------------------------------------------------------
#include "List.h"
#include "Vector.h"
#include "Tuple.h"
#include "BoundingBox.h"
#include <vector>
//------------------------------------------------------------------------
typedef IMP::Vector<int>									int_vector_t;
typedef IMP::Vector<int_vector_t *>				int_vector_ref_vector_t;
typedef IMP::Vector<double>								double_vector_t;
typedef IMP::Vector<bool>									bool_vector_t;
typedef IMP::Vector<__int64>							int64_vector_t;

typedef List<int_vector_t>						int_matrix_t;
typedef List<int_matrix_t>						int_matrix_list_t;
typedef List<double_vector_t>					double_matrix_t;
typedef List<double_matrix_t>					double_matrix_list_t;
typedef List<bool_vector_t>						bool_matrix_t;
typedef List<int64_vector_t>					int64_matrix_t;

typedef List<int_vector_t>						int_vector_ref_list_t;
typedef List<double_vector_t>					double_vector_ref_list_t;

typedef Tuple<double_vector_t, int>		point_data_t;
typedef List<point_data_t>						point_data_list_t;

typedef Tuple<int, int>								point_i32_t;
typedef Tuple<__int64, __int64>				point_i64_t;

typedef BoundingBox<int, int>					bb_pt_i32_t;
typedef Tuple<double, bb_pt_i32_t>		level_bb_pt_i32_t;
typedef List<level_bb_pt_i32_t>				level_bb_pt_i32_list_t;

typedef Tuple<double, point_i32_t>		level_point_i32_t;
typedef List<level_point_i32_t>				level_point_i32_list_t;
typedef IMP::Vector<level_point_i32_t>			level_point_i32_vector_t;

typedef IMP::Vector<point_i32_t>						point_i32_vector_t;
typedef List<point_i32_vector_t>			point_i32_vector_list_t;

typedef Tuple<double, int>						double_idx_t;
typedef IMP::Vector<double_idx_t>					double_idx_vector_t;

typedef Tuple<double, double>					point_double_t;
typedef IMP::Vector<point_double_t>				point_double_vector_t;

typedef Tuple<double_matrix_t, double_matrix_t>		gradient_matrix_t;
typedef List<gradient_matrix_t>										gradient_matrix_list_t;
//------------------------------------------------------------------------
//------------------------------------------------------------------------
#endif//__IMAGE_PROCESSING_TEMPLATE_STRUCTURES_H__
