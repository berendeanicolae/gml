#include "VectorOp.h"


void	GML::ML::VectorOp::AddVectors(double *v1,double *v2,UInt32 elements)
{
	while (elements>0)
	{
		(*v1)+=(*v2);
		v1++;
		v2++;
		elements--;
	}
}
double	GML::ML::VectorOp::ComputeVectorsSum(double *v1,double *v2,UInt32 elements)
{
	double	sum = 0.0;
	while (elements>0)
	{
		sum += ((*v1) * (*v2));
		v1++;
		v2++;
		elements--;
	}
	return sum;
}
bool GML::ML::VectorOp::IsPerceptronTrained(double *v1,double *v2,UInt32 elements,double label)
{
	return ((label*ComputeVectorsSum(v1,v2,elements))>0);
}
bool GML::ML::VectorOp::IsPerceptronTrained(double *v1,double *v2,UInt32 elements,double b,double label)
{	
	return (bool)((label*(ComputeVectorsSum(v1,v2,elements)+b))>0.0);
}
void GML::ML::VectorOp::AdjustTwoStatePerceptronWeights(double *features,double *weights,UInt32 elements,double error)
{
	while (elements>0)
	{
		if ((*features)!=0)
			(*weights) += error;
		features++;
		weights++;
		elements--;
	}
}
void GML::ML::VectorOp::AdjustPerceptronWeights(double *features,double *weights,UInt32 elements,double error)
{
	while (elements>0)
	{
		(*features) += ((*weights)*error);
		features++;
		weights++;
		elements--;
	}
}