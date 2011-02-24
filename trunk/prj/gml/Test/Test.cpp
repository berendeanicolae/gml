// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmllib.h"



int _tmain(int argc, _TCHAR* argv[])
{
	GML::Utils::AttributeList	attr;

	//attr.AddString("DbName","SimpleTextFileDB");
	//attr.AddString("DbConnString","FileName=E:\\lucru\\GML\\gml\\bin\\a.txt");  
	//attr.AddString("Conector","BitConnector{Table=RecordTable}");
	//attr.AddString("Notifyer","ConsoleNotifyer{useColors=True}");
	//attr.AddDouble("LearningRate",0.02);
	//attr.AddUInt32("MaxIteratii",30);

	GML::Algorithm::IAlgorithm *alg = GML::Builder::CreateAlgorithm("GDTPerceptrons","RosenblattPerceptron");
	if (alg==NULL)
		return 1;
	if (alg->SetProperty(
		"DataBase=SimpleTextFileDB{FileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.text-plugin};"
		"Conector=BitConnector{Table=RecordTable};"
		"Notifier=ConsoleNotifyer{useColors=True};"
		"LearningRate=0.02;"
		"InitialWeight=random;"
		"useB=true;"
		"SaveData=AfterEachIteration;"
		"SaveBest=BestACC;"
		"MaxIterations=30;")==false)
		return 1;
	if (alg->Init()==false)
		return 1;
		
	alg->Execute("train");
	alg->Wait();

	printf("Algorithm done !\n");
	return 0;
}

