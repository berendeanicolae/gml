#pragma once

#include "compat.h"

namespace GML
{
	namespace Notify
	{
		class EXPORT INotify
		{
		public:
			virtual bool	Init(void *initData) = 0;
			virtual bool	Uninit() = 0;
			virtual void	Notify(UInt32 messageID,void *Data,UInt32 DataSize) = 0;

			void			NotifyString(char* format,...);
		};
	}
}