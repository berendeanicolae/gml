#ifndef __INDEXES__
#define __INDEXES__

#include "Compat.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT Indexes
		{
			UInt32	*List;
			UInt32	MaxAlloc;
			UInt32	Count;
		public:
			Indexes();
			~Indexes();
			
			bool		Create(UInt32 count);
			void		Destroy();
			
			bool		Push(UInt32 index);
			UInt32	Get(UInt32 poz);
			UInt32*	GetList();
			UInt32	Len();
			bool		Truncate(UInt32 newCount);						
		}
	}
}

#endif

