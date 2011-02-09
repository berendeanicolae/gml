// dllmain.cpp : Defines the entry point for the DLL application.
#include "gmllib.h"
#include "SQLite.h"

GML::DB::IDataBase*	CreateNewDataBase(GML::Utils::INotify &notify,char *connectionString)
{
	SqliteDatabase	*db = new SqliteDatabase();

	if (db==NULL)
		return NULL;
	if (db->Init(notify,connectionString)==false)
	{
		delete db;
		return NULL;
	}
	return db;
}

BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	return TRUE;
}



