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
	DBGSTOP_CHECK((src_t < 0 || src_d < 1));
	if (src_t < 0 || src_d < 1 || !(src_d % 2))
		return false;

	t = src_t, d = src_d;

	for (i = 0; i < src_params.GetCount(); i++)
	{
		DBGSTOP_CHECK(src_params[i] < 0);
		if (src_params[i] < 0)
			return false;
	}
			

	params.Copy(src_params, 0, src_params.GetCount());
	return true;
}
//-----------------------------------------------------------------------------

