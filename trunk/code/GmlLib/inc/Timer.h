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
		public:

			void			Start();
			void			Stop();
			UInt32			GetPeriodAsMiliSeconds();
			char*			GetPeriodAsString(GString &str);
			char*			EstimateTotalTime(GString &str,UInt32 parts,UInt32 total);
			char*			EstimateETA(GString &str,UInt32 parts,UInt32 total);
		};
	}
}
