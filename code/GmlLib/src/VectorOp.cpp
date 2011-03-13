#include "VectorOp.h"
#include <math.h>

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
void GML::ML::VectorOp::AdjustTwoStatePerceptronWeights(double *features,double *weights,UInt32 elements,double error,double *featuresWeight)
{
	while (elements>0)
	{
		if ((*features)!=0)
			(*weights) += error*(*featuresWeight);
		features++;
		weights++;
		featuresWeight++;
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
double	GML::ML::VectorOp::PointToPlaneDistanceSquared(double *plane,double *point,UInt32 elements,double planeBias)
{
	double	sum2 = 0.0;	
	// dist Point(x1,x2,...xn) to Plane(a1,a2,...an,bias) = (x1*a1+x2*a2+ ... xn*an+bias)/(a1*a1+a2*a2+...an*an)

	while (elements>0)
	{
		sum2+=((*plane)*(*plane));
		planeBias+=((*plane)*(*point));
		plane++;
		point++;
		elements--;
	}
	
	return (planeBias*planeBias)/sum2;
}
double	GML::ML::VectorOp::PointToPlaneDistance(double *plane,double *point,UInt32 elements,double planeBias)
{
	return sqrt(PointToPlaneDistanceSquared(plane,point,elements,planeBias));
}
double	GML::ML::VectorOp::PointToPlaneDistanceSquaredSigned(double *plane,double *point,UInt32 elements,double planeBias)
{
	double	sum2 = 0.0;	
	// dist Point(x1,x2,...xn) to Plane(a1,a2,...an,bias) = (x1*a1+x2*a2+ ... xn*an+bias)/(a1*a1+a2*a2+...an*an)

	while (elements>0)
	{
		sum2+=((*plane)*(*plane));
		planeBias+=((*plane)*(*point));
		plane++;
		point++;
		elements--;
	}
	
	if (planeBias<0)
		return -(planeBias*planeBias)/sum2;
	else
		return (planeBias*planeBias)/sum2;
}
double	GML::ML::VectorOp::PointToPlaneDistanceSigned(double *plane,double *point,UInt32 elements,double planeBias)
{
	double dist = PointToPlaneDistanceSquaredSigned(plane,point,elements,planeBias);
	if (dist<0)
		return sqrt(-dist);
	return sqrt(dist);
}