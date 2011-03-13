// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmllib.h"



int _tmain(int argc, _TCHAR* argv[])
{
	//GML::Utils::AttributeList	attr;

	//attr.AddString("DbName","SimpleTextFileDB");
	//attr.AddString("DbConnString","FileName=E:\\lucru\\GML\\gml\\bin\\a.txt");  
	//attr.AddString("Conector","BitConnector{Table=RecordTable}");
	//attr.AddString("Notifyer","ConsoleNotifier{useColors=True}");
	//attr.AddDouble("LearningRate",0.02);
	//attr.AddUInt32("MaxIteratii",30);

	GML::Algorithm::IAlgorithm *alg = GML::Builder::CreateAlgorithm("BatchPerceptron");
	if (alg==NULL)
		return 1;
	if (alg->SetProperty(
		"Name=Test;"
		"DataBase=SimpleTextFileDB{FileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.text-plugin};"
		"Connector=BitConnector{Table=RecordTable};"
		"Notifier=ConsoleNotifier{useColors=True};"
		"LearningRate=0.01;"
		"AdjustWeightMode=UseSplitLearningRate;"
		"InitialWeight=zeros;"
		"useBias=true;"
		"minimAcc=101.0;"
		"SaveData=AfterEachIteration;"
		"SaveBest=BestACC;"
		"ThreadsCount=2;"
		"MaxError=10000;"
		"MarginType=Negative;"
		"MaxIterations=(INT32)30;")==false)
		return 1;
	if (alg->Init()==false)
		return 1;
		
	alg->Execute("train");
	alg->Wait();

	printf("Algorithm done !\n");
	return 0;
}

