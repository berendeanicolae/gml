// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmllib.h"



int _tmain(int argc, _TCHAR* argv[])
{
	GML::Utils::AttributeList	attr;

	attr.AddString("DbName","SimpleTextFileDB");
	attr.AddString("DbConnString","FileName=E:\\lucru\\GML\\gml\\bin\\a.txt");  
	attr.AddString("Conector","BitConnector{Table=RecordTable}");
	attr.AddString("Notifyer","ConsoleNotifyer{test=sal}");
	attr.AddDouble("LearningRate",0.02);
	attr.AddUInt32("MaxIteratii",30);

	GML::Algorithm::IAlgorithm *alg = GML::Builder::CreateAlgorithm("SimplePerceptronAlgorithm","");
	if (alg==NULL)
		return 1;
	if (alg->SetConfiguration(attr)==false)
		return 1;
	if (alg->Init()==false)
		return 1;
	
	alg->Execute("train");
	alg->Wait();

	printf("Algorithm done !\n");
	return 0;
}

