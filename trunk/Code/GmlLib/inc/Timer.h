#pragma once

#include "compat.h"
#include "GString.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT Timer
		{
			UInt32			timeStart,timeDiff;
			GString			Result;
		public:

			void			Start();
			void			Stop();
			UInt32			GetPeriodAsMiliSeconds();
			char*			GetPeriodAsString();
			char*			EstimateTotalTime(UInt32 parts,UInt32 total);
			char*			EstimateETA(UInt32 parts,UInt32 total);

		};
	}
}
