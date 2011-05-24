#ifndef __VECTOR__OP__
#define __VECTOR__OP__

#include "Compat.h"

#define Mean	Average

namespace GML
{
	namespace ML
	{
		class EXPORT VectorOp
		{
		public:
			static void		AddVectors(double *v1,double *v2,UInt32 elements);
			static double	ComputeVectorsSum(double *v1,double *v2,UInt32 elements);
			
			// perceptron specific
			static bool		IsPerceptronTrained(double *features,double *weights,UInt32 elements,double label);
			static bool		IsPerceptronTrained(double *features,double *weights,UInt32 elements,double b,double label);
			


			static void		AdjustTwoStatePerceptronWeights(double *features,double *weights,UInt32 elements,double error);
			static void		AdjustTwoStatePerceptronWeights(double *features,double *weights,UInt32 elements,double error,double *featuresWeight);			
			static void		AdjustPerceptronWeights(double *features,double *weights,UInt32 elements,double error);
			static void		AdjustPerceptronWeights(double *features,double *weights,UInt32 elements,double error,double *featuresWeight);
			static double	PointToPlaneDistance(double *plane,double *point,UInt32 elements,double planeBias);
			static double	PointToPlaneDistanceSquared(double *plane,double *point,UInt32 elements,double planeBias);
			static double	PointToPlaneDistanceSigned(double *plane,double *point,UInt32 elements,double planeBias);	
			static double	PointToPlaneDistanceSquaredSigned(double *plane,double *point,UInt32 elements,double planeBias);			
			
			static double	MinkowskiDistance(double *p1,double *p2,UInt32 elements,double power);
			static double	MinkowskiDistance(double *p1,double *p2,UInt32 elements,double power,double *pWeight);
			static double	ManhattanDistance(double *p1,double *p2,UInt32 elements);
			static double	ManhattanDistance(double *p1,double *p2,UInt32 elements,double *pWeight);
			static double	EuclideanDistance(double *p1,double *p2,UInt32 elements);
			static double	EuclideanDistanceSquared(double *p1,double *p2,UInt32 elements);
			static double	EuclideanDistance(double *p1,double *p2,UInt32 elements,double *pWeight);
			static double	BinomialDistance(double *p1,double *p2,UInt32 elements,double power);
			static double	RadialDistance(double *p1,double *p2,UInt32 elements,double sigma);
			static double	SigmoidDistance(double *p1,double *p2,UInt32 elements,double k=1);

			static double	Average(double *v,UInt32 elements);
			static double	StandardDeviation(double *v,UInt32 elements);
			
		};
	}
}


#endif
