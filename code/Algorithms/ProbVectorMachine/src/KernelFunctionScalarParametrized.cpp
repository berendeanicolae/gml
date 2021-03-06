//-----------------------------------------------------------------------------
#include "KernelFunctionScalarParametrized.h"
//-----------------------------------------------------------------------------
ker_f_scalar_param::ker_f_scalar_param()
{
	notif = NULL;
	ObjectName = "ProbVectorMachine_KF";
}
//-----------------------------------------------------------------------------
ker_f_scalar_param::~ker_f_scalar_param()
{
//
}
//-----------------------------------------------------------------------------
bool ker_f_scalar_param::set_parameters(GML::Utils::GTVector<pvm_double> &src_params)
{
	int i;
	
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
//-----------------------------------------------------------------------------