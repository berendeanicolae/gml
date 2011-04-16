#include "IMLAlgorithm.h"


GML::Algorithm::IMLAlgorithm::IMLAlgorithm::IMLAlgorithm()
{
	ObjectName = "IMLAlgorithm";

	db = NULL;
	con = NULL;
	notif = NULL;

	LinkPropertyToString("DataBase"					,DataBase				,"DataBase connection string");
	LinkPropertyToString("Connector"				,Conector				,"Connector connection string");
	LinkPropertyToString("Notifier"					,Notifier				,"Notifier plugin");
	LinkPropertyToUInt32("ThreadsCount"				,threadsCount			,1,"Number of threads to be used for parallel computations.");
}

bool GML::Algorithm::IMLAlgorithm::InitConnections()
{
	if ((notif = GML::Builder::CreateNotifier(Notifier.GetText()))==NULL)
		return false;
	if (DataBase.Len()!=0)
	{
		if ((db = GML::Builder::CreateDataBase(DataBase.GetText(),*notif))==NULL)
		{
			notif->Error("[%s] -> Unable to create Database (%s)",ObjectName,DataBase.GetText());
			return false;
		}
		if (db->Connect()==false)
		{
			notif->Error("[%s] -> Unable to connesct to Database (%s)",ObjectName,DataBase.GetText());
			return false;
		}
		if ((con = GML::Builder::CreateConnectors(Conector.GetText(),*notif,*db))==NULL)
		{
			notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,Conector.GetText());
			return false;
		}
	} else {
		if ((con = GML::Builder::CreateConnectors(Conector.GetText(),*notif))==NULL)
		{
			notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,Conector.GetText());
			return false;
		}
	}
	return true;
}