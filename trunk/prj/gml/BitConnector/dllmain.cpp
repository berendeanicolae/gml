#include "gmllib.h"
#include "BitConnector.h"

BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{	
	return TRUE;
}

GML::ML::IConector*	CreateNewConnector()
{
	BitConnector* connector = new BitConnector();	
	return connector;
}