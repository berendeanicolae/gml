#include "VectorOp.h"


void	GML::ML::VectorOp::AddVectors(double *v1,double *v2,UInt32 elements)
{
	Int32	contor = (Int32) elements;
	while (contor>=0)
	{
		(*v1)+=(*v2);
		v1++;
		v2++;
		contor--;
	}
}
double	GML::ML::VectorOp::ComputeVectorsSum(double *v1,double *v2,UInt32 elements)
{
	Int32	contor = (Int32) elements;
	double	sum = 0.0;
	while (contor>=0)
	{
		sum += ((*v1) * (*v2));
		v1++;
		v2++;
		contor--;
	}
	return sum;
}
bool GML::ML::VectorOp::IsPerceptronTrained(double *v1,double *v2,UInt32 elements,double label)
{
	return ((label*ComputeVectorsSum(v1,v2,elements))>0);
}
void GML::ML::VectorOp::AdjustTwoStatePerceptronWeights(double *features,double *weights,UInt32 elements,double error)
{
	Int32	contor = (Int32) elements;
	
	while (contor>=0)
	{
		if ((*features)!=0)
			(*weights) += error;
		features++;
		weights++;
		contor--;
	}
}
void GML::ML::VectorOp::AdjustPerceptronWeights(double *features,double *weights,UInt32 elements,double error)
{
	Int32	contor = (Int32) elements;
	
	while (contor>=0)
	{
		(*features) += ((*weights)*error);
		features++;
		weights++;
		contor--;
	}
}