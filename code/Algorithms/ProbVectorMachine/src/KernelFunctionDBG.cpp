//-----------------------------------------------------------------------------
#include "KernelFunctionDBG.h"
#include "KernelWrapper.h"
//-----------------------------------------------------------------------------
bool ker_f_dbg::exec_kernel_func_dbg(GML::ML::IConnector *connector)
{
	int i, j, k, rec_count, feat_count;
	pvm_float ker_res;
	ker_f_poly kf_poly;
	ker_f_rbf kf_rbf;
	ker_f_scalar kf_scalar;
	float *x, *y;
	ker_f_wrapper kf_wrap;


	GML::ML::MLRecord ml_rec;
	connector->CreateMlRecord(ml_rec);

	rec_count = connector->GetRecordCount();

	if (rec_count > 10) rec_count = 10;

	feat_count = connector->GetFeatureCount();

	kf_poly.set_parameters(1.0f, 3);
	kf_rbf.set_parameters(1.0f);

	x = (pvm_float *)malloc(feat_count * sizeof(pvm_float));
	y = (pvm_float *)malloc(feat_count * sizeof(pvm_float));

	for (i = 0; i < rec_count; i++)
	{
		connector->GetRecord(ml_rec, i);
		for (k = 0; k < feat_count; k++)
			x[k] = (pvm_float)ml_rec.Features[k];

		for (j = i + 1; j < rec_count; j++)
		{
			connector->GetRecord(ml_rec, j);
			for (k = 0; k < feat_count; k++)
				y[k] = (pvm_float)ml_rec.Features[k];

			//kf_poly.compute_for(x, y, feat_count, ker_res);
			//kf_rbf.compute_for(x, y, feat_count, ker_res);
			kf_scalar.compute_for(x, y, feat_count, ker_res);
		}
	}

	if (rec_count)
	{
		pvm_float **xs;
		pvm_float *results;

		connector->GetRecord(ml_rec, 0);
		for (k = 0; k < feat_count; k++)
			x[k] = (pvm_float)ml_rec.Features[k];
		
		connector->GetRecord(ml_rec, 1);
		for (k = 0; k < feat_count; k++)
			y[k] = (pvm_float)ml_rec.Features[k];

		
		xs = (pvm_float **)malloc(2 * sizeof(pvm_float *));
		results = (pvm_float *)malloc(3 * sizeof(pvm_float));
		xs[0] = x;
		xs[1] = y;

		kf_wrap.set_params(1.0, 3, NULL, KERPOLY);

		kf_wrap.compute_for_many(xs, results, 2, feat_count, KERPOLY);


		free(xs);
		free(results);
	}

	free(x);
	free(y);

	return true;
}
//-----------------------------------------------------------------------------