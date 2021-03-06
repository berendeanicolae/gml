#ifndef __PVM_KERNEL_FUNCTION_RADIAL_BASIS_H__
#define __PVM_KERNEL_FUNCTION_RADIAL_BASIS_H__
//-----------------------------------------------------------------------------
#include "KernelFunction.h"
//-----------------------------------------------------------------------------
class ker_f_rbf: public GML::Utils::GMLObject, public ker_f
{//general form : exp(-gamma * ||X - Y||^2)
public:
	pvm_double gamma;

	GML::Utils::INotifier *notif;

	ker_f_rbf();
	~ker_f_rbf();
	
	pvm_double compute_for(pvm_double *x, pvm_double *y, int count);
	bool set_parameters(pvm_double src_gamma);
};
//-----------------------------------------------------------------------------
pvm_inline pvm_double ker_f_rbf::compute_for(pvm_double *x, pvm_double *y, int count)
{
	int i;
	pvm_double temp;
	pvm_double res = 0.0;
	DBGSTOP_CHECKMSG((x && y), "One of the pointers is NULL");
	for (i = 0; i < count; i++, x++, y++)
	{
#ifdef MISSING_VALUES_ALLOWED
		if (*x <= MISSING_VALUES_BOUND || *y <= MISSING_VALUES_BOUND)
			continue;
#endif//MISSING_VALUES_ALLOWED	
		temp = (*x) - (*y), res += temp * temp;
	}

	return exp(- gamma * res);
}
//-----------------------------------------------------------------------------
pvm_inline bool ker_f_rbf::set_parameters(pvm_double src_gamma)
{
	DBGSTOP_CHECKMSG(src_gamma > 0, "Gamma should be positive");
	if (src_gamma <= 0)
		return false;

	gamma = src_gamma;
	return true;
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_RADIAL_BASIS_H__