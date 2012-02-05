#ifndef __PVM_KERNEL_FUNCTION_SCALAR_PARAMETRIZED_H__
#define __PVM_KERNEL_FUNCTION_SCALAR_PARAMETRIZED_H__
//-----------------------------------------------------------------------------
#include "KernelFunction.h"
//-----------------------------------------------------------------------------
class ker_f_scalar_param : public GML::Utils::GMLObject, public ker_f
{
public:
	GML::Utils::GTVector<pvm_double> params;
	GML::Utils::INotifier *notif;

	ker_f_scalar_param();
	~ker_f_scalar_param();

	pvm_double compute_for(pvm_double *x, pvm_double *y, int count);
	void set_parameters(GML::Utils::GTVector<pvm_double> &src_params);
};
//-----------------------------------------------------------------------------
pvm_inline pvm_double ker_f_scalar_param::compute_for(pvm_double *x, pvm_double *y, int count)
{
	int i;
	pvm_double res = 0.0;

	DBGSTOP_CHECK((params.GetCount() != count));
	DBGSTOP_CHECK((!x || !y));

	for (i = 0; i < count; i++)
		res += x[i] * y[i] * params[i];

	return res;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif//__PVM_KERNEL_FUNCTION_SCALAR_PARAMETRIZED_H__