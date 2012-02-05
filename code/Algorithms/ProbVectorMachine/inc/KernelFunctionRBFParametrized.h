#ifndef __PVM_KERNEL_FUNCTION_RBF_PARAMETRIZED_H__
#define __PVM_KERNEL_FUNCTION_RBF_PARAMETRIZED_H__
//-----------------------------------------------------------------------------
#include "gmllib.h"
#include "PVMDefinesInclude.h"
#include "MathFunctions.h"
//-----------------------------------------------------------------------------
class ker_f_rbf_param : public GML::Utils::GMLObject
{
	pvm_float gamma;

	GML::Utils::GTVector<pvm_float> params;
	GML::Utils::INotifier *notif;

	ker_f_rbf_param();
	~ker_f_rbf_param();

	void compute_for(pvm_float *x, pvm_float *y, int count, pvm_float &res);
	void set_parameters(pvm_float &src_gamma, GML::Utils::GTVector<pvm_float> &src_params);
};
//-----------------------------------------------------------------------------
//--------------------------INLINES--------------------------------------------
//-----------------------------------------------------------------------------
pvm_inline void ker_f_rbf_param::compute_for(pvm_float *x, pvm_float *y, int count, pvm_float &res)
{
	int i;
	pvm_float temp;
	res = 0.0;

	DBGSTOP_CHECK((params.GetCount() != count));
	DBGSTOP_CHECK((!x || !y));
												 
	for (i = 0; i < count; i++)
		temp = x[i] - y[i], res += temp * temp * params[i];

	res = exp(- gamma * res * res);
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_RBF_PARAMETRIZED_H__