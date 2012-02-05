#ifndef __PVM_KERNEL_WRAPPER_H__
#define __PVM_KERNEL_WRAPPER_H__
//-----------------------------------------------------------------------------
#include "KernelFunctionInclude.h"
//-----------------------------------------------------------------------------
typedef enum KerFuncType {KERPOLY, KERSCALAR, KERRBF, KERPOLYPARAM, KERSCALARPARAM, KERRBFPARAM};
//-----------------------------------------------------------------------------
class ker_f_wrapper : public GML::Utils::GMLObject
{
public:
	ker_f_scalar		kf_scalar;
	ker_f_poly			kf_poly;
	ker_f_rbf			kf_rbf;
		
	ker_f_scalar_param	kf_scalar_param;
	ker_f_poly_param	kf_poly_param;
	ker_f_rbf_param		kf_rbf_param;

	ker_f				*kf;

	KerFuncType			kf_type;

	

	ker_f_wrapper();
	~ker_f_wrapper();

	double compute_for(pvm_double *x, pvm_double *y, int count);
	double compute_for(GML::ML::MLRecord &ml_rec0, GML::ML::MLRecord &ml_rec1);

	void compute_for_many(pvm_double **xs, pvm_double *results, //the ker result for (xs[i], xs[j]) will be found at position = (i * entries_count + j)
						  int entries_count, int feat_count, KerFuncType kf_type);

	//because each kernel type has at most one float, one int and the vector coresponding to weights
	//we use this less transparent form of parameter setting
	void set_params(pvm_double src_fl0, int src_i0,  GML::Utils::GTVector<pvm_double> *src_weights, KerFuncType kf_type);
	void set_ker_type(KerFuncType src_kf_type);
};
//-----------------------------------------------------------------------------
//--------------------------INLINES--------------------------------------------
//-----------------------------------------------------------------------------
pvm_inline double ker_f_wrapper::compute_for(pvm_double *x, pvm_double *y, int count)
{
	DBGSTOP_CHECK(!kf);
	return kf->compute_for(x, y, count);
}
//-----------------------------------------------------------------------------
pvm_inline double ker_f_wrapper::compute_for(GML::ML::MLRecord &ml_rec0, GML::ML::MLRecord &ml_rec1)
{
	DBGSTOP_CHECK(ml_rec0.FeatCount != ml_rec1.FeatCount);	
	return compute_for(ml_rec0.Features, ml_rec1.Features, ml_rec1.FeatCount);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif// __PVM_KERNEL_WRAPPER_H__


