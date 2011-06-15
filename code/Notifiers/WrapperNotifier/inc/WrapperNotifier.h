#ifndef __WRAPPER__NOTIFYER__
#define __WRAPPER__NOTIFYER__

#include "gmllib.h"

class WrapperNotifier: public GML::Utils::INotifier
{
	GML::Utils::GString		PluginPath;	

	HMODULE					hModule;
	void*					Context;
	void*					(*fnInit)();
	bool					(*fnNotify)(void* Context,UInt32 messageID,void *Data,UInt32 DataSize);
	bool					(*fnUninit)(void* Context);
	
public:
	WrapperNotifier();
	bool	OnInit();
	bool	Uninit();
	bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize);
	bool	SuportObjects() { return true; }
};
 

#endif
