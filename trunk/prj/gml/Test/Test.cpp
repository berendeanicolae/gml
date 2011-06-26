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
void PrintBitSet(GML::Utils::BitSet &bs)
{
	for (int tr=0;tr<bs.Len();tr++)
		printf("%d,",bs.Get(tr));
	printf(" => %d\n",bs.CountElements(true));
}
int my_fnc(UInt32 &e1,UInt32 &e2)
{
	if (e1>e2)
		return 1;
	if (e1<e2)
		return -1;
	return 0;
}
int _tmain(int argc, _TCHAR* argv[])
{
	GML::Utils::GString	tmp;
	

	tmp.Set("Salut");
	//tmp.AddChars('-',10);
	//tmp.InsertChars('-',0,10);
	//tmp.InsertChars('x',3,4);

	//tmp.AddFormatedEx(".Result [%{uint8,L%%,G%%,F%%,B%%}] sal %{int} %{s,R20,F%%} value=[%{dbl,Z%%}]",1000,20,2,'-',2,10,"testare",'.',5.2,0);
	return 1;
	GML::Utils::GTFVector<UInt32> v;
	v.Create(100);
	for (int tr=0;tr<100;tr++)
		v.Push(rand()%10000);
	
	v.Push(100);
	v.Push(50);
	v.Push(1000);
	v.Push(5);
	v.Push(2);
	v.Sort(my_fnc);
	for (int tr=0;tr<v.Len();tr++)
		printf("%d,",v[tr]);
	printf("\n");
	UInt32 val = 1;
	printf("%d\n",v.BinarySearch(val,my_fnc));
	return 1;

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

	GML::Algorithm::IAlgorithm *alg = GML::Builder::CreateAlgorithm("OneSidePerceptron");
	if (alg==NULL)
		return 1;
	if (alg->SetProperty(
		"Name=Test;"
		"DataBase = CSV{FileName=E:\\lucru\\GML\\gml\\prj\\gml\\Release\\a.csv};" //*/
		/*"DataBase=SimpleTextFileDB{FileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.text-plugin};"//*/
		/*"DataBase=MySQL{Server='127.0.0.1';Database=Test_DB;Username=root;Password=a;};" //*/
		/*"Connector=BitConnector{Query=SELECT * FROM MAL;CountQuery = SELECT COUNT(*) from MAL;StoreRecordHash=True};" //*/
		/*"Connector=BitConnector{DataFileName=E:\\lucru\\GML\\gml\\bin\\smalldb_2.cache};" //*/
		"Connector=FullCacheConnector;"
		/*"Connector=SplitConnector{SplitMode=CustomPercentage;PozitiveStart=0;PozitiveEnd=80;NegativeStart=0;NegativeEnd=20;}=>BitConnector{Table=RecordTable;DataFileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.cache};" //*/
		/*"Connector=BitConnector=>SplitConnector{SplitMode=CustomPercentage;PozitiveStart=0;PozitiveEnd=80;NegativeStart=0;NegativeEnd=20;}=>BitConnector{DataFileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.cache};" //*/
		/*"Connector=BitConnector{StoreRecordHash=False}=>SplitConnector{SplitMode=CustomPercentage;PozitiveStart=0;PozitiveEnd=100;NegativeStart=0;NegativeEnd=100;}=>BitConnector{Query=SELECT * FROM MAL;CountQuery = SELECT COUNT(*) from MAL;StoreRecordHash=True};" //*/
		/*"Connector=BitConnector{StoreRecordHash=True;DataFileName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.cache.1};" //*/
		"Notifier=ConsoleNotifier{UseColors=True;FileName=E:\\lucru\\GML\\gml\\prj\\gml\\Release\\a.t;FlushAfterEachWrite=False;TimeFormat=DateTime;ParsableFormat=True};"
		"SortResults=True;"
		"VotePropertyName=acc;"
		"WeightFileList=E:\\a\\a.txt;"
		"HashFileName=E:\\lucru\\GML\\gml\\bin\\hash.txt;"
		"HashStoreMethod=Text;"
		"HashSelectMethod=NegativeCorectelyClasify;"
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
		"ResultsNam = '.\\tmp\\res';"
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
		"Command=Train;"
		"CacheName=E:\\lucru\\GML\\gml\\bin\\small_mcu.txt.cache.test;"
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

