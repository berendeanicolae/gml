// GMLExec.cpp : Defines the entry point for the console application.
//

#include "gmllib.h"


int main(int argc, char* argv[])
{
	GML::Utils::AttributeList	config;
	GML::Utils::GString			algName;
	GML::Algorithm::IAlgorithm	*alg;

	if (argc<=1)
	{
		printf("Usage: GMLExec.exe <config_file>\n");
		return 1;
	}
	if (config.Load(argv[1])==false)
	{
		printf("[ERROR] -> unable to load properties from %s\n",argv[1]);
		return 2;
	}
	// aflu numele algoritmului
	if (config.UpdateString("AlgorithmName",algName)==false)
	{
		printf("[ERROR] -> Missing 'AlgorithmName' property from %s",argv[1]);
		return 3;
	}
	if ((alg=GML::Builder::CreateAlgorithm(algName.GetText()))==NULL)
	{
		printf("[ERROR] -> Error creating '%s'",algName.GetText());
		return 4;
	}
	if (alg->SetProperty(config)==false)
	{
		printf("[ERROR] -> Error updateing configuration to '%s'",algName.GetText());
		return 5;
	}
	if (alg->Init()==false)
	{
		printf("[ERROR] -> Error on initialization of '%s'",algName.GetText());
		return 6;
	}
	if (alg->Execute()==false)
	{
		printf("[ERROR] -> Error on execution of '%s'",algName.GetText());
		return 7;
	}
	alg->Wait();
	printf("Algorithm '%s' done !\n",algName.GetText());
	return 0;
}

