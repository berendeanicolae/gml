// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmllib.h"



int _tmain(int argc, _TCHAR* argv[])
{
	GML::Utils::AttributeList	attr;
	GML::Utils::GString			str;

	attr.Clear();
	attr.Create("dbPath=E:\\mcu.db");
	attr.UpdateString("dbPath",str);

	attr.AddString("DbName","Sqlite");
	attr.AddString("DbConnString","FileName=D:\\Research\\gml\\tests\\mcu.db.tiny");  
	attr.AddString("Conector","FullCacheConnector");
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

