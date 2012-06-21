#ifndef __PVM_KERNEL_FUNCTION_H__
#define __PVM_KERNEL_FUNCTION_H__
//-----------------------------------------------------------------------------
#include "gmllib.h"
#include "PVMDefinesInclude.h"
#include "MathFunctions.h"
//-----------------------------------------------------------------------------
class ker_f
{
public:
	GML::ML::IConnector*	con;
	GML::Utils::INotifier*	notif;

	virtual pvm_double compute_for(pvm_double *x, pvm_double *y, int count) = 0;
	bool set_inherit_data(GML::ML::IConnector *src_con, GML::Utils::INotifier *src_notif);
};
//-----------------------------------------------------------------------------
pvm_inline bool ker_f::set_inherit_data(GML::ML::IConnector *src_con, GML::Utils::INotifier *src_notif)
{
	con = src_con, notif = src_notif;
	return true;
}
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_H__