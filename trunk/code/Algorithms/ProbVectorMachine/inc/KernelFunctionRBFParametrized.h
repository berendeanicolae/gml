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
	bool set_parameters(pvm_double &src_gamma, GML::Utils::GTVector<pvm_double> &src_params);
};
//-----------------------------------------------------------------------------
//--------------------------INLINES--------------------------------------------
//-----------------------------------------------------------------------------
pvm_inline pvm_double ker_f_rbf_param::compute_for(pvm_double *x, pvm_double *y, int count)
{
	int i;
	pvm_double temp;
	pvm_double res = 0.0;

	DBGSTOP_CHECKMSG((params.GetCount() == count), "Weights count should equal feature count");
	DBGSTOP_CHECKMSG((x && y), "One of the pointers is NULL");
												 
	for (i = 0; i < count; i++, x++, y++)
		temp = (*x) - (*y), res += temp * temp * params[i];

	return exp(- gamma * res * res);
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_RBF_PARAMETRIZED_H__