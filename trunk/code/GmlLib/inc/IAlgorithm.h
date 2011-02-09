#ifndef __I_ALGORITHM__
#define __I_ALGORITHM__

#include "Compat.h"
#include "AttributeList.h"

namespace GML
{
	namespace Algorithm
	{
		class EXPORT IAlgorithm
		{
		public:
			virtual bool	SetConfiguration(GML::Utils::AttributeList &config) = 0;
			virtual bool	GetConfiguration(GML::Utils::AttributeList &config) = 0;
			virtual bool	Init() = 0;
			virtual void	Execute(UInt32 command)=0;
		};
	}
}


#endif
