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
bool ker_f_rbf_param::set_parameters(pvm_double &src_gamma, GML::Utils::GTVector<pvm_double> &src_params)
{
	int i;

	DBGSTOP_CHECK((src_gamma < 0));
	if (src_gamma < 0)
		return false;

	gamma = src_gamma;

	for (i = 0; i < src_params.GetCount(); i++)
	{
		DBGSTOP_CHECK(src_params[i] < 0)
		if (src_params[i] < 0)
			return false;
	}

	params.Copy(src_params, 0, src_params.GetCount());
	return true;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------