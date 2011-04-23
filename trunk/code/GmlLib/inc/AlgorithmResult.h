#pragma once

#include "Timer.h"
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
			double		sp,se,acc,med;
			Timer		time;

		public:
			AlgorithmResult(void);

			void		Update(bool classType,bool corectellyClasified,double updateValue = 1);
			void		Clear();
			void		Copy(AlgorithmResult *res);
			void		Add(AlgorithmResult *res);
			void		Compute();
		};
	}
}
