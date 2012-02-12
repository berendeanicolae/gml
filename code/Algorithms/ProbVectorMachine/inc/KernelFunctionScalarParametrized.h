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
	bool set_parameters(GML::Utils::GTVector<pvm_double> &src_params);
};
//-----------------------------------------------------------------------------
pvm_inline pvm_double ker_f_scalar_param::compute_for(pvm_double *x, pvm_double *y, int count)
{
	int i;
	pvm_double res = 0.0;

	DBGSTOP_CHECKMSG((params.GetCount() == count), "Weights count should equal feature count");
	DBGSTOP_CHECKMSG((x && y), "One of the pointers is NULL");

	for (i = 0; i < count; i++, x++, y++)
		res += (*x) * (*y) * params[i];

	return res;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif//__PVM_KERNEL_FUNCTION_SCALAR_PARAMETRIZED_H__