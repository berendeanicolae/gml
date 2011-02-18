// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmllib.h"



int _tmain(int argc, _TCHAR* argv[])
{
	GML::Utils::AttributeList	attr;

	attr.AddString("DbName","SimpleTextFileDB");
	attr.AddString("DbConnString","FileName=E:\\lucru\\GML\\gml\\bin\\gdt-db.txt");  
	attr.AddString("Conector","BitConnector");
	attr.AddString("Notifyer","ConsoleNotifyer");
	attr.AddDouble("LearningRate",0.01);
	attr.AddUInt32("MaxIteratii",100);

	GML::Algorithm::IAlgorithm *alg = GML::Builder::CreateAlgorithm("SimplePerceptronAlgorithm","");
	if (alg==NULL)
		return 1;
	if (alg->SetConfiguration(attr)==false)
		return 1;
	if (alg->Init()==false)
		return 1;
	
	alg->Execute(0);

	return 0;
}
