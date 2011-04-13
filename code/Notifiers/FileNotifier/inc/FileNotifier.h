#ifndef __FILE__NOTIFYER__
#define __FILE__NOTIFYER__

#include "gmllib.h"

class FileNotifier: public GML::Utils::INotifier
{
	GML::Utils::GString		fileName;
	bool					flushAfterEachWrite;

	GML::Utils::File		file;
public:
	FileNotifier();
	bool	OnInit();
	bool	Uninit();
	bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize);
};
 

#endif
