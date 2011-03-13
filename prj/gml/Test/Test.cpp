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

	GML::Algorithm::IAlgorithm *alg = GML::Builder::CreateAlgorithm("OneSidePerceptron");
	if (alg==NULL)
		return 1;
	if (alg->SetProperty(
		"Name=Test;"
		"DataBase=SimpleTextFileDB{FileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.text-plugin};"
		"Connector=BitConnector{Table=RecordTable};"
		"Notifier=ConsoleNotifier{useColors=True};"
		"LearningRate=1;"
		"AdjustWeightMode=UseFeaturesWeight;"
		"InitialWeight=zeros;"
		"useBias=true;"
		"minimAcc=101.0;"
		"SaveData=AfterEachIteration;"
		"SaveBest=BestACC;"
		"ThreadsCount=2;"
		"MaxError=0;"
		"MarginType=Negative;"
		"SortBy=F2;"
		"SortDirection=descendent;"
		"NotifyResult=false;"
		"ResultFile=E:\\a.txt;"
		"MultiplyFactor=1000;"
		"ColumnWidth=20;"
		"SaveFeaturesWeight=F2;"
		"FeaturesWeightFile=E:\\a.a;"
		"MaxIterations=(INT32)20;")==false)
		return 1;
	if (alg->Init()==false)
		return 1;
		
	alg->Execute("train");
	alg->Wait();

	printf("Algorithm done !\n");
	return 0;
}

