#ifndef __PVM_KERNEL_FUNCTION_POLY_PARAMETRIZED_H__
#define __PVM_KERNEL_FUNCTION_POLY_PARAMETRIZED_H__
//-----------------------------------------------------------------------------
#include "gmllib.h"
#include "PVMDefinesInclude.h"
#include "MathFunctions.h"
//-----------------------------------------------------------------------------
class ker_f_poly_param : public GML::Utils::GMLObject
{
public:
	pvm_float	t;
	int			d;
	GML::Utils::GTVector<pvm_float> params;

	GML::Utils::INotifier *notif;

	ker_f_poly_param();
	~ker_f_poly_param();

	void compute_for(pvm_float *x, pvm_float *y, int count, pvm_float &res);
	void set_parameters(pvm_float &src_t, int &src_d, 
						GML::Utils::GTVector<pvm_float> &src_params);
};
//-----------------------------------------------------------------------------
//----------------------------------INLINES------------------------------------
//-----------------------------------------------------------------------------
pvm_inline void ker_f_poly_param::compute_for(pvm_float *x, pvm_float *y, int count, pvm_float &res)
{
	int i;
	res = t;
	DBGSTOP_CHECK((count != params.GetCount()));
	DBGSTOP_CHECK((!x || !y));
	for (i = 0; i < count; i++)	
		res += x[i] * y[i] * params[i];

	res = PVMMathFunctions::pow_i(res, d);
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_POLY_PARAMETRIZED_H__