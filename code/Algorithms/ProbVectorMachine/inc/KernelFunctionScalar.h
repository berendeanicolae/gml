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
	DBGSTOP_CHECKMSG((x && y), "One of the pointers is NULL");
	for (i = 0; i < count; i++, x++, y++)
	{
#ifdef MISSING_VALUES_ALLOWED
		if (*x <= MISSING_VALUES_BOUND || *y <= MISSING_VALUES_BOUND)
			continue;
#endif//MISSING_VALUES_ALLOWED
		
		res += (*x) * (*y);
	}

	return res;
}
//-----------------------------------------------------------------------------
#endif// __PVM_KERNEL_FUNCTION_SCALAR_H__