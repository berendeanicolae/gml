//-----------------------------------------------------------------------------
#include "KernelWrapper.h"
//-----------------------------------------------------------------------------
ker_f_wrapper::ker_f_wrapper()
{
	kf = NULL;
	ObjectName = "ProbVectorMachine_KFW";
}
//-----------------------------------------------------------------------------
ker_f_wrapper::~ker_f_wrapper()
{
//
}
//-----------------------------------------------------------------------------	
void ker_f_wrapper::compute_for_many(pvm_double **xs, pvm_double *results, 
						  int entries_count, int feat_count, KerFuncType kf_type)
{
	int i, j;
	int pos = 0;

	switch(kf_type)
	{
		case KERPOLY: 
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					results[pos] = kf_poly.compute_for(xs[i], xs[j], feat_count); 
			break;

		case KERSCALAR: 
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					results[pos] = kf_scalar.compute_for(xs[i], xs[j], feat_count); 
			break;	

		case KERRBF: 
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					results[pos] = kf_rbf.compute_for(xs[i], xs[j], feat_count); 
			break;
		case KERPOLYPARAM: 
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					results[pos] = kf_poly_param.compute_for(xs[i], xs[j], feat_count); 
			break;
			
		case KERSCALARPARAM:
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					results[pos] = kf_scalar_param.compute_for(xs[i], xs[j], feat_count); 
			break;
			
		case KERRBFPARAM: 
			for (i = 0; i < entries_count; i++)
				for (j = i; j < entries_count; j++, pos++)
					results[pos] = kf_rbf_param.compute_for(xs[i], xs[j], feat_count); 
			break;			

		default : memset(results, 0, feat_count * sizeof(pvm_double)); break;
	}
}
//-----------------------------------------------------------------------------
bool ker_f_wrapper::set_params(pvm_double src_fl0, int src_i0,  GML::Utils::GTVector<pvm_double> *src_weights, KerFuncType kf_type)
{
	bool ret;
	switch(kf_type)	
	{
		case KERPOLY: 			
			ret = kf_poly.set_parameters(src_fl0, src_i0); break;
		case KERSCALAR: 
			ret =  true; break;
		case KERRBF: 
			ret = kf_rbf.set_parameters(src_fl0); break;
		case KERPOLYPARAM: 
			DBGSTOP_CHECK(!src_weights);
			ret = kf_poly_param.set_parameters(src_fl0, src_i0, *src_weights); break;
		case KERSCALARPARAM: 
			DBGSTOP_CHECK(!src_weights);
			ret = kf_scalar_param.set_parameters(*src_weights); break;
		case KERRBFPARAM: 
			DBGSTOP_CHECK(!src_weights);
			ret = kf_rbf_param.set_parameters(src_fl0, *src_weights); break;

		default : ret = false;
	}

	if (ret)
		set_ker_type(kf_type);

	return ret;
}
//-----------------------------------------------------------------------------
void ker_f_wrapper::set_ker_type(KerFuncType src_kf_type)
{
	kf_type = src_kf_type;
	switch(kf_type)
	{
		case KERPOLY: kf = &kf_poly; break;
		case KERSCALAR: kf = &kf_scalar; break;
		case KERRBF: kf = &kf_rbf; break;
		case KERPOLYPARAM: kf = &kf_poly_param; break;
		case KERSCALARPARAM: kf = &kf_scalar_param; break;
		case KERRBFPARAM: kf = &kf_rbf_param; break;
	  
		default : kf = NULL; break;	
	}
}
//-----------------------------------------------------------------------------