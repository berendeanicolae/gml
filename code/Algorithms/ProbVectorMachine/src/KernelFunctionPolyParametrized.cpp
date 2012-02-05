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
void ker_f_poly_param::set_parameters(pvm_double &src_t, int &src_d, 
											GML::Utils::GTVector<pvm_double> &src_params)
{
	DBGSTOP_CHECK((src_t < 0 || src_d < 1));
	t = src_t, d = src_d;
	params.Copy(src_params, 0, src_params.GetCount());
}
//-----------------------------------------------------------------------------

