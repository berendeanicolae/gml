// dllmain.cpp : Defines the entry point for the DLL application.
#include "gmllib.h"
//#include "SimplePerceptronAlgorithm.h"

GML::Algorithm::IAlgorithm*	CreateNewAlgorithm(char *algName)
{
	//SimplePerceptronAlgorithm	*alg = new SimplePerceptronAlgorithm();

	//if (alg==NULL)
		return NULL;

	//return alg;
}

BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	return TRUE;
}