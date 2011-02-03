// dllmain.cpp : Defines the entry point for the DLL application.
#include "gmllib.h"
#include "ConsoleNotifyer.h"

GML::Utils::INotify*	Create(void *objData)
{
	ConsoleNotifyer		*notif = new ConsoleNotifyer();

	if (notif==NULL)
		return NULL;
	if (notif->Init(objData)==NULL)
	{
		delete notif;
		return NULL;
	}
	return notif;
}

BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	return TRUE;
}

