#ifndef __PVM_DEFINES_CONSTANTS_H__
#define __PVM_DEFINES_CONSTANTS_H__
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define MISSING_VALUES_ALLOWED

#ifdef MISSING_VALUES_ALLOWED
#define MISSING_VALUES_BOUND		-98//if a normalized value is under this threshold, it will be considered missing
#endif//MISSING_VALUES_ALLOWED
//-----------------------------------------------------------------------------
#define PVM_EPS			1e-06
//-----------------------------------------------------------------------------
#define EXACT_SCORE_ONLY
//#define LOCAL_OPERATOR_AVERAGE
#define USE_RECORD_WEIGHTS
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif//__PVM_DEFINES_CONSTANTS_H__

