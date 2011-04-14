#include "CacheBuilder.h"


CacheBuilder::CacheBuilder()
{
	ObjectName = "CacheBuilder";

	SetPropertyMetaData("Command","!!LIST:None=0,CreateCache!!");
	LinkPropertyToString("CacheName",				CacheName				,"","Name of the file that will contain the cache");
	LinkPropertyToString("DataBase"					,DataBase				,"");
	LinkPropertyToString("Connector"				,Conector				,"");
	LinkPropertyToString("Notifier"					,Notifier				,"");
}
bool CacheBuilder::Init()
{
	if ((notif = GML::Builder::CreateNotifier(Notifier.GetText()))==NULL)
		return false;
	if (CacheName.Len()==0)
	{
		notif->Error("[%s] -> CacheName property is not set",ObjectName);
		return false;
	}

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
	return true;
}
void CacheBuilder::OnExecute()
{
	if (Command==1)	//CreateCache
	{
		if (con!=NULL)
		{
			if (con->Save(CacheName.GetText())==false)
			{
				notif->Error("[%s] -> Unable to save cache to %s",ObjectName,CacheName.GetText());
			} else {
				notif->Info("[%s] -> Cache created (%s)",ObjectName,CacheName.GetText());
			}
		}
	}
}