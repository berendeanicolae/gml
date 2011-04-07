// GML.cpp : Defines the entry point for the console application.
//

#include "gmllib.h"

int  Error(char *message)
{
	printf("[ERROR] : %s\n",message);
	return -1;
}
int Run(char *templateName)
{
	GML::Utils::AttributeList	config;
	GML::Utils::GString			algName;
	GML::Algorithm::IAlgorithm	*alg;

	if (config.Load(templateName)==false)
	{
		printf("[ERROR] -> unable to load properties from %s\n",templateName);
		return 2;
	}
	// aflu numele algoritmului
	if (config.UpdateString("AlgorithmName",algName)==false)
	{
		printf("[ERROR] -> Missing 'AlgorithmName' property from %s\n",templateName);
		return 3;
	}
	if ((alg=GML::Builder::CreateAlgorithm(algName.GetText()))==NULL)
	{
		printf("[ERROR] -> Error creating '%s'\n",algName.GetText());
		return 4;
	}
	if (alg->SetProperty(config)==false)
	{
		printf("[ERROR] -> Error updateing configuration to '%s'\n",algName.GetText());
		return 5;
	}
	if (alg->Init()==false)
	{
		printf("[ERROR] -> Error on initialization of '%s'\n",algName.GetText());
		return 6;
	}
	if (alg->Execute()==false)
	{
		printf("[ERROR] -> Error on execution of '%s'\n",algName.GetText());
		return 7;
	}
	alg->Wait();
	printf("Algorithm '%s' done !\n",algName.GetText());
	return 0;
}
void ShowHelp()
{
	printf("GML.EXE , Copyright @2011 , Gavrilut Dragos & co\n");
	printf("Build on %s (%s)\n",__DATE__,__TIME__);
	printf("Usage: gml.exe command <parameters>\n");
	printf("Where: command is one of the following\n");
	printf("       run <template_file>  -> executes a template file\n");
	printf("       info <object_name>   -> shows informations about a specific Algorithm,Conector,DataBase or Notifier\n");

	printf("\n");
}
int  main(int argc, char* argv[])
{
	if (argc<=1)
	{
		ShowHelp();
		return 1;
	}
	if (GML::Utils::GString::Equals(argv[1],"run",true))
	{
		if (argc!=3)
			return Error("run command requare a parameter (a template file)");
		return Run(argv[2]);
	}
	printf("[ERROR] Unknwon command : %s \n",argv[1]);
	return -1;
}