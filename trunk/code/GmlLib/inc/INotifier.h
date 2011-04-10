#pragma once

#include "compat.h"
#include "GMLObject.h"
#include "AttributeList.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT INotifier: public GMLObject
		{

		public:
			enum {
				NOTIFY_ERROR = 0,
				NOTIFY_INFO,
				NOTIFY_GLOBAL_PROC_TEXT,
				NOTIFY_LOCAL_PROC_TEXT,
				NOTIFY_GLOBAL_PROC,
				NOTIFY_LOCAL_PROC
			};
			bool			Init(char *attributeString);

			virtual bool	OnInit() = 0;
			virtual bool	Uninit() = 0;
			virtual bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize) = 0;

			bool			NotifyString(UInt32 messageID,char* format,...);
			bool			Info(char *format,...);
			bool			Error(char *format,...);
		};
	}
}