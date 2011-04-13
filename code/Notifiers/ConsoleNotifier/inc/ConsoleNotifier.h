#ifndef __CONSOLE__NOTIFYER__
#define __CONSOLE__NOTIFYER__

#include "gmllib.h"

class ConsoleNotifier: public GML::Utils::INotifier
{
	COORD					savedCursorCoord;
	GML::Utils::GString		tempStr;
	bool					useColors;
	void					SetColor(unsigned char fore,unsigned char back);
	void					SaveCursorCoord();
	void					RestoreCursorCoord();
	void					PrintText(char *text,unsigned char fore,unsigned char back,bool printCRLF);
public:
	ConsoleNotifier();
	bool	OnInit();
	bool	Uninit();
	bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize);
};
 

#endif
