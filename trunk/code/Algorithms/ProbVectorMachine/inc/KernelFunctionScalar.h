#ifndef __PVM_KERNEL_FUNCTION_SCALAR_H__
#define __PVM_KERNEL_FUNCTION_SCALAR_H__
//-----------------------------------------------------------------------------
#include "KernelFunction.h"
//-----------------------------------------------------------------------------
class ker_f_scalar : public GML::Utils::GMLObject, public ker_f
{
public:
	GML::Utils::INotifier *notif;

	ker_f_scalar();
	~ker_f_scalar();

	pvm_double compute_for(pvm_double *x, pvm_double *y, int count);
};
//-----------------------------------------------------------------------------
pvm_inline pvm_double ker_f_scalar::compute_for(pvm_double *x, pvm_double *y, int count)
{
	int i;
	pvm_double res = 0.0;
	DBGSTOP_CHECK((!x || !y));
	for (i = 0; i < count; i++, x++, y++)
		res += (*x) * (*y);

	return res;
}
//-----------------------------------------------------------------------------
#endif// __PVM_KERNEL_FUNCTION_SCALAR_H__