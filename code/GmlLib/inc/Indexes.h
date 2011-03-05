#ifndef __INDEXES__
#define __INDEXES__

#include "Compat.h"

#define INVALID_INDEX	((UInt32)(-1))

namespace GML
{
	namespace Utils
	{
		class EXPORT Interval
		{
		public:
			UInt32	Start,End;

			Interval();
			void	Set(UInt32 _start,UInt32 _end);
			UInt32	Size();
		};
		class EXPORT Indexes
		{
		protected:
			UInt32		*List;
			UInt32		MaxAlloc;
			UInt32		Count;
		public:
			Indexes();
			~Indexes();
			
			bool		Create(UInt32 count);
			void		Destroy();
			
			bool		Push(UInt32 index);
			UInt32		Get(UInt32 poz);
			UInt32*		GetList();			
			UInt32		Len();
			UInt32		GetTotalAlloc();
			bool		Truncate(UInt32 newCount);	
			bool		CopyTo(GML::Utils::Indexes &idx);
		};
	}
}

#endif

