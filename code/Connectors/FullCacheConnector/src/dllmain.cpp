// dllmain.cpp : Defines the entry point for the DLL application.

#include <Windows.h>
#include "gmllib.h"
#include "FullCacheConnector.h"

BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{	
	return TRUE;
}

GML::ML::IConector*	CreateNewConnector()
{
	FullCacheConnector* connector = new FullCacheConnector();	
	return connector;
}

bool DestroyConnector (GML::ML::IConector* connector) 
{
	if (connector!=NULL)
		delete connector;
	return true;
}


