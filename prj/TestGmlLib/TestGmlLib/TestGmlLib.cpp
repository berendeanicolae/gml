// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "gmllib.h"


int _tmain(int argc, _TCHAR* argv[])
{
	GML::Utils::AttributeList	attr;

	attr.AddString("DbName","E:\\lucru\\GML\\gml\\bin\\Sqlite.db");
	attr.AddString("DbConnString","IP=127.0.0.1;Port=80;");
	attr.AddString("Conector","MCU_conector");
	attr.AddString("Notifyer","E:\\lucru\\GML\\gml\\bin\\ConsoleNotifyer.ntf");
	attr.AddDouble("LearningRate",0.01);
	attr.AddUInt32("MaxIteratii",100);

	GML::Algorithm::IAlgorithm *alg = GML::Builder::CreateAlgorithm("E:\\lucru\\GML\\gml\\bin\\SimplePerceptronAlgorithm.alg","");
	if (alg==NULL)
		return 1;
	if (alg->SetConfiguration(attr)==false)
		return 1;
	if (alg->Init()==false)
		return 1;
	alg->Execute(0);

	return 0;
}

