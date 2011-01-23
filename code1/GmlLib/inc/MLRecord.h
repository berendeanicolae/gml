#pragma once

#include "Compat.h"
#include "DBRecord.h"

namespace GML
{
	namespace ML
	{
		struct EXPORT MLRecord
		{
			UInt32				FeatCount;
			double*				Features;
			double				Weight;
			GML::DB::RecordHash	Hash;	
			double				Label;
		};
	}
}

