//-----------------------------------------------------------------------------
#include "KernelWrapper.h"
//-----------------------------------------------------------------------------
ker_f_wrapper::ker_f_wrapper()
{
	ObjectName = "ProbVectorMachine_KFW";
}
//-----------------------------------------------------------------------------
ker_f_wrapper::~ker_f_wrapper()
{
//
}
//-----------------------------------------------------------------------------	
void ker_f_wrapper::compute_for_many(pvm_float **xs, pvm_float *results, 
						  int entries_count, int feat_count, KerFuncType kf_type)
{
	int i, j;
	int pos = 0;

	switch(kf_type)
	{
		case KERPOLY: 
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					kf_poly.compute_for(xs[i], xs[j], feat_count, results[pos]); 
			break;

		case KERSCALAR: 
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					kf_scalar.compute_for(xs[i], xs[j], feat_count, results[pos]); 
			break;	

		case KERRBF: 
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					kf_rbf.compute_for(xs[i], xs[j], feat_count, results[pos]); 
			break;
		case KERPOLYPARAM: 
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					kf_poly_param.compute_for(xs[i], xs[j], feat_count, results[pos]); 
			break;
			
		case KERSCALARPARAM:
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					kf_scalar_param.compute_for(xs[i], xs[j], feat_count, results[pos]); 
			break;
			
		case KERRBFPARAM: 
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					kf_rbf_param.compute_for(xs[i], xs[j], feat_count, results[pos]); 
			break;			

		default : memset(results, 0, feat_count * sizeof(pvm_float)); break;
	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------