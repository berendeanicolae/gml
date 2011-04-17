#ifndef __BIT_SET__
#define __BIT_SET__

#include "compat.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT BitSet
		{
			UInt8	*Data;
			UInt32	Allocated;
			UInt32	ElementsCount;
		public:
			BitSet();
			~BitSet();

			bool	Create(UInt32 elements);
			void	Destroy();
			bool	Get(UInt32 poz);
			bool	Set(UInt32 poz,bool value);
			void	SetAll(bool value);
			bool	Reverse(UInt32 poz);
			void	ReverseAll();
			UInt32	Len();
			UInt32	GetAllocated();
			UInt8*	GetData();
		};
	}
}
#endif
