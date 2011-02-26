// dllmain.cpp : Defines the entry point for the DLL application.
#include "gmllib.h"
#include "RosenblattPerceptron.h"
#include "BatchPerceptron.h"

GML::Algorithm::IAlgorithm*	CreateNewAlgorithm(char *algName)
{
	if ((GML::Utils::GString::Equals(algName,"RosenblattPerceptron",true)) ||
		(GML::Utils::GString::Equals(algName,"SimplePerceptron",true)))
	{
		return new RosenblattPerceptron();
	}
	if (GML::Utils::GString::Equals(algName,"BatchPerceptron",true))
		return new BatchPerceptron();


	return NULL;
}

BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	return TRUE;
}