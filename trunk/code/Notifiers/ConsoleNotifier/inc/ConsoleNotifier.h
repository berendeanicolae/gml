#ifndef __CONSOLE__NOTIFYER__
#define __CONSOLE__NOTIFYER__

#include "gmllib.h"

class ConsoleNotifier: public GML::Utils::INotifier
{
	bool	useColors;
	void	SetColor(unsigned char fore,unsigned char back);
public:
	ConsoleNotifier();
	bool	OnInit();
	bool	Uninit();
	bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize);
};
 

#endif
