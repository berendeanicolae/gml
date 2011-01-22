#pragma once

#include "compat.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT AlgorithmResult
		{
		public:
			UInt32		Iteration;
			double		tp,tn,fp,fn;
			double		sp,se,acc;

		public:
			AlgorithmResult(void);

			void		Clear();
			void		Copy(AlgorithmResult *res);
			void		Add(AlgorithmResult *res);
			void		Compute();
		};
	}
}
