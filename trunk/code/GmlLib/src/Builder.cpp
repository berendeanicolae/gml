#include "Builder.h"
#include "GString.h"

bool AdjustNameWithExtensionAndPath(GML::Utils::GString *path,char *extension)
{
	// verific daca are extensia care trebuie
	if (path->EndsWith(extension,true)==false)
	{
		if (path->Add(extension)==false)
			return false;
	}
	// verific daca are locatia care trebuie
	return true;
}

//==========================================================================================
GML::Utils::INotify*	GML::Builder::CreateNotifyer(char *pluginName,void *objectData)
{
	GML::Utils::GString		path;
	HMODULE					hModule;
	GML::Utils::INotify*	(*fnCreate)(void *objData);

	if (path.Set(pluginName)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(&path,NOTIFYER_EXT)==false)
		return NULL;

	// incarc libraria
	if ((hModule = LoadLibraryA(path.GetText()))==INVALID_HANDLE_VALUE)
		return NULL;
	// incarc functia Create
	*(FARPROC *)&fnCreate = GetProcAddress(hModule,"Create");
	if (fnCreate==NULL)
		return NULL;
	// am incarcat si totul e ok -> cer o interfata
	return fnCreate(objectData);
}
GML::DB::IDataBase*		GML::Builder::CreateDataBase(char *pluginName,GML::Utils::INotify &notify,char *connectionString)
{
	GML::Utils::GString		path;
	HMODULE					hModule;
	GML::DB::IDataBase*		(*fnCreate)(GML::Utils::INotify &notify,char *connectionString);

	if (path.Set(pluginName)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(&path,DATABASE_EXT)==false)
		return NULL;

	// incarc libraria
	if ((hModule = LoadLibraryA(path.GetText()))==INVALID_HANDLE_VALUE)
		return NULL;
	// incarc functia Create
	*(FARPROC *)&fnCreate = GetProcAddress(hModule,"Create");
	if (fnCreate==NULL)
		return NULL;
	// am incarcat si totul e ok -> cer o interfata
	return fnCreate(notify,connectionString);	
}
GML::ML::IConector*		GML::Builder::CreateConectors(char *conectorsList,GML::Utils::INotify &notify,GML::DB::IDataBase &database)
{
	GML::Utils::GString		list,path;
	int						poz;
	HMODULE					hModule;
	GML::ML::IConector*		(*fnCreate)();
	GML::ML::IConector		*con,*last;
	bool					first;

	if (list.Set(conectorsList)==false)
		return NULL;
	if (list.Len()==0)
		return NULL;
	// vad daca mai am elemente in lista
	first = true;
	while (list.Len()>0)
	{
		poz = list.FindLast("=>");
		if (poz<0)
			poz = 0;
		else
			poz++;
		if (path.Set(&list.GetText()[poz])==false)
			return NULL;
		if (AdjustNameWithExtensionAndPath(&path,CONNECTOR_EXT)==false)
			return NULL;
		// incarc libraria
		if ((hModule = LoadLibraryA(path.GetText()))==INVALID_HANDLE_VALUE)
			return NULL;
		// incarc functia Create
		*(FARPROC *)&fnCreate = GetProcAddress(hModule,"Create");
		if (fnCreate==NULL)
			return NULL;
		if ((con = fnCreate())==NULL)
			return NULL;
		if (first)
		{
			last = con;
			if (last->Init(notify,database)==false)
				return NULL;
		} else {
			if (con->Init(*last)==false)
				return NULL;
			last = con;
		}
		first = false;
		list.Truncate(poz);
	}
	return last;
}

