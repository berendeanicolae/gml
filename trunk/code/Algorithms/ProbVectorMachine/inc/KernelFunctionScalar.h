#ifndef __PVM_KERNEL_FUNCTION_SCALAR_H__
#define __PVM_KERNEL_FUNCTION_SCALAR_H__
//-----------------------------------------------------------------------------
#include "gmllib.h"
#include "PVMDefinesInclude.h"
#include "MathFunctions.h"
//-----------------------------------------------------------------------------
class ker_f_scalar : public GML::Utils::GMLObject
{
public:
	GML::Utils::INotifier *notif;

	ker_f_scalar();
	~ker_f_scalar();

	void compute_for(pvm_float *x, pvm_float *y, int count, pvm_float &res);
};
//-----------------------------------------------------------------------------
pvm_inline void ker_f_scalar::compute_for(pvm_float *x, pvm_float *y, int count, pvm_float &res)
{
	int i;
	res = 0.0;
	DBGSTOP_CHECK((!x || !y));
	for (i = 0; i < count; i++)
		res += x[i] * y[i];
}
//-----------------------------------------------------------------------------
#endif// __PVM_KERNEL_FUNCTION_SCALAR_H__