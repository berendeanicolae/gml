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
void ker_f_scalar_param::set_parameters(GML::Utils::GTVector<pvm_double> &src_params)
{
	params.Copy(src_params, 0, src_params.GetCount());
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------