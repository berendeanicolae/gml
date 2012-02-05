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
	

	ker_f_wrapper();
	~ker_f_wrapper();

	void compute_for(pvm_float *x, pvm_float *y, int count, KerFuncType kf_type, pvm_float &res);
	void compute_for_many(pvm_float **xs, pvm_float *results, //the ker result for (xs[i], xs[j]) will be found at position = (i * entries_count + j)
						  int entries_count, int feat_count, KerFuncType kf_type);

	//because each kernel type has at most one float, one int and the vector coresponding to weights
	//we use this less transparent form of parameter setting
	void set_params(pvm_float src_fl0, int src_i0,  GML::Utils::GTVector<pvm_float> *src_weights, KerFuncType kf_type);
};
//-----------------------------------------------------------------------------
pvm_inline void ker_f_wrapper::compute_for(pvm_float *x, pvm_float *y, int count, KerFuncType kf_type, pvm_float &res)
{
	switch(kf_type)
	{
		case KERPOLY: kf_poly.compute_for(x, y, count, res); break;
		case KERSCALAR: kf_scalar.compute_for(x, y, count, res); break;
		case KERRBF: kf_rbf.compute_for(x, y, count, res); break;
		case KERPOLYPARAM: kf_poly_param.compute_for(x, y, count, res); break;
		case KERSCALARPARAM: kf_scalar_param.compute_for(x, y, count, res); break;
		case KERRBFPARAM: kf_rbf_param.compute_for(x, y, count, res); break;

		default : res = 0; break;
	}
}
//-----------------------------------------------------------------------------
#endif// __PVM_KERNEL_WRAPPER_H__


