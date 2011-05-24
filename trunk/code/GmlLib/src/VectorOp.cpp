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

bool    GML::ML::VectorOp::IsPerceptronTrained(double *v1,double *v2,UInt32 elements,double label)
{
	return ((label*ComputeVectorsSum(v1,v2,elements))>0);
}
bool	GML::ML::VectorOp::IsPerceptronTrained(double *v1,double *v2,UInt32 elements,double b,double label)
{	
	return (bool)((label*(ComputeVectorsSum(v1,v2,elements)+b))>0.0);
}

void	GML::ML::VectorOp::AdjustTwoStatePerceptronWeights(double *features,double *weights,UInt32 elements,double error)
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
void	GML::ML::VectorOp::AdjustTwoStatePerceptronWeights(double *features,double *weights,UInt32 elements,double error,double *featuresWeight)
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
void	GML::ML::VectorOp::AdjustPerceptronWeights(double *features,double *weights,UInt32 elements,double error)
{
	while (elements>0)
	{
		(*weights) += ((*features)*error);
		features++;
		weights++;
		elements--;
	}
}
void	GML::ML::VectorOp::AdjustPerceptronWeights(double *features,double *weights,UInt32 elements,double error,double *featuresWeight)
{
	while (elements>0)
	{
		(*weights) += error*(*featuresWeight)*(*features);
		features++;
		weights++;
		featuresWeight++;
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

double	GML::ML::VectorOp::MinkowskiDistance(double *p1,double *p2,UInt32 elements,double power)
{
	double sum=0.0;;
	double temp;
	while (elements>0)
	{
		temp = (*p1)-(*p2);
		if (temp<0)
			temp = -temp;
		temp = pow(temp,power);
		sum+= temp;
		p1++;
		p2++;
		elements--;
	}
	return pow(sum,1.0/power);
}
double	GML::ML::VectorOp::MinkowskiDistance(double *p1,double *p2,UInt32 elements,double power,double *pWeight)
{
	double sum=0.0;;
	double temp;
	while (elements>0)
	{
		temp = ((*p1)-(*p2))*(*pWeight);
		if (temp<0)
			temp = -temp;

		temp = pow(temp,power);
		sum+= temp;
		p1++;
		p2++;
		pWeight++;
		elements--;
	}
	return pow(sum,1.0/power);
}
double	GML::ML::VectorOp::ManhattanDistance(double *p1,double *p2,UInt32 elements)
{
	double sum=0.0;;
	double temp;
	while (elements>0)
	{
		temp = (*p1)-(*p2);
		if (temp<0)
			temp = -temp;
		sum+= temp;
		p1++;
		p2++;
		elements--;
	}
	return sum;
}
double	GML::ML::VectorOp::ManhattanDistance(double *p1,double *p2,UInt32 elements,double *pWeight)
{
	double sum=0.0;;
	double temp;
	while (elements>0)
	{
		temp = ((*p1)-(*p2))*(*pWeight);
		if (temp<0)
			temp = -temp;
		sum+= temp;
		p1++;
		p2++;
		pWeight++;
		elements--;
	}
	return sum;
}
double	GML::ML::VectorOp::EuclideanDistance(double *p1,double *p2,UInt32 elements)
{
	double sum=0.0;;
	double temp;
	while (elements>0)
	{
		temp = (*p1)-(*p2);
		sum+= (temp*temp);
		p1++;
		p2++;
		elements--;
	}
	return sqrt(sum);
}
double	GML::ML::VectorOp::EuclideanDistance(double *p1,double *p2,UInt32 elements,double *pWeight)
{
	double sum=0.0;;
	double temp;
	while (elements>0)
	{
		temp = ((*p1)-(*p2))*(*pWeight);
		sum+= (temp*temp);
		p1++;
		p2++;
		pWeight++;
		elements--;
	}
	return sqrt(sum);
}
double  GML::ML::VectorOp::EuclideanDistanceSquared(double *p1,double *p2,UInt32 elements)
{
	double sum=0.0;;
	double temp;
	while (elements>0)
	{
		temp = (*p1)-(*p2);
		sum+= (temp*temp);
		p1++;
		p2++;
		elements--;
	}
	return sum;
}
double  GML::ML::VectorOp::BinomialDistance(double *p1,double *p2,UInt32 elements,double power)
{
	double sum=0.0;
	while (elements>0)
	{
		sum+= ((*p1)*(*p2));
		p1++;
		p2++;
		elements--;
	}
	return pow(sum,power);
}
double  GML::ML::VectorOp::RadialDistance(double *p1,double *p2,UInt32 elements,double sigma)
{
	double sum=0.0;
	double sum1=0.0;
	double sum2=0.0;

	while (elements>0)
	{
		sum  += ((*p1)*(*p2));
		sum1 += ((*p1)*(*p1));
		sum2 += ((*p2)*(*p2));
		p1++;
		p2++;
		elements--;
	}
	return exp((-(sum1+sum2-sum*2))/(2*sigma*sigma));
}
double  GML::ML::VectorOp::SigmoidDistance(double *p1,double *p2,UInt32 elements,double k)
{
	double sum=0.0;

	while (elements>0)
	{
		sum  += ((*p1)*(*p2));
		p1++;
		p2++;
		elements--;
	}
	return 1.0/(1+pow(2.71828182845904523536,-k*sum));
}
double  GML::ML::VectorOp::Average(double *v,UInt32 elements)
{
	double sum = 0;
	double count = elements;
	while (elements>0)
	{
		sum+=(*v);
		v++;
		elements--;
	}
	return sum / count;
}
double  GML::ML::VectorOp::StandardDeviation(double *v,UInt32 elements)
{
	double av = Average(v,elements);
	double sum = 0;
	double count = elements;

	while (elements>0)
	{
		sum+=((*v)-av) * ((*v)-av);
		v++;
		elements--;
	}
	return sqrt(sum / count);
}
