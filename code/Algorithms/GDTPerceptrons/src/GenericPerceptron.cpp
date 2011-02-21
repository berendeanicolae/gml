#include "GenericPerceptron.h"


GenericPerceptron::GenericPerceptron()
{
	db = NULL;
	con = NULL;

	LinkPropertyToString("Name"					,Name					,"");
	LinkPropertyToString("DataBase"				,DataBase				,"");
	LinkPropertyToString("Conector"				,Conector				,"");
	LinkPropertyToString("Notifier"				,Notifier				,"");
	LinkPropertyToDouble("LearningRate"			,learningRate			,0.01);
	LinkPropertyToBool  ("UseWeight"			,useWeight				,false);
	LinkPropertyToUInt32("TestAfterIterations"	,testAfterIterations	,1);
	LinkPropertyToDouble("MinimAcc"				,minimAcc				,100.0);
	LinkPropertyToDouble("MinimSe"				,minimAcc				,100.1);
	LinkPropertyToDouble("MinimSp"				,minimAcc				,100.1);
	LinkPropertyToUInt32("MaxIterations"		,maxIterations			,10);
}
bool	GenericPerceptron::Init()
{
	// creez obiectele:
	if ((notif = GML::Builder::CreateNotifyer(Notifier.GetText()))==NULL)
		return false;
	if ((db = GML::Builder::CreateDataBase(DataBase.GetText(),*notif))==NULL)
	{
		notif->Error("Unable to create Database (%s)",DataBase.GetText());
		return false;
	}
	if (db->Connect()==false)
	{
		notif->Error("Unable to connect to Database (%s)",DataBase.GetText());
		return false;
	}
	if ((con = GML::Builder::CreateConectors(Conector.GetText(),*notif,*db))==NULL)
	{
		notif->Error("Unable to create Conector (%s)",Conector.GetText());
		return false;
	}

	return true;
}