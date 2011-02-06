#include "IDataBase.h"


bool GML::DB::IDataBase::Init(GML::Utils::INotify &_notifier, char *connectionString)
{
	notifier = &_notifier;

	Attr.Clear();
	if (connectionString!=NULL)
	{
		if (Attr.Create(connectionString)==false)
		{
			notifier->NotifyString(0,"Invalid format for DataBase initializations: %s",connectionString);
			return false;
		}
	}
	// am creat atributele
	return OnInit();
}