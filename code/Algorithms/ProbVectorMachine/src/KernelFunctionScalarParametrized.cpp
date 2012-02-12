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
//-----------------------------------------------------------------------------