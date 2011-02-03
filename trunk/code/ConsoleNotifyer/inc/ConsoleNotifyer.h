#ifndef __CONSOLE__NOTIFYER__
#define __CONSOLE__NOTIFYER__

#include "gmllib.h"

class ConsoleNotifyer: public GML::Utils::INotify
{
public:
	bool	Init(void *initData);
	bool	Uninit();
	bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize);
};


#endif
