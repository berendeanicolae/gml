#ifndef __PVM_KERNEL_FUNCTION_POLY_H__
#define __PVM_KERNEL_FUNCTION_POLY_H__
//-----------------------------------------------------------------------------
#include "gmllib.h"
#include "PVMDefinesInclude.h"
#include "MathFunctions.h"
//-----------------------------------------------------------------------------
typedef struct ker_f_poly
{
	//general form : K(X, Y) = (<X, Y> + t)^d, where 
	//<*, *> is the scalar product, 
	//"t" is the intercept 
	//"d" is the degree and must be an odd positive integer
	pvm_float							t;
	int										d;
	GML::Utils::INotifier *notif;

	ker_f_poly();
	~ker_f_poly();
	
	void compute_for(pvm_float *x, pvm_float *y, int count, pvm_float &res);
	void set_parameters(pvm_float &src_t, int &src_d);

}ker_f_poly;
//-----------------------------------------------------------------------------
//---------------------INLINES-------------------------------------------------
//-----------------------------------------------------------------------------
pvm_inline void ker_f_poly::compute_for(pvm_float *x, pvm_float *y, int count, pvm_float &res)
{
	int i;
	res = t;
	for (i = 0; i < count; i++) 
		res += x[i] * y[i];

	res = PVMMathFunctions::pow_i(res, d);
}
//-----------------------------------------------------------------------------
pvm_inline void ker_f_poly::set_parameters(pvm_float &src_t, int &src_d)
{
	t = src_t, d = src_d;
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_POLY_H__