#include "Builder.h"
#include "GString.h"

bool FindGMLLibPath(GML::Utils::GString &gmlLib)
{
	HMODULE					hModule;
	GML::Utils::GString		tmp;
	char					temp[2048];

	
	if ((hModule = GetModuleHandleA("gmllib.dll"))==NULL)
		return false;
	if (GetModuleFileNameA(hModule,temp,2048)==0)
		return false;
	if (tmp.Set(temp)==false)
		return false;
	if (tmp.CopyPathName(&gmlLib)==false)
		return false;

	return true;
}
bool AdjustNameWithExtensionAndPath(GML::Utils::GString &path,char *extension)
{
	// verific daca are extensia care trebuie
	if (path.EndsWith(extension,true)==false)
	{
		if (path.Add(extension)==false)
			return false;
	}
	// verific daca are locatia care trebuie
	if (path.Contains("\\")==false) // e doar numele
	{
		GML::Utils::GString		gmlLibPath;
		if (FindGMLLibPath(gmlLibPath)==false)
			return false;
		if (gmlLibPath.PathJoinName(&path)==false)
			return false;
		if (path.Set(&gmlLibPath)==false)
			return false;
	}

	return true;
}

//==========================================================================================
GML::Utils::INotify*		GML::Builder::CreateNotifyer(char *pluginName,void *objectData)
{
	GML::Utils::GString		path;
	HMODULE					hModule;
	GML::Utils::INotify*	(*fnCreate)(void *objData);

	if (path.Set(pluginName)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(path,NOTIFYER_EXT)==false)
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
GML::DB::IDataBase*			GML::Builder::CreateDataBase(char *pluginName,GML::Utils::INotify &notify,char *connectionString)
{
	GML::Utils::GString		path;
	HMODULE					hModule;
	GML::DB::IDataBase*		(*fnCreate)(GML::Utils::INotify &notify,char *connectionString);

	if (path.Set(pluginName)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(path,DATABASE_EXT)==false)
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
GML::ML::IConector*			GML::Builder::CreateConectors(char *conectorsList,GML::Utils::INotify &notify,GML::DB::IDataBase &database)
{
	GML::Utils::GString		list,path,attributeList;
	int						poz,a_poz;
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
		if (path.Strip()==false)
			return NULL;
		if (path.Set(&list.GetText()[poz])==false)
			return NULL;
		a_poz = path.Find("{");
		if (a_poz>=0)
		{
			if (attributeList.Set(&path.GetText()[a_poz+1])==false)
				return NULL;
			if (attributeList.EndsWith("}"))
				attributeList.Truncate(attributeList.Len()-1);
			if (attributeList.Strip()==false)
				return NULL;
			path.Truncate(a_poz);
			if (path.Strip()==false)
				return NULL;
		} else {
			if (attributeList.Set("")==false)
				return NULL;
		}
		if (AdjustNameWithExtensionAndPath(path,CONNECTOR_EXT)==false)
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
			if (last->Init(notify,database,attributeList.GetText())==false)
				return NULL;
		} else {
			if (con->Init(*last,attributeList.GetText())==false)
				return NULL;
			last = con;
		}
		first = false;
		list.Truncate(poz);
	}
	return last;
}
GML::Algorithm::IAlgorithm*	GML::Builder::CreateAlgorithm(char *algorithmPath,char *algorithmName)
{
	GML::Utils::GString			path;
	HMODULE						hModule;
	GML::Algorithm::IAlgorithm*	(*fnCreate)(char *algorithmName);

	if (path.Set(algorithmPath)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(path,ALGORITHM_EXT)==false)
		return NULL;

	// incarc libraria
	if ((hModule = LoadLibraryA(path.GetText()))==INVALID_HANDLE_VALUE)
		return NULL;
	// incarc functia Create
	*(FARPROC *)&fnCreate = GetProcAddress(hModule,"Create");
	if (fnCreate==NULL)
		return NULL;
	// am incarcat si totul e ok -> cer o interfata
	return fnCreate(algorithmName);
}
char*						GML::Builder::GetAlgorithmList(char *algorithmLib)
{
	GML::Utils::GString			path;
	HMODULE						hModule;
	char*						(*fnGetAlgorithmList)();

	if (path.Set(algorithmLib)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(path,ALGORITHM_EXT)==false)
		return NULL;

	// incarc libraria
	if ((hModule = LoadLibraryA(path.GetText()))==INVALID_HANDLE_VALUE)
		return NULL;
	// incarc functia Create
	*(FARPROC *)&fnGetAlgorithmList = GetProcAddress(hModule,"GetAlgorithmList");
	if (fnGetAlgorithmList==NULL)
		return NULL;
	// am incarcat si totul e ok -> cer o interfata
	return fnGetAlgorithmList();
}