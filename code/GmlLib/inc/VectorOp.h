#ifndef __VECTOR__OP__
#define __VECTOR__OP__

#include "Compat.h"

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
			static void		AdjustTwoStatePerceptronWeights(double *features,double *weights,UInt32 elements,double error);
			static void		AdjustPerceptronWeights(double *features,double *weights,UInt32 elements,double error);

		};
	}
}


#endif
