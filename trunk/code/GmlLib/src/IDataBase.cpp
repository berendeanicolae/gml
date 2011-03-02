#include "IDataBase.h"

GML::DB::IDataBase::~IDataBase()
{
	ObjectName = "IDataBase";
}
bool GML::DB::IDataBase::Init(GML::Utils::INotifier &_notifier, char *connectionString)
{
	bool	result;
	notifier = &_notifier;

	if ((connectionString!=NULL) && (connectionString[0]!=0))
	{
		if (SetProperty(connectionString)==false)
		{
			notifier->Error("[%s] -> Invalid format for DataBase initializations: %s",ObjectName,connectionString);
			return false;
		}
	}
	// am creat atributele
	notifier->Info("[%s] -> OnInit()",ObjectName);
	result = OnInit();
	if (result==false)
		notifier->Error("[%s] -> OnInit() returned false",ObjectName);
	return result;
}