// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "gmllib.h"


void Test(GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr)
{
	GML::DB::DBRecord	rec;

	for (int tr=0;tr<6;tr++)
		VectPtr.PushByRef(rec);
	for (int tr=0;tr<6;tr++)
	{
		VectPtr[tr].DoubleVal = tr;
		VectPtr[tr].Name = "Hi";
	}

}

int _tmain(int argc, _TCHAR* argv[])
{
	GML::Utils::GTVector<GML::DB::DBRecord> VectPtr;

	Test(VectPtr);
	VectPtr.DeleteAll();


	GML::Utils::AttributeList	attr;
	GML::Utils::GString			str;

	attr.Clear();
	attr.Create("dbPath=E:\\mcu.db");
	attr.UpdateString("dbPath",str);

	attr.AddString("DbName","SimpleTextFileDB");
	attr.AddString("DbConnString","FileName=E:\\lucru\\GML\\gml\\bin\\gdt-db.txt");  
	attr.AddString("Conector","FullCacheConnector");
	attr.AddString("Notifyer","ConsoleNotifyer");
	attr.AddDouble("LearningRate",0.01);
	attr.AddUInt32("MaxIteratii",2);

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

