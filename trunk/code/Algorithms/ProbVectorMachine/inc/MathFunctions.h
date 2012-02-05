#ifndef __PVM_MATH_FUNCTIONS_H__
#define __PVM_MATH_FUNCTIONS_H__
//-----------------------------------------------------------------------------
#include "PVMDefinesInclude.h"
//-----------------------------------------------------------------------------
namespace PVMMathFunctions
{
	pvm_float pow_i(pvm_float x, int p);//in case the power is an integer, 
										//the minimum number of ops is given by it base 2 writing
	pvm_double pow_i(pvm_double x, int p);//in case the power is an integer, 
										//the minimum number of ops is given by it base 2 writing
};
//-----------------------------------------------------------------------------
//---------------------------------INLINES-------------------------------------
//-----------------------------------------------------------------------------
pvm_inline pvm_float PVMMathFunctions::pow_i(pvm_float x, int p)
{
	pvm_float res = 1.0, c_x = x;
	
	while(p)
	{
		if (p & 1) res *= c_x;
		c_x *= c_x;
		p >>=	1;
	};

	return res;
}
//-----------------------------------------------------------------------------
pvm_inline pvm_double PVMMathFunctions::pow_i(pvm_double x, int p)
{
	pvm_double res = 1.0, c_x = x;
	
	while(p)
	{
		if (p & 1) res *= c_x;
		c_x *= c_x;
		p >>=	1;
	};

	return res;
}
//-----------------------------------------------------------------------------
#endif//__PVM_MATH_FUNCTIONS_H__