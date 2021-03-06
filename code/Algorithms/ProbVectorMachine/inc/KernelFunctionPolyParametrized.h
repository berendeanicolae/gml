#ifndef __PVM_KERNEL_FUNCTION_POLY_PARAMETRIZED_H__
#define __PVM_KERNEL_FUNCTION_POLY_PARAMETRIZED_H__
//-----------------------------------------------------------------------------
#include "KernelFunction.h"
//-----------------------------------------------------------------------------
class ker_f_poly_param : public GML::Utils::GMLObject, public ker_f
{
public:
	pvm_double	t;
	int			d;
	GML::Utils::GTVector<pvm_double> params;
	double		params_start;

	GML::Utils::INotifier *notif;

	ker_f_poly_param();
	~ker_f_poly_param();

	pvm_double compute_for(pvm_double *x, pvm_double *y, int count);
	bool set_parameters(pvm_double &src_t, int &src_d, 
						GML::Utils::GTVector<pvm_double> &src_params);
};
//-----------------------------------------------------------------------------
//----------------------------------INLINES------------------------------------
//-----------------------------------------------------------------------------
pvm_inline pvm_double ker_f_poly_param::compute_for(pvm_double *x, pvm_double *y, int count)
{
	int i;
	pvm_double res = t;

	DBGSTOP_CHECKMSG((count == (int)params.GetCount()), "Weight count should equal feature count");
	DBGSTOP_CHECKMSG((x && y), "One of the pointers is NULL");
	for (i = 0; i < count; i++, x++, y++)	
	{
#ifdef MISSING_VALUES_ALLOWED
		if (*x <= MISSING_VALUES_BOUND || *y <= MISSING_VALUES_BOUND)
			continue;
#endif//MISSING_VALUES_ALLOWED	
		res += (*x) * (*y) * params[i];
	}
	
	return PVMMathFunctions::pow_i(res, d);;
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_POLY_PARAMETRIZED_H__