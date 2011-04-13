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
				NOTIFY_START_PROCENT,
				NOTIFY_END_PROCENT,
				NOTIFY_PROCENT,				
			};
			bool			Init(char *attributeString);

			virtual bool	OnInit() = 0;
			virtual bool	Uninit() = 0;
			virtual bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize) = 0;

			bool			NotifyString(UInt32 messageID,char* format,...);
			bool			Info(char *format,...);
			bool			Error(char *format,...);
			bool			StartProcent(char *format,...);
			bool			SetProcent(double procValue);
			bool			SetProcent(double procValue,double maxValue);
			bool			EndProcent();
		};
	}
}