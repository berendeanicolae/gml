//-----------------------------------------------------------------------------
#include "KernelFunctionRBFParametrized.h"
//-----------------------------------------------------------------------------
ker_f_rbf_param::ker_f_rbf_param()
{
	gamma = 0.0;
	notif = NULL;
	ObjectName = "ProbVectorMachine_KF";
}
//-----------------------------------------------------------------------------
ker_f_rbf_param::~ker_f_rbf_param()
{
	//nothing to do
}
//-----------------------------------------------------------------------------
void ker_f_rbf_param::set_parameters(pvm_float &src_gamma, GML::Utils::GTVector<pvm_float> &src_params)
{
	DBGSTOP_CHECK((src_gamma < 0));
	gamma = src_gamma;
	params.Copy(src_params, 0, src_params.GetCount());
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------