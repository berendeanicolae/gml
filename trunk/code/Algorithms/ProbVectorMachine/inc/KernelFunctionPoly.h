#ifndef __PVM_KERNEL_FUNCTION_POLY_H__
#define __PVM_KERNEL_FUNCTION_POLY_H__
//-----------------------------------------------------------------------------
#include "KernelFunction.h"
//-----------------------------------------------------------------------------
class ker_f_poly: public GML::Utils::GMLObject, public ker_f
{
public:
	//general form : K(X, Y) = (<X, Y> + t)^d, where 
	//<*, *> is the normal scalar product, 
	//"t" is the intercept 
	//"d" is the degree and must be an odd positive integer
	pvm_double				t;
	int						d;
	GML::Utils::INotifier *notif;

	ker_f_poly();
	~ker_f_poly();
	
	pvm_double compute_for(pvm_double *x, pvm_double *y, int count);
	bool set_parameters(pvm_double src_t, int src_d);
};
//-----------------------------------------------------------------------------
//---------------------INLINES-------------------------------------------------
//-----------------------------------------------------------------------------
pvm_inline pvm_double ker_f_poly::compute_for(pvm_double *x, pvm_double *y, int count)
{
	int i;
	pvm_double res = t;
	DBGSTOP_CHECKMSG((x && y), "One of the pointers is NULL");
	for (i = 0; i < count; i++, x++, y++) 
	{
#ifdef MISSING_VALUES_ALLOWED
		if (*x <= MISSING_VALUES_BOUND || *y <= MISSING_VALUES_BOUND)
			continue;
#endif//MISSING_VALUES_ALLOWED	
		res += (*x) * (*y);
	}
	
	return PVMMathFunctions::pow_i(res, d);;
}
//-----------------------------------------------------------------------------
pvm_inline bool ker_f_poly::set_parameters(pvm_double src_t, int src_d)
{
	DBGSTOP_CHECKMSG((src_t >= 0 && src_d >= 1 && (src_d % 2)), "Bad params sent to ker function");
	if (src_t < 0 || src_d < 1 || !(src_d % 2))
		return false;
		
	t = src_t, d = src_d;
	return true;
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_POLY_H__