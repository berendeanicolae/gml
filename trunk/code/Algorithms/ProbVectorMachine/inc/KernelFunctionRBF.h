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
	DBGSTOP_CHECK((!x || !y));
	for (i = 0; i < count; i++, x++, y++)
		temp = (*x) - (*y), res += temp * temp;

	return exp(- gamma * res);
}
//-----------------------------------------------------------------------------
pvm_inline bool ker_f_rbf::set_parameters(pvm_double src_gamma)
{
	DBGSTOP_CHECK(src_gamma <= 0);
	if (src_gamma <= 0)
		return false;

	gamma = src_gamma;
	return true;
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_RADIAL_BASIS_H__