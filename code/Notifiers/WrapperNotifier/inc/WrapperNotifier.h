#ifndef __PIPE__NOTIFYER__
#define __PIPE__NOTIFYER__

#include "gmllib.h"

class PipeNotifier: public GML::Utils::INotifier
{
	GML::Utils::GString		Client;
	bool					CreateNewConsole;

	PROCESS_INFORMATION		pi;
	HANDLE					hPipe;
	
public:
	PipeNotifier();
	bool	OnInit();
	bool	Uninit();
	bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize);
};
 

#endif
