#include "CacheBuilder.h"


CacheBuilder::CacheBuilder()
{
	ObjectName = "CacheBuilder";

	SetPropertyMetaData("Command","!!LIST:None=0,CreateCache!!");
	LinkPropertyToString("CacheName",				CacheName				,"","Name of the file that will contain the cache");
}
bool CacheBuilder::Init()
{
	return InitConnections();
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
		return;
	}
	notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
}