#pragma once

#include "compat.h"
#include "AttributeList.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT INotify
		{
			GML::Utils::AttributeList	Attr;
		public:
			enum {
				NOTIFY_ERROR = 0,
				NOTIFY_INFO,
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