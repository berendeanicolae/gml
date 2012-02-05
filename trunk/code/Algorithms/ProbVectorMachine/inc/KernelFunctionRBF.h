#ifndef __PVM_KERNEL_FUNCTION_RADIAL_BASIS_H__
#define __PVM_KERNEL_FUNCTION_RADIAL_BASIS_H__
//-----------------------------------------------------------------------------
#include "gmllib.h"
#include "PVMDefinesInclude.h"
#include "MathFunctions.h"
//-----------------------------------------------------------------------------
class ker_f_rbf: public GML::Utils::GMLObject
{//general form : exp(-gamma * ||X - Y||^2)
public:
	pvm_float gamma;

	GML::Utils::INotifier *notif;

	ker_f_rbf();
	~ker_f_rbf();
	
	void compute_for(pvm_float *x, pvm_float *y, int count, pvm_float &res);
	void set_parameters(pvm_float src_gamma);
};
//-----------------------------------------------------------------------------
pvm_inline void ker_f_rbf::compute_for(pvm_float *x, pvm_float *y, int count, pvm_float &res)
{
	int i;
	pvm_float temp;
	res = 0.0;
	DBGSTOP_CHECK((!x || !y));
	for (i = 0; i < count; i++)
		temp = x[i] - y[i], res += temp * temp;
	res = exp(- gamma * res);
}
//-----------------------------------------------------------------------------
pvm_inline void ker_f_rbf::set_parameters(pvm_float src_gamma)
{
	DBGSTOP_CHECK(src_gamma <= 0);
	gamma = src_gamma;
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_RADIAL_BASIS_H__