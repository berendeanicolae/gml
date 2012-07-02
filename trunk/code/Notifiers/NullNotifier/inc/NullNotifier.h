#ifndef __NULL__NOTIFYER__
#define __NULL__NOTIFYER__

#include "gmllib.h"

class NullNotifier: public GML::Utils::INotifier
{
public:
	NullNotifier();
	bool	OnInit();
	bool	Uninit();
	bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize);
};
 

#endif
