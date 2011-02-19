#ifndef __CONSOLE__NOTIFYER__
#define __CONSOLE__NOTIFYER__

#include "gmllib.h"

class ConsoleNotifyer: public GML::Utils::INotify
{
	bool	useColors;
	void	SetColor(unsigned char fore,unsigned char back);
public:
	ConsoleNotifyer();
	bool	OnInit();
	bool	Uninit();
	bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize);
};
 

#endif
