// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmllib.h"


void TestDB()
{
	GML::Utils::INotifier *n = GML::Builder::CreateNotifier("ConsoleNotifier");
	GML::DB::IDataBase *db =  GML::Builder::CreateDataBase("MySQL",*n);
	db->SetProperty("Database=TestDB;Username=root;Password=a");
	db->Connect();

}

int _tmain(int argc, _TCHAR* argv[])
{
	GML::Utils::AttributeList	attr;

	//TestDB();return 0;
	//GML::Utils::Attribute		*a;

	//attr.Load("E:\\aaa.txt");
	//a = attr.Get("Test");
	//printf("%s\n------------------------------\n",a->MetaData);

	//a = attr.Get("Value");
	//printf("%s\n--------------------\n",a->MetaData);

	///*

	//attr.AddString("Test","value","Salut\n!!list:10,20,30!!\nCalculeaza date");
	//attr.AddBool("Value",true);

	//attr.Save("E:\\aaa.txt");
	////*/
	//return 1;

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
		"Notifier=ConsoleNotifier{UseColors=true};"
		"LearningRate=0.01;"
		"AdjustWeightMode=UseLearningRate;"
		"InitialWeight=FromFile;"
		"useBias=true;"
		"minimAcc=100.0;"
		"SaveData=None;"
		"SaveBest=BestACC;"
		"WeightFileName=E:\\lucru\\GML\\gml\\prj\\gml\\Debug\\test.txt;"
		"ThreadsCount=2;"
		"MaxError=0;"
		"MarginType=Negative;"
		"SortBy=F2;"
		"SortDirection=descendent;"
		"NotifyResult=false;"
		"ResultFile=E:\\aa.txt;"
		"MultiplyFactor=1000;"
		"ColumnWidth=20;"
		"SaveFeaturesWeight=F2;"
		"FeaturesWeightFile=E:\\a.a;"
		"Command=Train;"
		"MaxIterations=(INT32)200;")==false)
		return 1;
	//alg->GetProperty(attr);
	//attr.Save("E:\\lucru\\GML\\gml\\prj\\gml\\Release\\template.txt");
	if (alg->Init()==false)
		return 1;
		
	alg->Execute();
	alg->Wait();

	printf("Algorithm done !\n");
	return 0;
}

