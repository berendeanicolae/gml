//-----------------------------------------------------------------------------
#include "KernelFunctionRBF.h"
//-----------------------------------------------------------------------------
ker_f_rbf::ker_f_rbf()
{
	gamma = 0;
	notif = NULL;
	ObjectName = "ProbVectorMachine_KF";
}
//-----------------------------------------------------------------------------
ker_f_rbf::~ker_f_rbf()
{
	//nothing to do
}
//-----------------------------------------------------------------------------