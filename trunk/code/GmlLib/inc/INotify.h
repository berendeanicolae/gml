#pragma once

#include "compat.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT INotify
		{
		public:
			enum {
				NOTIFY_ERROR = 0,
				NOTIFY_INFO,
			};
			virtual bool	Init(void *initData) = 0;
			virtual bool	Uninit() = 0;
			virtual bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize) = 0;

			bool			NotifyString(UInt32 messageID,char* format,...);
			bool			Info(char *format,...);
			bool			Error(char *format,...);
		};
	}
}