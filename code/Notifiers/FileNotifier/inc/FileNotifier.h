#ifndef __FILE__NOTIFYER__
#define __FILE__NOTIFYER__

#include "gmllib.h"

class FileNotifier: public GML::Utils::INotifier
{
	enum
	{
		SHOW_TIME_NONE = 0,
		SHOW_TIME_DATETIME,
		SHOW_TIME_MILISECONDS
	};

	GML::Utils::GString		fileName;
	bool					flushAfterEachWrite;
	UInt32					showNotificationTime;
	double					lastProcValue;
	GML::Utils::File		file;
public:
	FileNotifier();
	bool	OnInit();
	bool	Uninit();
	bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize);
};
 

#endif
