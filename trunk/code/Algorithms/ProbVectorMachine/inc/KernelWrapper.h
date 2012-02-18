#ifndef __PVM_KERNEL_WRAPPER_H__
#define __PVM_KERNEL_WRAPPER_H__
//-----------------------------------------------------------------------------
#include "KernelFunctionInclude.h"
//-----------------------------------------------------------------------------
typedef enum KerFuncType {KERPOLY, KERSCALAR, KERRBF, KERPOLYPARAM, KERSCALARPARAM, KERRBFPARAM};
//-----------------------------------------------------------------------------
class ker_f_wrapper : public GML::Utils::GMLObject
{
	ker_f_wrapper();//this is set here to avoid usage of the constructor without setting the notifier
public:
	ker_f_scalar		kf_scalar;
	ker_f_poly			kf_poly;
	ker_f_rbf			kf_rbf;
		
	ker_f_scalar_param	kf_scalar_param;
	ker_f_poly_param	kf_poly_param;
	ker_f_rbf_param		kf_rbf_param;

	ker_f				*kf;

	KerFuncType			kf_type;

	GML::ML::IConnector*	con;
	GML::Utils::INotifier*	notif;	
	

	ker_f_wrapper(GML::ML::IConnector *src_con, GML::Utils::INotifier *src_notif);
	ker_f_wrapper(ker_f_wrapper &src_wrapper);
	~ker_f_wrapper();

	double compute_for(pvm_double *x, pvm_double *y, int count);
	double compute_for(GML::ML::MLRecord &ml_rec0, GML::ML::MLRecord &ml_rec1);

	void compute_for_many(pvm_double **xs, pvm_double *results, //the ker result for (xs[i], xs[j]) will be found at position = (i * entries_count + j)
						  int entries_count, int feat_count, KerFuncType kf_type);

	/*because each kernel type has at most one float, one int and the vector coresponding to weights
	we use this less transparent form of parameter setting

	kf_type == KERPOLY : General form :  K(X, Y) = (<X, Y> + t)^d, where t = src_fl0, d = src_i0, with t >=0, d > 0 and odd
						 Default settings : t = 1.0, d = 3

	kf_type == KERSCALAR : General form : K(X, Y) = <X, Y>. It ignores the values for the parameters

	kf_type == KERRBF : General form : K(X, Y) = exp(-gamma * ||X - Y||^2) = exp( -gamma * <X - Y, X - Y>), where gamma = src_fl0
						Default settings : gamma = exp (-5)

	Note that every kernel type is based on the KERSCALAR kernel type. For every ker type there's a coresponding weighted kernel.
	If the ker type is PARAM type, then it will use the weights supplied in the src_weights vector, which will alter the 
	base scalar product : <X, Y> = sum(Xi * Yi * weight_i)
	*/	
	bool set_params(pvm_double src_fl0, int src_i0,  GML::Utils::GTVector<pvm_double> *src_weights, KerFuncType kf_type);

	//must be called with a valid notifier, otherwise it will generate access violations in case of assertion fails
	bool set_inherit_data(GML::ML::IConnector *src_con, GML::Utils::INotifier *src_notif);
	void set_ker_type(KerFuncType src_kf_type);	
};
//-----------------------------------------------------------------------------
//--------------------------INLINES--------------------------------------------
//-----------------------------------------------------------------------------
pvm_inline double ker_f_wrapper::compute_for(pvm_double *x, pvm_double *y, int count)
{
	DBGSTOP_CHECKMSG(kf, "Should have been set by calling set_params");
	return kf->compute_for(x, y, count);
}
//-----------------------------------------------------------------------------
pvm_inline double ker_f_wrapper::compute_for(GML::ML::MLRecord &ml_rec0, GML::ML::MLRecord &ml_rec1)
{
	DBGSTOP_CHECKMSG(ml_rec0.FeatCount == ml_rec1.FeatCount, "Records must have same number of features");	
	return compute_for(ml_rec0.Features, ml_rec1.Features, ml_rec1.FeatCount);
}
//-----------------------------------------------------------------------------
pvm_inline bool ker_f_wrapper::set_inherit_data(GML::ML::IConnector *src_con, GML::Utils::INotifier *src_notif)
{
	bool ret = true;
	con = src_con, notif = src_notif;

	ret = ret && kf_scalar.set_inherit_data(src_con, src_notif);
	ret = ret && kf_scalar_param.set_inherit_data(src_con, src_notif);
	ret = ret && kf_poly.set_inherit_data(src_con, src_notif);
	ret = ret && kf_poly_param.set_inherit_data(src_con, src_notif);
	ret = ret && kf_rbf.set_inherit_data(src_con, src_notif);
	ret = ret && kf_rbf_param.set_inherit_data(src_con, src_notif);

	return ret;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif// __PVM_KERNEL_WRAPPER_H__


