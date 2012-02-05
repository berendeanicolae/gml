#ifndef __PVM_KERNEL_FUNCTION_RBF_PARAMETRIZED_H__
#define __PVM_KERNEL_FUNCTION_RBF_PARAMETRIZED_H__
//-----------------------------------------------------------------------------
#include "KernelFunction.h"
//-----------------------------------------------------------------------------
class ker_f_rbf_param : public GML::Utils::GMLObject, public ker_f
{
public:
	pvm_double gamma;

	GML::Utils::GTVector<pvm_double> params;
	GML::Utils::INotifier *notif;

	ker_f_rbf_param();
	~ker_f_rbf_param();

	pvm_double compute_for(pvm_double *x, pvm_double *y, int count);
	void set_parameters(pvm_double &src_gamma, GML::Utils::GTVector<pvm_double> &src_params);
};
//-----------------------------------------------------------------------------
//--------------------------INLINES--------------------------------------------
//-----------------------------------------------------------------------------
pvm_inline pvm_double ker_f_rbf_param::compute_for(pvm_double *x, pvm_double *y, int count)
{
	int i;
	pvm_double temp;
	pvm_double res = 0.0;

	DBGSTOP_CHECK((params.GetCount() != count));
	DBGSTOP_CHECK((!x || !y));
												 
	for (i = 0; i < count; i++)
		temp = x[i] - y[i], res += temp * temp * params[i];

	return exp(- gamma * res * res);
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_RBF_PARAMETRIZED_H__