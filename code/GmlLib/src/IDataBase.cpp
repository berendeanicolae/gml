#include "IDataBase.h"

GML::DB::IDataBase::~IDataBase()
{
}
bool GML::DB::IDataBase::Init(GML::Utils::INotify &_notifier, char *connectionString)
{
	notifier = &_notifier;

	if ((connectionString!=NULL) && (connectionString[0]!=0))
	{
		if (SetProperty(connectionString)==false)
		{
			notifier->Error("Invalid format for DataBase initializations: %s",connectionString);
			return false;
		}
	}
	// am creat atributele
	return OnInit();
}