// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmllib.h"


void TestDB()
{
	GML::Utils::GTFVector<GML::DB::DBRecord>		v;
	GML::Utils::INotifier *n = GML::Builder::CreateNotifier("ConsoleNotifier");
	GML::DB::IDataBase *db =  GML::Builder::CreateDataBase("MySQL",*n);
	db->SetProperty("Server='127.0.0.1';Database=TestDB;Username=root;Password=a");
	db->Connect();
	db->ExecuteQuery("Select * from gdtdb limit 5;");
	while (db->FetchNextRow(v))
	{
		for (int tr=0;tr<v.Len();tr++)
		{
			printf("%s->",v[tr].Name);
		}
		printf("\n");
	}

}

int _tmain(int argc, _TCHAR* argv[])
{
	//GML::Utils::AttributeList	attr;

	//if (GML::Builder::GetPluginProperties("RosenblattPerceptron",attr))
	//{
	//	for (int tr=0;tr<attr.GetCount();tr++)
	//		printf("%s\n",attr.Get(tr)->Name);
	//	return 1;
	//}
	
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

	GML::Algorithm::IAlgorithm *alg = GML::Builder::CreateAlgorithm("LinearVote");
	if (alg==NULL)
		return 1;
	if (alg->SetProperty(
		"Name=Test;"
		"DataBase=SimpleTextFileDB{FileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.text-plugin};"//*/
		/*"DataBase=MySQL{Server='127.0.0.1';Database=TestDB;Username=root;Password=a};" //*/
		/*"Connector=SplitConnector{SplitMode=CustomPercentage;PozitiveStart=0;PozitiveEnd=80;NegativeStart=0;NegativeEnd=20;}=>BitConnector{Table=RecordTable;DataFileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.cache};" //*/
		/*"Connector=BitConnector=>SplitConnector{SplitMode=CustomPercentage;PozitiveStart=0;PozitiveEnd=80;NegativeStart=0;NegativeEnd=20;}=>BitConnector{DataFileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.cache};" //*/
		"Connector=BitConnector{StoreRecordHash=False}=>SplitConnector{SplitMode=CustomPercentage;PozitiveStart=0;PozitiveEnd=100;NegativeStart=0;NegativeEnd=100;}=>BitConnector{Query=SELECT * FROM MAL;CountQuery = SELECT COUNT(*) from MAL;StoreRecordHash=True};" //*/
		/*"Connector=BitConnector{StoreRecordHash=True;DataFileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.cache.1};" //*/
		"Notifier=FileNotifier{UseColors=True;FileName=E:\\lucru\\GML\\gml\\prj\\gml\\Release\\a.t;FlushAfterEachWrite=False;TimeFormat=DateTime;ParsableFormat=True};"
		"VotePropertyName=acc;"
		"WeightFileList=E:\\a\\Test_it_7.txt;"
		"LearningRate=0.01;"
		"AdjustWeightMode=UseLearningRate;"
		"InitialWeight=zeros;"
		"useBias=true;"
		"minimAcc=100.0;"
		"SaveData=AfterEachIteration;"
		"ShowFeatureName=true;"
		"FeatureColumnWidth=5;"		
		"SaveBest=None;"
		"WeightFileName=E:\\lucru\\GML\\gml\\prj\\gml\\Debug\\test.txt;"
		"ThreadsCount=2;"
		"MaxError=0;"
		"MarginType=Negative;"
		"SortBy=F2;"
		"SortDirection=descendent;"
		"NotifyResult=True;"
		"ResultFile=E:\\aa.txt;"
		"MultiplyFactor=1000;"
		"ColumnWidth=20;"
		"SaveFeaturesWeight=F2;"
		"FeaturesWeightFile=E:\\a.a;"
		"Command=Test;"
		"CacheName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.cache;"
		"MaxIterations=(INT32)40;")==false)
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

