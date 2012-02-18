//-----------------------------------------------------------------------------
#include "KernelFunctionPolyParametrized.h"
//-----------------------------------------------------------------------------
ker_f_poly_param::ker_f_poly_param()
{
	t = 0.0;
	d = 0;
	notif = NULL;
	ObjectName = "ProbVectorMachine_KF";
}
//-----------------------------------------------------------------------------
ker_f_poly_param::~ker_f_poly_param()
{
	params.DeleteAll();
}
//-----------------------------------------------------------------------------
bool ker_f_poly_param::set_parameters(pvm_double &src_t, int &src_d, 
											GML::Utils::GTVector<pvm_double> &src_params)
{
	int i;
	DBGSTOP_CHECKMSG((src_t >= 0 || src_d >= 1 && (src_d % 2)), "Bad params sent to ker func");
	if (src_t < 0 || src_d < 1 || !(src_d % 2))
		return false;

	t = src_t, d = src_d;

	for (i = 0; i < (int)src_params.GetCount(); i++)
	{
		DBGSTOP_CHECKMSG(src_params[i] >= 0, "Weights should be positive");
		if (src_params[i] < 0)
			return false;
	}
			

	params.Copy(src_params, 0, (int)src_params.GetCount());
	return true;
}
//-----------------------------------------------------------------------------

