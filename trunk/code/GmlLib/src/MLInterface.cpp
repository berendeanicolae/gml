#include "MLInterface.h"


GML::ML::IConector::IConector()
{
	notifier = NULL;
	database = NULL;
	conector = NULL;
}
bool GML::ML::IConector::Init(GML::Utils::INotify &_notifier,GML::DB::IDataBase &_database)
{
	// daca a fost deja initializat
	if ((database!=NULL) || (conector!=NULL))
	{
		if (notifier)
			notifier->Error("Conector already initilized !");
		return false;
	}
	notifier = &_notifier;
	database = &_database;
	conector = NULL;

	return OnInit();
}
bool GML::ML::IConector::Init(GML::ML::IConector &_conector)
{
	// daca a fost deja initializat
	if ((database!=NULL) || (conector!=NULL))
	{
		if (notifier)
			notifier->Error("Conector already initilized !");
		return false;
	}
	conector = &_conector;
	notifier = conector->notifier;
	database = NULL;

	return OnInit();
}