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
	virtual pvm_double compute_for(pvm_double *x, pvm_double *y, int count) = 0;
};
//-----------------------------------------------------------------------------
#endif//__PVM_KERNEL_FUNCTION_H__